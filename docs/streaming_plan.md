# Streaming and asset loading plan (Quanta pages, texture tiles, load time)

This is a work order for an implementing model. Do the tasks in the order of the table under "Order and checkpoints"
(phase L is written after phase 3 but most of it runs first). Each task is small enough to build and test on
its own; do not start the next one until the user has confirmed the previous one builds and its tests pass.

## 0. Rules that apply to every task

Read `CLAUDE.md` first. The points that matter most here:

- **You never build.** Do not run `msbuild`, `generate_project.bat` or the test executables. After each task, stop, list
  what changed, and ask the user to build in Visual Studio and run the tests named in the task. They will paste errors
  back.
- **No commits, no branches** unless the user asks.
- **New or removed source files** need `generate_project.bat`: tell the user, do not run it.
- **`.prism` edits** need a Prism regenerate (use the `prism-regen` skill instructions, or ask the user to run
  Tools → Regenerate Prism code). Never hand-edit anything under `autogen/`.
- **Edit with the Edit tool**, not scripts.
- **No platform ifdefs.** Backend differences go in `sources/HAL/API/D3D12/` and `sources/HAL/API/Vulkan/`.
- **Vulkan code never uses typed `Buffer<T>` reads**; use `StructuredBuffer`.
- **Comments**: only where a competent reader would otherwise think the code is wrong. Keep the existing comments that
  explain ordering and lifetime; update them when the thing they describe changes.
- **`PROFILE(L"name")`** scopes around each distinct per-frame section you add, in an explicit `{ }` block.
- **Runtime knobs** are `Variable<T>` members of the existing contexts (`"Quanta streaming"`, `"Texture streaming"`).
- **Visual results** (does the texture look right, does geometry pop) are checked by the user. Say which toggle to flip
  and what to look for; do not judge them yourself.
- **Temporary logging** goes to `*.temp` files and is listed for removal at the end, not deleted silently.
- End every report with the five sections from *Status reports* in `CLAUDE.md`.

If something in the code does not match what this plan says, stop and report the difference. Do not guess.

## 1. Background: how it works now

Read these fully before task 1:

| File | What it is |
|---|---|
| `sources/RenderSystem/Streaming/Streaming.ixx`, `.cpp` | `Streaming::Streamer`: slots, parent ordering, eviction, slot retirement. Shared by both clients. One thread per streamer. |
| `sources/RenderSystem/Quanta/QuantaResidency.ixx`, `.cpp` | Quanta client (`Residency`, a `Streaming::Backend`). Pages live in the `pages` virtual buffer; `page_slots` is the page table. |
| `sources/RenderSystem/Streaming/TextureStreaming.ixx`, `.cpp` | Texture client (`Manager`, a `Streaming::Backend`). Pages are 64 KB tiles of reserved textures. |
| `workdir/shaders/streaming/stream_compact.hlsl`, `sources/Prism/defs/streaming.prism` | GPU pass that turns Quanta's feedback buffer into a 5 KB request/eviction list. |
| `workdir/shaders/streaming/stream_feedback.hlsl` | What material pixel shaders write for textures. |
| `sources/HAL/HAL.Queue.ixx`, `HAL.Queue.cpp` | `StorageRequest`, `DirectStorageQueue`, `Queue::execute_internal` (applies tile mappings). |
| `sources/HAL/API/D3D12/HAL.D3D12.Queue.cpp`, `sources/HAL/API/Vulkan/HAL.Vulkan.Queue.cpp` | The two `DirectStorageQueue` implementations and `update_tile_mappings`. |
| `sources/HAL/HAL.VirtualBuffer.ixx` | `virtual_gpu_buffer`: queued writes, applied by `prepare(list)`. |
| `sources/RenderSystem/Assets/MeshAsset.cpp` (`SceneFrameManager::prepare`) | The once-per-frame call site of both clients. |
| `sources/Test/Tests/Test.TextureStreaming.ixx`, `Test.Quanta.ixx`, `Test.SceneParity.ixx` | The tests to keep green. |

Lock model today: `Streamer::m` guards the streamer and both clients' state. The streamer thread holds it for the whole
of `process()`, including every `Backend::begin_load`. The render thread takes it in `prepare()`.

## 2. Problems being fixed

1. File reads, CPU decompression and command-list execution happen inside `begin_load`, under `Streamer::m`, so the
   render thread's `prepare()` waits for disk.
2. DirectStorage: one `OpenFile`, one `Submit`, one fence signal and one blocking executor task per request.
3. Texture feedback: 2 MB read back per frame, then a CPU walk over every tile of every texture under the lock.
4. Texture root tiles count inside the pool budget, so enough textures leave no room to load anything.
5. Page-table writes (`page_slots`, `page_flags`, `stream_info`) are one heap vector and one copy command per entry.
6. Tile mapping is one `UpdateTileMappings` per tile.
7. Both streamer threads wake every 2 ms forever.
8. `TextureStreaming::Manager::supported()` is a string compare on the backend name, called on draw paths.
9. Vulkan: the DirectStorage stand-in ignores `StorageRequest::compressed`, and `HAL::gdeflate_decompress` returns
   false there, so a GDeflate-compressed Quanta page cannot be loaded at all.

Target: D3D12 on a modern PC is the fast path. Vulkan (PC, Android) must run correctly through a simple fallback; it
does not need to be fast.

## 3. All findings, by code area

Everything found so far, including small items no task covers yet. When a task takes you into one of these files, fix
the other items listed for that file in the same change if they are marked **fold in**, and mention them in the report.
Items marked **ask** need a decision first. "Read" means found by reading code; "measured" means seen in a capture.

### `sources/RenderSystem/Streaming/Streaming.ixx/.cpp`
| Id | Finding | Evidence | Task |
|---|---|---|---|
| S1 | `process()` holds `m` across every `begin_load` (disk, decompress, list execute) | read | 1.4 |
| S2 | Root pages count inside the budget (texture pool starves) | read | 1.1 |
| S3 | Thread wakes every 2 ms when idle | read | 1.8 |
| S4 | `loading.erase(begin()+i)` in `publish()` and the `remove_if` in `remove()` are O(n²) for large batches | read | fold in with 1.4 |
| S5 | `evict(any=true)` scans every page of every resource | read | fold in with 1.1: keep a count, scan only when over budget (already the case; just early-out when nothing is evictable) |
| S6 | `submit()` keeps the last 2 request sets and drops older ones silently | read | leave; note in a comment why 2 |
| S7 | `page_refs` is sized to `max_page_ids` up front (1M entries × 16 bytes for Quanta) | read | leave |

### `sources/RenderSystem/Quanta/QuantaResidency.ixx/.cpp`, `Quanta.cpp`
| Id | Finding | Evidence | Task |
|---|---|---|---|
| Q1 | `acquire_many`: one file open per root page, main thread waits | measured, 304 ms | L.5 |
| Q2 | `allocate_pool`: one `pages.allocate` per slot | measured, 195 ms | L.5 |
| Q3 | CPU fallback `read_page` under the streamer mutex when `pool_mapped()` is false | read | 1.4 |
| Q4 | One `StorageRequest`, `flush()` per page | measured, 673 calls / 140 ms | 1.3, 1.4 |
| Q5 | Page bytes copied three times on the CPU path | read | 1.5 |
| Q6 | `show`/`hide`/`set_flags`: heap vector + copy command per entry | read | 1.5 |
| Q7 | Compaction passes `Policy{}.eviction_age`, not the streamer's | read | 2.1 |
| Q8 | `acquire_many` is synchronous on the main thread | measured, 534 ms | **ask** (after L.5) |
| Q9 | `prefetched` is a `std::map` keyed by pair, cleared per batch | read | fold in with L.5 (vector indexed by fetch) |
| Q10 | `acquire()` builds `group_dwords` / `node_dwords` by per-record `memcpy` into fresh vectors, then `write` copies again | read | fold in with 1.5 |

### `sources/RenderSystem/Streaming/TextureStreaming.ixx/.cpp`
| Id | Finding | Evidence | Task |
|---|---|---|---|
| T1 | 2 MB feedback readback + clear every frame | measured, ~1 ms/frame, 23 ms worst | 2.2 |
| T2 | `request()` walks every tile of every texture under the mutex | read | 2.2 |
| T3 | Whole-mip granularity: one sample pulls a full mip | read | phase 3 |
| T4 | One copy-queue list per tile to map it before DirectStorage | read | 1.4 |
| T5 | One `update_tilings` per op | measured, 2860 calls, 62 ms worst | 1.7 |
| T6 | `info_writes`: one copy per dword | read | 1.5 |
| T7 | Tile file is uncompressed and the asset also stores full `texdata` (about 2x on disk) | read | phase 3 |
| T8 | `supported()` is a string compare | read | 1.2 |
| T9 | `free_ids` ranges are never merged | read | fold in with 2.2 (merge adjacent on free) |
| T10 | `TileFile` reads through one `fstream` behind a mutex | read | fold in with 1.4 (roots only use it afterwards) |
| T11 | `collect_feedback` callback walks `by_resource` under the mutex on the GPU-wait thread | read | removed by 2.2 |

### `sources/HAL` (queue, DirectStorage, copies, virtual buffer)
| Id | Finding | Evidence | Task |
|---|---|---|---|
| H1 | `DirectStorageQueue::execute`: `OpenFile`, signal and a blocking executor task per request | measured, up to 12 ms per call | 1.3 |
| H2 | `update_tile_mappings`: one native call per tile, vectors reallocated per heap | measured | 1.7 |
| H3 | No batched small-write API on `CopyContext` | read | 1.5 |
| H4 | `virtual_gpu_buffer::write` copies its vector; `aquire()` is `ASSERT(false)` | read | 1.5 |
| H5 | Vulkan DirectStorage stand-in is synchronous and ignores `compressed` | read | 1.6 |
| H6 | `HAL::gdeflate_decompress` returns false on Vulkan | read | 1.6 (**ask**) |
| H7 | `Cache::operator[]` (`Core/Data/Data.ixx`) spins on `yield()` and hangs if the creator throws | read | L.2 |
| H8 | `PipelineStateCache` writes `pso.bin` only in its destructor: a crash loses the session's PSO blobs | read | fold in with L.2 only if trivial, else leave and report |

### `sources/RenderSystem/Materials/universal_material.*`, `Assets/*`
| Id | Finding | Evidence | Task |
|---|---|---|---|
| M1 | Full pipeline deserialized per material before the hash dedupe | measured, 18.7 s thread time (cause read) | L.1, L.2 |
| M2 | `compile()` creates the whole texture via `get_texture()` | measured, 1636 ms | L.3 |
| M3 | BLAS built one command list each | measured, 503 ms | L.4 |
| M4 | Mesh deserialize starts only after all its materials finish | measured, starts at 1654 ms | **ask** (needs a change to how `AssetStorage::load_asset_task` orders work) |
| M5 | Mesh deserialize waits on ~14 fences one after another | measured, 186 ms | fold in with L.4: collect the waiters, wait once |
| M6 | Mesh `load_partitions` 154 ms | measured | not investigated; add a scope inside in L.1 |
| M7 | Engine PSO init about 2 s at startup (`pso init:1957` in the log) | measured | not investigated |
| M8 | Asset lane has 14 of 32 workers | measured | revisit after L.2 |

### `sources/HAL/SIG/RTX.ixx`, `HAL.PipelineState.*` (ray tracing state object)
See phase R below for the details.
| Id | Finding | Evidence | Task |
|---|---|---|---|
| R1 | The update is not in either capture | measured (absent) | R.1 |
| R2 | One set of collections per material, even when materials share one hit library | read | R.2 |
| R3 | One collection per per-material pass, each compiling the same library again | read | R.2 |
| R4 | Collections are built in `update()` while the frame waits | read | R.3 |
| R5 | A material edit rebuilds every engine pass and raygen collection | read | R.4 |
| R6 | `init_ids()` recreates two buffers and `execute_and_wait`s under the lock | read | R.4 |
| R7 | Materials are never removed from the state object or the hit table | read | R.5 |
| R8 | `update_material` / `get_index` touch `materials` without the lock; `materials[mat]` inserts an empty entry for a material still pending | read | R.5 |

---

## Phase 1: shared IO, shared uploads, loads off the lock

### Task 1.1 Root budget in the streamer (small, do first)

**Why:** problem 4. Quanta works around it in `Residency::streaming_budget()` with `streamed_root_pages`; textures do not.

**Change** (`Streaming.ixx/.cpp`):
- Add `uint32_t streamed_roots = 0;` to `Streamer`. In `load_root`, when `resource.streamed`, increment it as well as
  `streamed_in_use`. In `remove`, for each page with a slot where `resource.streamed && resource.is_root(p)`, decrement it.
- Add `uint32_t evictable_in_use() const { return streamed_in_use - streamed_roots; }` and make every budget comparison
  in `process()`, `trim_pool()` and `evict()` use `evictable_in_use()` against `budget`. `budget` now means "slots for
  non-root pages of streamed resources". Update the comment on `budget` and on `set_budget`.
- `QuantaResidency.cpp`: remove `streamed_root_pages` and the `extra_roots` parameter. `streaming_budget()` becomes
  `min(max(2, g_pool_pages), room)` where `room` still subtracts `pinned_pages` and now also the streamer's root count
  (add a `Streamer::root_slots()` accessor). `ensure_pool` calls must still reserve `pinned + roots + budget`.
- `TextureStreaming.cpp`: no change to `set_budget(pool_tiles)` calls; the meaning changed underneath. Update the
  `g_pool_tiles` comment: the pool size is now on top of the always-resident tiles.

**Tests:** all of `Test.Quanta`, `Test.TextureStreaming`, the streamed cases in `Test.SceneParity`. Tests that set a pool
size expecting roots inside it (`set_streaming(true, 2 * roots + detail_pages)` in `Test.SceneParity.ixx`, anything in
`Test.TextureStreaming.ixx` calling `set_pool_tiles`) need their numbers adjusted: read each one and subtract the roots.

**Done when:** tests pass and `stats().free_pages` behaviour in the tests is unchanged apart from the adjusted numbers.

### Task 1.2 Capability bit instead of the backend-name compare

**Why:** problem 8.

**Change:** find how adapter/device capabilities are exposed (`sources/HAL/API/Vulkan/HAL.Vulkan.Adapter.ixx` has
`tiled_3d`, `tiled_depth`; find the D3D12 equivalent and the common accessor). Add a `streamed_textures` capability:
true on D3D12, false on Vulkan. `Manager::supported()` returns that bit. Do not change which backends are supported.

**Done when:** `Test.TextureStreaming` still skips on Vulkan and runs on D3D12.

### Task 1.3 Batched DirectStorage in HAL

**Why:** problem 2. This is the HAL piece both clients will share.

**Change** (`HAL.Queue.ixx`, `API/D3D12/HAL.D3D12.Queue.cpp`, `API/Vulkan/HAL.Vulkan.Queue.cpp`):
- New method `HAL::FenceWaiter DirectStorageQueue::execute(std::span<const StorageRequest> requests);`
  It enqueues every request, then **one** `EnqueueSignal`, then **one** `Submit`, and returns the one waiter. Keep the
  existing single-request `execute` as a wrapper that does not submit (callers call `flush()` as they do now).
- **File cache (D3D12):** a `std::unordered_map<std::wstring, D3D::StorageFile>` in the D3D12 `DirectStorageQueue`,
  guarded by `queue_mutex`. `OpenFile` only on a miss. Remove the per-request `executor.enqueue` that waits and closes
  the file. Add `void DirectStorageQueue::close_file(const std::filesystem::path&)` that erases the entry; it must be
  called before an asset file is rewritten. Find the callers: `TileFile::relocate`
  (`TextureStreaming.cpp`) and wherever a mesh asset file is saved over (`Quanta::Mesh::file`; search `MeshAsset.cpp`
  and `Asset.cpp` for the save path). If you cannot find a clear single place for meshes, stop and ask.
- **Vulkan:** the batch overload loops over the existing single path for now. Task 1.6 replaces it.
- Keep the `DS_STAGING_BUFFER_SIZE` assert per request.

**Tests:** `Test.TextureStreaming` (it uses `get_ds_queue()` directly around line 608), `Test.Quanta`, and loading a
scene from asset files in the app.

**Done when:** tests pass; the D3D12 queue no longer opens a file per request (confirm by reading your own diff, there
is nothing to measure here without a build).

### Task 1.4 Loads start outside the streamer mutex

**Why:** problem 1. This is the biggest change in phase 1; keep it mechanical.

**New shape of `Backend`** (`Streaming.ixx`): split `begin_load` in two.

```cpp
// Under the mutex: decide where the bytes come from. Must not touch disk, decompress, or execute a command list.
virtual LoadPlan plan_load(Resource& resource, uint32_t page, uint32_t slot, bool allow_file) = 0;

// Without the mutex, on the streamer thread: start the IO for a whole batch of plans made by plan_load.
virtual void start_loads(std::span<LoadPlan> plans) = 0;
```

`LoadPlan` holds what `Load` holds today (`from_file`, `waiter`, `poll`) plus whatever the backend needs to start it.
Give it a `std::shared_ptr<void> backend_data` so each backend can keep its own struct without the streamer knowing it.
A plan must keep alive everything it touches (the `HAL::Resource::ptr`, the `std::shared_ptr<Mesh>` or `TileFile`),
because the resource can be removed from the streamer while the IO is in flight.

**`Streamer::process`:**
1. Lock. Build `order`, evict, grow, take slots, set `Loading`, call `plan_load` for each, push onto `loading`, call
   `sync_flags`. Copy the new plans into a local vector. Unlock.
2. Call `backend.start_loads(plans)` with no lock held.
3. Lock. Write each plan's waiter back into its `loading` entry (match by resource pointer and page, and skip entries
   that `remove()` erased meanwhile). Call `publish()`. Unlock.

`load_root` keeps the synchronous path: roots are loaded at acquire/create time, not on the streamer thread. It calls
`plan_load(..., false)` then `start_loads` for that one plan. That call site already holds the mutex; that is
acceptable for roots for now (`acquire_many` prefetches them in parallel). Say so in the comment on `load_root`.

**Quanta backend** (`QuantaResidency.cpp`):
- `plan_load`: choose DirectStorage (same condition as today) or CPU. For CPU with bytes already in memory
  (`source.data` or `prefetched`), queue the `pages.write` right here as today. For CPU needing a file read, record
  mesh, page, slot in the plan.
- `start_loads`: collect all DirectStorage plans into one `execute(span)` call (one waiter shared by the batch). For
  CPU file reads, do `read_page` here (no lock), then take the mutex briefly only to call `pages.write`. Such a plan is
  `from_file = false` and is complete when `start_loads` returns.
- Keep the "cannot read page" log and the zero-fill.

**Texture backend** (`TextureStreaming.cpp`):
- `plan_load` for a file tile: build the `Op` (premapped), remove a pending `Unmap` of the same tile, put the `Op` in
  `staged`, record the mapping (`tiled.map_tile` into an `update_tiling_info`) in the plan. No command list here.
- `start_loads`: build **one** copy-queue list for the whole batch, `update_tilings` each plan's info, execute once.
  All plans share that `mapped` waiter. `DirectTileLoad::poll` stays, but when the map is done it must issue the
  DirectStorage requests for **all** plans of the batch in one `execute(span)`. Simplest: a shared `Batch` object
  (`shared_ptr`) holding the map waiter, the request list, a `requested` flag and the done waiter; each plan's `poll`
  calls `batch->poll()`.
- `plan_load` for a non-file tile (`source` in memory) and for roots: unchanged behaviour (`make_map` + `cut_tile`),
  but `cut_tile`'s file read for roots (`read_tile`) moves into `start_loads`.
- `DirectTileLoad::~DirectTileLoad` waits for the read; keep that guarantee on the batch object.

**Threading rules to keep true:**
- `Backend::show`, `hide`, `set_flags` are still called only with the mutex held.
- `remove()` can run between steps 1 and 3. After it, the plan's slot is in `retiring`; the IO may still write into it.
  That is the same situation as today's in-flight DirectStorage read, and is safe because retired slots are only reused
  after the frame fence. Do not add new handling; do add the "skip erased entries" check in step 3.

**Tests:** all three test files. Then ask the user to run the app with Quanta streaming on and fly through a scene;
what to look for: no missing geometry that stays missing, and the "Loads"/"file loads" stats still rising.

**Done when:** tests pass and no `Backend` method that touches disk or executes a list is reachable with `m` held,
except through `load_root`.

### Task 1.5 Batched table writes

**Why:** problem 5.

**HAL change** (`HAL.CommandList.ixx/.cpp`): add to `CopyContext`

```cpp
struct BufferWrite { uint64 offset; std::span<const std::byte> bytes; };
void update_buffer_batch(Resource* resource, std::span<const BufferWrite> writes);
```

One `pre_command`, one `add_resource_usage(COPY_DEST)`, one `place_data` for the summed size, then one `copy_buffer`
per write from consecutive offsets of that one upload block, one `post_command`. Before copying, sort by offset and
merge writes that are adjacent in the destination into one `copy_buffer`.

**`virtual_gpu_buffer`** (`HAL.VirtualBuffer.ixx`):
- Add `void write(size_t offset, std::span<const T> v)` and `void write(size_t offset, std::vector<T>&& v)` (moves).
  Leave the existing virtual `write(size_t, std::vector<T>&)` because `DataAllocator` declares it.
- `prepare` uses `update_buffer_batch` over `update_list`.

**Clients:**
- `Residency::show` / `hide` / `set_page_present`: use the span overload with a stack value, no vector.
- `Residency::flag_writes`: change to a flat `std::vector<std::pair<uint32_t, std::array<uint32_t,2>>>` plus the
  existing range form for the acquire-time zero fill; flush with `update_buffer_batch`.
- `Residency::begin_load`/`start_loads` CPU path: build the dword vector once and move it into `pages.write`.
- `Manager::info_writes`: flush with `update_buffer_batch`. If the same `view` appears more than once, the last value
  wins: dedupe keeping the last before flushing.

**Tests:** all three test files.

### Task 1.6 Vulkan fallback IO

**Why:** problems 1 and 9 on Vulkan.

**Stop and ask the user first** which of these they want for GDeflate pages on Vulkan, and recommend (a):
- (a) `spectrum_tool`/asset save writes Quanta pages **uncompressed** when the target is not D3D12, and the Vulkan
  loader refuses compressed pages with a clear log line naming the asset. Simple, larger files on Android.
- (b) Add a CPU GDeflate decoder to the Vulkan backend (a new vcpkg port or vendored reference decoder). More work,
  needs an arm64-android port in `AndroidMetadata.VcpkgPorts`.

Whichever is chosen:
- `HAL.Vulkan.Queue.cpp` `DirectStorageQueue::execute(span)`: no `execute_and_wait`. Read all files on the calling
  thread (after task 1.4 that is the streamer thread, without the lock), record all uploads into one upload list,
  `execute()` it and return that list's waiter wrapped so `is_completed()` reflects the GPU fence. If the fence types
  do not line up, signal `requestCounter` from the list's `on_done` callback and return a waiter on that.
- Handle `srequest.compressed`: decode with `HAL::gdeflate_decompress` when it is supported, otherwise log once per
  file and upload zeros of `uncompressed_size`. Never upload compressed bytes as if they were raw.
- `StorageRequest::Tiles` stays unsupported on Vulkan (`ASSERT(false)`), since streamed textures are off there.

**Tests:** `test_vulkan.exe` Quanta and scene-parity tests (the Vulkan test exe has its own name, see memory note
"FrameGraph tests").

### Task 1.7 Batched tile mapping

**Why:** problem 6.

**Change:**
- `Manager::prepare` (`TextureStreaming.cpp`): group ops by texture. For each texture build **one**
  `update_tiling_info` containing all its maps and unmaps of this frame (the `tiles` map inside the info is already
  keyed by heap, so several tiles fit in one info), and call `list->update_tilings` once per texture. Order matters
  when the same tile is unmapped then mapped in one frame: process ops in order and let the later entry replace the
  earlier one for the same `(mip, x, y)` before building the info.
- `Queue::update_tile_mappings` (D3D12): hoist the five vectors out of the loop and `reserve`. Within one heap, merge
  consecutive entries whose tile coordinates are adjacent in x on the same subresource **and** whose heap offsets are
  consecutive into one region (`NumTiles = n`, `UseBox = false`) with one range. Keep null (unmap) ranges separate.
- Do not change `Queue::execute_internal`'s structure.

**Tests:** `Test.TextureStreaming`, VSM still works (it uses the same tile path: ask the user to check shadows render,
toggle none, just look at a shadowed scene), VoxelGI (uses `update_tilings` in `VoxelGIGraph.cpp`).

### Task 1.8 Streamer sleeps when idle

**Why:** problem 7.

**Change** (`Streaming.cpp` `run()`): wait with the 2 ms timeout only while `loading` is non-empty; otherwise wait with
no timeout on `wake`. `loading` is guarded by `m`, the wait by `queue_mutex`: keep a `std::atomic<bool> has_loading`
that `process()` and `publish()` update, and read that in `run()`. Do not take `m` inside the `queue_mutex` wait.

**Tests:** all three; additionally ask the user to confirm the app still exits cleanly (the `stop()` path).

---

## Phase 2: one feedback path for both clients

Goal: textures stop reading 2 MB back and stop walking every tile on the CPU. They report into a per-page-id buffer
and go through the same `StreamCompact` pass as Quanta. Granularity stays **per mip of a texture** in this phase.

### Task 2.1 Move the compaction into the shared layer

- New class `Streaming::GpuFeedback` in `Streaming.ixx/.cpp` owning what `Residency` owns today for this: the
  `feedback`, `page_flags` and `stream_out` buffers, the flag-write queue, the layout constants
  (`RequestBuckets` … `OutDwords`), the dispatch and the `read_buffer` callback that builds `Streaming::Requests` and
  calls `Streamer::submit`. Constructor takes the device and `max_page_ids`. Methods:
  `set_flags(page_id, flags, epoch)`, `clear_range(first, count)`, `prepare(list, page_count, epoch, eviction_age)`.
- The callback must hold the streamer weakly (`std::weak_ptr<Streamer>`), not go through a singleton.
- Pass the streamer's real `policy.eviction_age` (add an accessor); today `Residency::prepare` passes
  `Streaming::Policy{}.eviction_age`.
- `Residency` uses it; its public `feedback` member becomes an accessor to `GpuFeedback`'s buffer. Fix the three users
  (`MeshRenderer.cpp:275`, `VSM.cpp:1005`, `DebugView.cpp:179`).
- No shader or `.prism` change in this task.

**Tests:** `Test.Quanta`, `Test.SceneParity`.

### Task 2.2 Textures report per mip into page-id space

Design:
- Each streamed texture gets one **mip page id** per standard mip, in addition to its tile page ids. Reporting "mip k
  of texture T was sampled" writes `stamp` to `feedback[(mip_id_base + k) * 2]` and the epoch to the touch dword, the
  same two dwords Quanta uses.
- `stream_info` grows from one dword per view to two: `[view*2] = min_mip + 1` (as now), `[view*2+1] = mip_id_base`
  (0 when not streamed). Update `ViewCapacity` buffer size, `bind_view`, the info writes, `stream_min_mip` in
  `universal_material.hlsl`, `stream_report` in `stream_feedback.hlsl`, and `test/stream_clamp_probe.hlsl` with its
  test in `Test.TextureStreaming.ixx`.
- `stream_report` keeps the wave trick: one `InterlockedMax` per wave when all lanes share the view.
  Priority for the stamp: `16 - mip` scaled into the half-float range the compaction buckets by (see `PriorityBucket`
  in `stream_compact.hlsl`); coarser mips must get the **higher** priority. Write the mapping down in a comment in
  `stream_feedback.hlsl` because the constant comes from the bucket layout, not from choice.
- CPU side, a "mip page" is a real `Streaming::Resource` page whose load does nothing and whose **children are the
  tiles of that mip**... do **not** do this. Keep it simpler: `Manager` owns a `GpuFeedback` sized for mip ids only,
  but does not hand its output to `Streamer::submit` directly. Its readback callback receives the compacted list
  (wanted mip ids, idle mip ids) and expands each into tile `Want`s / `Victim`s for that mip under the mutex, then
  calls `streamer->submit`. To allow that, give `GpuFeedback::prepare` an optional
  `std::function<void(Streaming::Requests&&)>` sink; default sink submits to the streamer.
- Flags for mip ids: `FlagLoading` while any tile of the mip is loading and none missing; `FlagEvictable` while the mip
  is finer than the root and has resident tiles; the "slot" test in the shader reads `page_slots[page] == INVALID`, so
  `Manager` needs a small `mip_present` buffer bound as `page_slots` (INVALID while the mip is incomplete, 0 once
  complete). Maintain it from the same place `published_mip` is updated in `Manager::prepare`.
- Delete `wanted`, `collect_feedback`, `request_from_feedback`, `HoldFrames`, `wanted_mip`, `last_seen`, and the
  `request()` tile walk. Keep `Manager::request(span<Demand>)` as a test hook only if tests use it (they do: check
  `Test.TextureStreaming.ixx`); implement it by building the `Requests` directly as today.
- `QuantaFeedback::textures` in `meshrender.prism` now points at the texture `GpuFeedback` buffer; update the comment.
  `set_draw_feedback` in `MeshRenderer.cpp` binds it.
- `SceneFrameManager::prepare` (`MeshAsset.cpp`): the texture block becomes just `textures.prepare(command_list)`.

This task touches `.prism` (regenerate) and shaders (the user runs `spectrum_tool shaders` or the app recompiles; ask).

**Tests:** `Test.TextureStreaming` end to end. Then visual check by the user: Properties → "Texture streaming" →
"Debug view" = `MissingMips`, move the camera toward a large textured surface; expected: red fades to green within a
second, and returns to green-only when still. Then `SampledMip` to confirm fine mips appear near the camera.

**Done when:** no per-frame readback larger than `OutDwords * 4` bytes exists in either client.

---

## Phase 3: per-tile texture residency (only after the user says go)

Do not start this without an explicit instruction. It changes what the shaders sample and how assets are stored.

Outline, to be turned into tasks at that point:
1. **Min-mip map.** Per streamed texture, a small region in a shared `R8_UINT` atlas, one texel per mip-0 tile region
   (for example 32×32 for an 8K BC texture): the finest mip resident for that region. Material shaders read it with
   the UV and use it as the LOD clamp instead of the per-texture `stream_info` value. `stream_info` then carries the
   atlas rectangle.
2. **Per-tile feedback.** `stream_report` computes the tile page id from UV and mip
   (`first_page[mip] + y * tiles_x + x`; the per-mip `first_page` and `tiles_x` come from a small per-texture table in
   a buffer) and writes straight into tile page-id space. The mip-id indirection from task 2.2 is removed, and the
   compaction output goes directly to `Streamer::submit`.
3. **Parents** already express "coarser tile first"; `min_mip()`/`complete_mip()` are replaced by the map.
4. **GDeflate tiles** in `TileFile` (per-tile offset table instead of fixed 64 KB stride), with
   `StorageRequest::compressed = true` on the `Tiles` destination. Needs an asset version bump and a fallback read of
   the old layout.
5. Ray tracing keeps reading the always-resident root mips; nothing changes there.

---

## Batch 1 results (measured after tasks 1.1–1.4, L.1, R.1, L.3, L.4 were built)

Same two actions, same asset. These numbers replace the guesses made before the instrumentation existed.

| What | Before | After | Verdict |
|---|---|---|---|
| Drop asset, until usable | 2498 ms | 2464 ms | unchanged, as expected: the material stage (L.2) is not done yet |
| `material_compile_get_texture` | 1636 ms | 0.1 ms | L.3 done |
| `mesh_init_gpu` | 503 ms | 397 ms | L.4 missed its target, see below |
| `DirectStorageQueue::execute` in the add-instance capture | 673 calls, 140 ms | 27 batched calls, 12 ms | 1.3 and 1.4 work |
| `quanta_acquire_many` | 534 ms | 539 ms | unchanged, L.5 not done |
| `RTX update` on the frame after the drop | not captured | 298 ms, once | measured, see below |

**Material pipeline load (M1): the cause was not what this plan assumed.**
- Of 132 materials, 56 load their pipeline in under 1 ms and create nothing. The other 76 take about 250 ms each.
- In those 76, `pso_create_graphics` is 15.0 s of the 19.0 s (1734 creations, 22 to 27 per material, median 10.7 ms),
  `shader_ser_load` is 2.4 s (70 to 73 shaders per material).
- The spinning `Cache` wait accounts for at most the 2 s that is left. It is not the main cost.
- A PSO creation takes about 1 ms when few threads are creating and about 10 ms when all 14 asset workers are, so the
  driver or the PSO cache serializes them. `cache/d3d12/pso.bin` exists (98 MB), so blobs are being used.
- Conclusion: the cost is **creating every key variant of every PSO set at load** (GBuffer, depth, voxel, each with
  all its key combinations), most of which a session never draws with.

**Ray tracing state object (R1): measured.**
- One `RTX update` of 298 ms on the frame after the drop; the frame waits for it.
- 289 ms of that is `rtx_build_pending_collections`: 396 collections (132 materials × 3 passes), 7.7 s of thread time
  on 30 threads. 305 of them take under 20 ms, 91 take 20 to 190 ms.
- `AddToStateObject` is 8 ms. The add path itself is cheap; compiling the collections is the cost.
- The count of distinct hit libraries is still missing (the first temporary log did not compile; it now lives in
  `RTX::update()` in `Effects/RTX/RTX.cpp` and writes `workdir/rtx_libs.temp`).

**BLAS (M3): batching was not the bottleneck.**
- 25 lists instead of 1591, and `mesh_init_gpu_blas_execute` is 13 ms in total. The 381 ms is inside the recording:
  about 0.16 ms median per structure in `RaytracingAccelerationStructure`'s constructor (its own scratch and result
  buffers) and `build_ras`.

**New, from this capture.**
- `mesh_load_quanta_partition`: 150 ms on one thread (M6 measured).
- `asset_load_partitions`: 3.2 s of thread time over 536 assets (was 1.5 s). Texture assets create their streamed
  texture there and all take the texture streamer's mutex to load roots; the rise is contention, not wall time.

## Batch 2 results (measured after tasks 1.5, 1.7, 1.8, L.5, revised L.2 were built)

Same two actions, same asset, assets saved so the materials load without regenerating.

| What | Batch 1 | Batch 2 | Verdict |
|---|---|---|---|
| `drop_load_asset` | 2464 ms | 1430 ms | L.2 works; target was 1200 ms |
| PSO creations during the drop | 1734, 15.0 s of thread time | 431, 5.6 s | lazy variants; still about 13 ms each under load |
| `mesh_init_gpu` | 397 ms | 430 ms | unchanged; `blas_create_buffers` is 404 ms of it (1591 × 0.25 ms): L.4b confirmed |
| `quanta_acquire_many` | 539 ms | 504 ms | L.5 missed its target, see below |
| `RTX update` after the drop | 298 ms | 207 ms | 396 collections, 5.3 s of thread time in `state_object_create` |
| `mesh_load_quanta_partition` | 150 ms | 192 ms | unchanged (L.6 only added scopes) |

**`acquire_many` (L.5): the two costs moved, they did not shrink.**
- `quanta_read_stored_pages`: 253 ms. One task per file means one thread reads every root page of the asset.
- `quanta_allocate_pool`: 177 ms for one range. The cost is in mapping the range's tiles, not in the allocator.
- `update_tile_mappings`: 3277 calls in the add-instance capture and 4916 in the drop: still one native call per
  small update.

**Ray tracing (R.1 finished).** 132 pending materials have 69 distinct hit libraries (`workdir/rtx_libs.temp`). R.2
takes the collections from 396 (132 × 3 passes) to 69.

**New, from this capture.**
- The mesh asset's own `asset_deserialize` is 620 ms on one thread, and it only starts 618 ms into the drop, after
  its dependencies. Not split into scopes yet.

**A regression found on the way (commit "fast mesh loading", fixed).** Loaded materials stopped regenerating, even
with a stale saved pipeline: the decision compared header versions, and a freshly loaded header is at version 0 like
a material that was never generated. Translucent thin meshes (wires, ropes, poles) vanished. `load_partitions` now
sets `need_regenerate_material` unless both saved header hashes match. A material file saved before the hashes
existed regenerates on every load until it is saved again.

## Batch 3 results (measured after R.2, L.4b, the `acquire_many` rework and run-based tile mapping were built)

| What | Batch 2 | Batch 3 | Verdict |
|---|---|---|---|
| `drop_load_asset` | 1430 ms | 1252 ms | target 1200 ms |
| RT collections built | 396, 5.3 s of thread time | 69, 2.2 s | R.2 works; `rtx_build_pending_collections` 198 ms → 82 ms, still inside the frame (R.3) |
| `mesh_init_gpu` | 430 ms | 280 ms | recording is now about 47 ms; 232 ms is `mesh_init_gpu_wait`, the load thread waiting for the GPU builds |
| `quanta_acquire_many` | 504 ms | 206 ms | target 150 ms; 10 read tasks of 85 to 100 ms each, so the read is now bound by per-page seeks and reads |
| `update_tile_mappings` | 4916 calls, one of 154 ms | 1810 calls, one of 83 ms | run-based mapping works |
| Mesh `asset_deserialize` | 620 ms | 470 ms | `mesh_ser_parts` is 185 ms of it (1591 parts) |
| `mesh_load_quanta_partition` | 192 ms | 160 ms | unchanged |

**Second regression from commit "fast mesh loading" (fixed).** A pipeline's `id` is saved in the material file and was
reused on load. Ids are only unique within the run that handed them out, so loaded pipelines could share one, and the
gather (which routes draws by pipeline id) dropped one material's draws: the "thin meshes missing, at random" bug.
`PipelineManager::get_pipeline` now gives a loaded pipeline a fresh id.

## Batch 4 results (measured after R.3, R.4, R.5 removal, 2.1, the coalesced page read and the deferred BLAS wait)

D3D12 only; Vulkan was not run.

| What | Batch 3 | Batch 4 | Verdict |
|---|---|---|---|
| `drop_load_asset` | 1252 ms | 961 ms | under the 1200 ms target |
| `RTX update` after the drop | 82 ms blocked | not in the capture's top scopes | R.3 works; `state_object_create` is 472 ms of thread time on the pool |
| Mesh `asset_deserialize` | 470 ms | 232 ms | the BLAS wait is gone from it |
| `mesh_ser_parts` | 185 ms | 183 ms | `mesh_part_vertex_view` alone is 175 ms (1591 parts); the index view and meshlet range are negligible |
| `quanta_acquire_many` | 206 ms | 267 ms | worse: the coalesced read is slower than the per-page one (10 tasks of 137 to 154 ms, were 85 to 100) |
| `mesh_load_quanta_partition` | 160 ms | 139 ms | 98 ms of it is self time, outside the `quanta_partition_*` scopes |

**New, from this capture.**
- `texture_stream_add_lock_wait`: 751 ms of thread time over 224 texture loads. The manager mutex serializes them.
- The mesh asset's deserialize starts 590 ms into the drop, after its dependencies. During that time the asset lane
  also runs the 431 PSO variant creations (4.8 s of thread time, about 11 ms each) and the 69 collection builds, so
  they compete with the loads they were moved out of.

## Phase L: asset loading time

Independent of the streaming phases. Tasks L.1 to L.4 touch no streaming code and go first, because they are backed by
measurements. L.5 edits `QuantaResidency.cpp` and goes after task 1.1.

**Measured baseline** (traces in `workdir/captures/`, one run each, build configuration not recorded):

- `drop_asset_0.json`: 2498 ms for 536 assets (132 materials, one mesh of 1591 parts).
  - 0–1650 ms: all 14 asset-lane workers inside `material_ser_pipeline`, 18.7 s of thread time, median 206 ms each.
  - `material_compile_get_texture`: 405 calls, 1636 ms.
  - 1840–2343 ms: `mesh_init_gpu`, 1591 BLAS builds on one thread.
- `add_mesh_instance_1.json`: `quanta_acquire_many` 534 ms on the main thread: `quanta_prefetch_roots` 304 ms,
  `quanta_acquire_many_grow_pool` 195 ms, everything else about 35 ms.

The captures are made by the profiler capture feature added in commit "fast mesh loading" (`Profiling.ixx/.cpp`); read
it to see how a capture is started and where the summary is logged. After each task ask the user to repeat the same
action (drop the same asset, add the same instance) and paste the new `capture '...'` lines from `workdir/log.txt`.
Compare against the numbers above in your report. You do not run the app yourself.

### Task L.1 Find out what `material_ser_pipeline` spends its time on

**Why:** the trace has no scopes inside it, so the two causes below are read from code, not measured. Confirm before
fixing.

**Change (instrumentation only, permanent `PROFILE` scopes, no `.temp` logging needed):**
- `sources/HAL/SIG/PSO.ixx`: a scope around the PSO set's deserialize and around each variant's creation. Find the
  `SERIALIZE()` of the generated PSO classes (`PSOS::Meshes::GBufferDraw` and friends derive from the templates in this
  file) and where the key variants are built (`shuffle_pairs`).
- `sources/HAL/HAL.PipelineState.cpp`: scopes in the two `PipelineStateCache` creation lambdas, named
  `pso_create_graphics` and `pso_create_compute`.
- `sources/Core/Data/Data.ixx` `Cache::operator[]`: a scope named `cache_wait_other_thread` around the
  `while (!elem) yield()` loop only.
- `sources/HAL/HAL.Shader.ixx`: a scope around the shader `SERIALIZE()` load branch (`ShaderType::get_resource`).

`Core/Data/Data.ixx` is in Core: check that `PROFILE` is usable there (the macro is forced-included through
`Core/Defines.h`; if the profiling module is not importable from that file, skip that one scope and say so).

**Done when:** the user's new capture shows where the 18.7 s goes. Report the split and stop. L.2 depends on it.

### Task L.2 Create PSO variants when they are first used (REVISED after measuring)

**Read "Batch 1 results" first.** The original L.2 below this task (hash-first dedupe, `Cache` spin) is kept as
**L.2b**, lower priority: it removes at most about 2 s of the 19 s of thread time.

**Why:** a material's pipeline holds PSO sets (`PSOS::Meshes::GBufferDraw`, `DepthDraw`, `GI::Voxel::Voxelization`,
`VSMDepthDrawMaterial`, `GBufferDrawVS`). Constructing or deserializing a set creates the PSO for **every** key
combination (22 to 27 per material, about 10 ms each under load). A session draws with a handful of them.

**Change:**
- Read `sources/HAL/SIG/PSO.ixx` (`PSOBase`, `shuffle_pairs`, `GetPSO`) and one generated set under
  `sources/HAL/autogen/pso/` to see where the variants are created and stored. The generated code comes from Prism
  templates in `sources/Prism/`: if the eager loop is in generated code, change the template and regenerate (see the
  `prism-regen` skill), never the generated file.
- Make variant creation lazy: the set keeps the description needed to build any variant, and `GetPSO(keys)` creates
  the variant on first request. The first request must not stall a frame: start the creation on the thread pool and
  have callers treat the variant as not built until it is. `PipelinePasses::is_built()` already exists for exactly
  this and `mesh_renderer` already skips pipelines that are not built; read both and extend them to "the variant this
  draw needs is built", not "all variants are built".
- Warm the variants that are certainly needed (the default keys of GBuffer and depth for the current device) right
  after load, on the asset lane at low priority, so the first frame with the material is not empty for long.
- Serialization must keep working in both directions: what is saved per variant today (shader references, the PSO
  description) must still be saved for all variants, so a device build without a shader compiler can create any of
  them later. Only the D3D12 object creation moves.
- `spectrum_tool shaders` uses `PSOBuildObserver` to see every PSO and key combination: that path must still visit
  all variants. Keep an explicit "build everything" entry point for it and for tests.

**Stop and ask** before coding if making `GetPSO` able to return "not ready" touches more than the mesh draw paths
(search for `GetPSO(` across `sources/`); list the call sites and propose how each handles a missing variant.

**Tests:** all tests (PSO sets are used everywhere). User check: drop the asset; materials appear within a few frames,
nothing stays invisible, shadows and voxel GI still include the new mesh.

**Done when:** `material_ser_pipeline` thread time in the drop capture falls below 4 s and `drop_load_asset` below
1.2 s.

### Task L.2b Pipeline dedupe before deserialize, and the `Cache` wait (was L.2)

**Expected cause 1: the dedupe runs after the work.** `universal_material`'s `SERIALIZE()`
(`sources/RenderSystem/Materials/universal_material.ixx`) does `ar & NVP(pipeline)`, which deserializes a complete
`PipelinePasses` with all its PSO variants, and only then calls `PipelineManager::get_pipeline(pipeline)`, which throws
the new one away if another material already registered the same `hash`.

**Expected cause 2: waiters spin.** `Cache::operator[]` (`sources/Core/Data/Data.ixx`): a thread that misses on a key
another thread is creating loops on `std::this_thread::yield()`, and loops forever if the creator throws.

**Change:**
- `Cache`: store `std::shared_future<V>` per key, the way `resource_manager_base::get_resource`
  (`sources/Core/FileSystem/ResourceManager.ixx`) does. The first thread creates outside the lock and sets the promise
  (set the exception on throw, then erase the entry so a later call retries); others wait on the future. `Cache` has a
  public `table` member: find every user of `table` and `size()` first and keep them working.
- Pipeline dedupe before the PSOs are read. On save, write `pipeline->hash` ahead of the pipeline. On load, read the
  hash, ask `PipelineManager` for an existing pipeline with it, and only deserialize into a new one on a miss.
  The archive is sequential, so on a hit the stored pipeline bytes still have to be consumed: the simplest correct
  form is to move the pipeline into its own partition (`save_partitions` / `load_partitions`, like the existing
  `material_build` partition in `universal_material.cpp`), which is only opened on a miss. Keep reading the old inline
  layout for files saved before the change: gate on a version field or on the presence of the new partition, and say
  in the report which you used.
- Two threads missing on the same hash at once must not both build: `PipelineManager` needs a "being loaded" entry
  (a shared future per hash) so the second waits for the first.

**Do not change** what a pipeline hash covers (`crc32(pixel + tess + mode)`), nor `PipelinePasses::is_built`.

**Tests:** `Test.MaterialX`, `Test.Serialization`, the scene-parity tests. Then the user repeats the drop-asset capture.
Visual check by the user: the dropped scene's materials look the same as before (no missing or swapped materials).

**Done when:** `material_ser_pipeline` thread time drops by at least 5x in the new capture, or you can show from L.1's
scopes that the remaining time is real PSO creation.

### Task L.3 Materials must not create the whole texture

**Why:** `universal_material::compile()` (`universal_material.cpp`, scope `material_compile_get_texture`) calls
`tex->get_texture()->texture_2d()` only to learn whether a usable 2D texture exists. For an asset loaded as tiles,
`TextureAsset::get_texture()` (`sources/RenderSystem/Assets/TextureAsset.cpp`) then reads the full `texdata` partition
and creates the complete texture, which is exactly what streaming is meant to avoid.

**Change:**
- Add `bool TextureAsset::has_texture_2d()` that answers without creating anything: true if `streamed` exists; else if
  `tiles` exists, from the `TileFile` header (it derives `HAL::texture_data_header`: 2D means `array_size == 1` and
  `depth <= 1`); else fall back to today's `get_texture()` test.
- `compile()` uses it, then calls `get_render_texture()` as it already does.
- Search for other `get_texture()` callers on load paths (`Grep` for `->get_texture()` under
  `sources/RenderSystem`). List them in your report. Change only ones that clearly just test for existence; ray tracing
  and editor previews may really need the whole texture, so leave those and name them.

**Tests:** `Test.TextureStreaming`, `Test.MaterialX`. User check: with "Texture streaming" → "Debug view" =
`SampledMip`, streamed textures still refine near the camera; and the whole-texture count in memory should drop (the
user can compare video memory before and after loading the same scene).

**Done when:** `material_compile_get_texture` is under 50 ms total in the drop-asset capture.

### Task L.4 Batch the BLAS builds

**Why:** `MeshAsset::init_gpu()` (`sources/RenderSystem/Assets/MeshAsset.cpp`) takes a command list, builds one BLAS
and executes, 1591 times in a row on one thread: 503 ms.

**Change:**
- Build `BlasPerList` (start with 64, a named constant with a one-line comment that it is a tuning value) acceleration
  structures per command list, then execute and start the next list. Keep only the last waiter and wait once at the
  end, as now.
- Check `RaytracingAccelerationStructure`'s constructor (search `sources/HAL` for the class) for anything that assumes
  one structure per list, such as scratch buffers taken from the list's per-list allocator or a resource-state change
  that must be flushed between builds. If scratch memory per list would grow too large, lower the batch size, do not
  restructure the class.
- Do not parallelize across threads in this task.

**Tests:** scene-parity tests (they cover ray tracing on both backends), `Test.Quanta`. User check: ray traced shadows
and reflections on the dropped mesh look as before (toggle "Shadow Src" to the ray traced reference if available and
compare).

**Done when:** `mesh_init_gpu` is under 150 ms in the drop-asset capture.

### Task L.4b BLAS recording cost (follow-up to L.4)

L.4 is in and correct but saved only 100 ms: the time is in recording, not in list execution (see "Batch 1 results").
- Add scopes inside `RaytracingAccelerationStructure`'s constructor: buffer creation, `build_ras`, anything else.
- Expected fix once confirmed: one scratch buffer per command list reused by every build in it (builds in one list
  run in order, so they can share scratch only if a UAV barrier separates them: check what `build_ras` emits), and
  result buffers sub-allocated from a few large buffers instead of one resource per structure.
- Alternative worth measuring: split the parts across the asset-lane workers, one list each.

**Done when:** `mesh_init_gpu` is under 150 ms.

### Task L.6 Quanta partition load (M6)

`mesh_load_quanta_partition` is 150 ms on one thread. Add scopes inside `Quanta::AssetPartition`'s deserialize
(`sources/RenderSystem/Quanta/QuantaAsset.ixx`, `Quanta.cpp`) to split header, group/node tables and the parents
table, then report. No fix until the split is known.

### Task L.5 `acquire_many`: one read, one pool growth (after task 1.1)

**Why:** 534 ms on the main thread when an instance is added.

**Change** (`sources/RenderSystem/Quanta/QuantaResidency.cpp`, `Quanta.cpp`):
- **Root page reads.** `read_page` opens the file anew for each page. Add
  `bool read_pages(const Mesh&, std::span<const uint32_t> pages, std::vector<std::vector<uint8_t>>& out)` in
  `Quanta.cpp` that opens the file once, reads the requested pages in ascending `file_offset` order, and decompresses
  each. In `acquire_many`, group the fetches by mesh file and run one task per file (not one per page) for the reads;
  decompression of the pages read may still fan out on the pool.
- **Pool growth.** `Residency::allocate_pool(count)` calls `pages.allocate(pool[i], PageDwords)` per slot. Allocate one
  range of `count * PageDwords` and carve the slots out of it. `pool` holds `TypedHandle<uint>` per slot and slots are
  freed individually only by the demand-pool path, which Quanta does not use (`use_demand_pool` is texture-only):
  confirm that by reading `Residency`, then store the range handles separately and make `pool[i]` a plain dword offset.
  Every `pool[slot].get_offset()` call site changes accordingly.
- **One batch.** Remove the `BatchSize = 128` loop unless the prefetched bytes of a whole instance would be too large
  to hold at once; if you keep batching, grow the pool once for the whole call, before the loop.
- Keep the behaviour that a refused mesh comes back null and is logged.
- Do **not** move the call off the main thread in this task. That changes when an instance becomes visible and needs a
  design decision from the user; mention it under Planned in your report.

**Tests:** `Test.Quanta`, streamed and pinned cases in `Test.SceneParity`. Then the user repeats the add-instance
capture.

**Done when:** `quanta_acquire_many` is under 150 ms in the new capture.

---

## Phase R: ray tracing state object

Files: `sources/HAL/SIG/RTX.ixx` (`RTXPSO`, `RaytracePass`, `RaytraceRaygen`), `sources/HAL/HAL.PipelineState.ixx/.cpp`
and `sources/HAL/API/D3D12/HAL.D3D12.PipelineState.cpp` (`StateObject`), `sources/RenderSystem/Effects/RTX/RTX.ixx/.cpp`,
`sources/RenderSystem/Materials/universal_material.cpp` (`init_material`, `update_rtx`, `generate_material`).

**How it works now.**
- `RTXPSO` owns one executable state object, `m_dxrStateObject`, linked from collections: one per engine pass, one per
  raygen, and for every material one collection per per-material pass (`RaytracePass::init_for_material`). Each
  collection is a `CreateStateObject` call, which is where the driver compiles DXIL to native code.
- A material registers with `init_material()` during `compile()`. `RTXPSO::update()` runs once per frame from a pool
  task started in `main.cpp` ("RTX update") that the frame waits for.
- New materials: `build_pending_collections()` compiles all their collections on the pool and waits, then
  `add_pending_materials()` calls `HAL::StateObject::add` (D3D12 `AddToStateObject`), then writes the new hit records.
  This path was added in commit "fast mesh loading".
- A changed material (shader edited or reloaded): its `on_change` handler rebuilds its collections under the lock and
  sets `need_full_rebuild`. `update()` then calls `init()`, which rebuilds **every** pass and raygen collection, links
  everything again and runs `init_ids()`.
- Hit records are `{shader identifier, local_addr}` in `hitgroup_ids`; `local_addr` is per material, the identifier
  comes from the hit group's export name (`material_<n>` + pass name).

**What is slow or wrong.**
- **R2/R3, duplicate compiles.** The collections are keyed by material pointer. Materials that share one
  `raytracing_lib` (same generated hit shader, different textures and uniforms, which is most of a scene like the one
  in the captures: 132 materials) each compile their own copies, differing only in export name. On top of that the same
  library is compiled once per per-material pass. Since the hit record already carries `local_addr` separately, the
  identifier can be shared.
- **R4, the frame waits.** All pending collections are compiled inside `update()`; the frame blocks on the task.
- **R5/R6, edits rebuild everything.** One edited material recompiles all engine ray tracing shaders, recreates the
  miss and raygen id buffers and blocks on an upload.
- **R7/R8, lifetime and locking.** Nothing ever erases from `materials`; a destroyed material leaves a dangling key
  that `init_ids()` dereferences on the next full rebuild, and its hit-table range and collections are never freed.
  `update_material()` and `get_index()` use the maps without the lock.

### Task R.1 Measure it

Neither existing capture contains the update (it runs on the frame after the drop finishes).
- Scopes already exist: `rtx_build_pending_collections`, `rtx_add_pending_materials`, `rtx_add_to_state_object`,
  `rtx_write_material_ids`, `d3d12_add_to_state_object`. Add `rtx_full_rebuild` around the `init()` branch in
  `update()`, `rtx_init_ids` in `init_ids()`, and `state_object_create` around the `CreateStateObject` call in
  `StateObject::on_change` (`HAL.D3D12.PipelineState.cpp`).
- Find how the "drop asset" capture is started and stopped (search `main.cpp` for the capture name) and extend it, or
  add a second capture, so it covers the frames up to the first `RTX update` that does work. Ask the user to drop the
  same asset, and also to edit one material's graph once (that exercises the full rebuild).

**Done when:** the report gives ms for: collections build, add, full rebuild, per-`CreateStateObject` average, and how
many distinct `raytracing_lib` pointers the 132 materials have (log the count once from `build_pending_collections`
into a `.temp` file, and list it for removal).

### Task R.2 Share collections between materials with the same library

**Change** (`RTX.ixx`):
- Key the collections by `HAL::library_shader*`, not by `material*`: a `std::map<library_shader*, shared_group>` where
  `shared_group` holds the collections, a use count, and a name `lib_<n>` that replaces `mat->wshader_name` in export
  and hit-group names. `material_info` keeps a pointer to its group.
- `init_for_material` becomes `init_for_library(rtx, library, name)`; `init_material_ids` looks the identifier up by
  the group's name and still writes `mat->local_addr`.
- Build **one** collection per library containing the exports and hit groups of all per-material passes, instead of
  one collection per pass. Check first that the passes export different functions of the same library
  (`Desc::hit_name`, `Desc::any_hit_name`); if two passes export the same function under different names, DXIL allows
  that with two `DefineExport` entries: keep it in one collection.
- `material::wshader_name` stays only if something outside `RTX.ixx` reads it (search); otherwise remove it.

**Risk to check before coding:** `local_addr` must be the only per-material input a hit shader has. Read how
`local_addr` reaches the shader (see the memory of the earlier bug: the local root signature's register is taken from
`T::Slot::ID`) and confirm nothing per-material is baked into the library through macros other than what already makes
the library text differ.

**Tests:** scene-parity tests on both backends. User check: ray traced reflections show the correct, different
materials on neighbouring objects (a shared identifier with a wrong `local_addr` shows as every object wearing one
material).

**Done when:** the number of per-material collections built equals the number of distinct libraries.

### Task R.3 Build collections when the material loads, not in the frame

- Start the group's collection build as a pool task when the first material with that library registers
  (`init_material`), outside the lock; store a `std::shared_future`. Use the asset lane for it during loads.
- `update()` adds only the groups whose future is ready (no waiting). Materials whose group is not in the state object
  yet must still be safe to hit: until then write a **fallback** identifier into their hit records. Use the
  non-per-material hit group of the same pass if one exists; if none does, **stop and ask**, since the alternatives
  (a tiny built-in "pending material" hit shader, or keeping such instances out of the TLAS) are a design choice.
- When a group lands, rewrite the hit records of all its materials (`write_material_ids`).

**Done when:** `RTX update` stays under 2 ms in frames where materials are loading, except the frame that calls
`AddToStateObject`, and that call's time is reported.

### Task R.4 A material edit is an add, not a rebuild

- On a material's `on_change` with a new library: release its old group (use count), take or create the group of the
  new library, and go through the add path of R.3. No `need_full_rebuild`.
- Keep the full rebuild only for engine shader hot-reload (a pass or raygen library changed; that already arrives
  through `StateObject`'s own `event_change`), and make that path rebuild only the collection whose library changed
  (`init()` currently re-inits every pass and raygen).
- Groups with use count 0 stay in the state object (it cannot shrink). Count them; when they exceed a threshold
  (start at 64, a named constant), schedule one background relink from the live collections and swap it in. The swap
  must rewrite **all** hit, miss and raygen ids, because identifiers from the old object are not valid in the new one.
- `init_ids()`: create `miss_ids` / `raygen_ids` once (their counts are fixed by the template arguments), and update
  their contents through the frame's list in `prepare()` like `hitgroup_ids`, not with `execute_and_wait`.

**Tests:** scene-parity. User check: open a material graph, change a node, confirm the ray traced result updates and
the edit no longer hitches (compare the `RTX update` time from R.1).

### Task R.5 Lifetime and locking

- Add `RTXPSO::remove_material(material*)`: unregister the `on_change` handler, free the `hitgroup_ids` range, drop the
  group's use count, erase from `materials` / `new_materials`. Call it from the material's teardown. There is no
  `~universal_material` today: find where a material asset is destroyed or unloaded and whether `RTX` can already be
  gone at that point (`RTX::is_good()`), as `Streamed::~Streamed` does for its manager.
- A hit-table range must not be reused while frames in flight can still index it: free it through the same deferred
  path other per-frame GPU data uses (look at how `universal_rtx_manager` or `virtual_gpu_buffer::free` users handle
  this; if nothing defers, **ask**).
- Take the lock in `update_material()` and `get_index()`, and make `update_material()` look in `new_materials` too
  instead of inserting an empty entry.

---

## What needs a measurement first, and what does not

Do not hold the whole plan back for instrumentation. Only two groups of tasks wait on numbers.

**Start without measuring.**
- Already measured in the existing captures: L.3 (whole texture in `compile()`), L.4 (BLAS), L.5 (`acquire_many`),
  1.3 (per-request DirectStorage), 1.7 (per-tile mapping), 2.2 (texture feedback readback).
- Wrong regardless of how long they take: 1.1 (root budget), 1.4 (loads under the streamer lock), R.5 (materials never
  removed, unlocked map access).

**Wait on a measurement.**
- **L.2** waits on L.1. The capture shows 18.7 s inside `material_ser_pipeline` with no scopes inside it; the two
  suspected causes (duplicate PSO builds, the spinning `Cache` wait) cannot be told apart yet.
- **R.2, R.3, R.4** wait on R.1. The state object update runs on the frame after the drop finishes, outside the
  existing capture window.

**The measuring steps are small and change no behaviour:** a few `PROFILE` scopes (L.1) and a longer capture window
(R.1). Do both in one change so the user builds and captures once.

**Shortcuts the user may take; offer them, do not decide for them.**
- For R.1, the scopes `rtx_build_pending_collections` and `rtx_add_to_state_object` already exist. If the user reads
  the `RTX update` time from the live profiler on the frame after a drop and reports it, treat that as R.1's timing
  result and skip the capture change. The count of distinct hit libraries is still needed before R.2; get it with the
  one-off `.temp` log R.1 describes.
- L.2 may be done without L.1 if the user says so. Both of its fixes are correct on their own (the spin wait can hang
  when the creator throws; deduping before deserializing is strictly less work). If you skip L.1, say in the report
  that the expected gain is unconfirmed, and still add L.1's scopes in the same change so the next capture shows where
  any remaining time goes.

---

## Order and checkpoints

The table is one valid order, not the only one: anything in the "start without measuring" list above can be pulled
forward, as long as L.5 stays after 1.1 and each task's own "after task X" note is respected.

| Step | Task | Risk | Needs regen / project gen |
|---|---|---|---|
| L1 | L.1 instrument pipeline load, together with R.1 | low | no |
| L2 | L.2 pipeline dedupe + cache wait | medium (asset format) | no |
| L3 | L.3 no whole texture in `compile()` | low | no |
| L4 | L.4 batched BLAS | medium | no |
| R1 | R.1 measure the state object update | low | no |
| R2 | R.2 share collections per library | medium | no |
| R3 | R.3 build collections at load (**may ask**) | high | no |
| R4 | R.4 edit is an add | high | no |
| R5 | R.5 lifetime and locking (**may ask**) | medium | no |
| 1 | 1.1 root budget | low | no |
| L5 | L.5 `acquire_many` | medium | no |
| 2 | 1.2 capability bit | low | no |
| 3 | 1.3 batched DirectStorage | medium | no |
| 4 | 1.4 loads off the lock | high | no |
| 5 | 1.5 batched table writes | medium | no |
| 6 | 1.6 Vulkan fallback IO | medium, **ask first** | maybe (new port) |
| 7 | 1.7 batched tile mapping | medium | no |
| 8 | 1.8 idle sleep | low | no |
| 9 | 2.1 shared compaction | medium | no |
| 10 | 2.2 texture feedback | high | Prism regen, shaders |
| 11 | Phase 3 | high, **ask first** | yes |

After each step: stop, report (five sections), wait for the user's build and test result.

## Things that are easy to get wrong

- A changed `.ixx` layout with a stale module BMI crashes at startup with the same fault offset across builds. If the
  user reports that after a header-ish change, ask for a Rebuild of the affected project before debugging.
- `Streamer` is created with `make_shared` and its callbacks hold it weakly. Any new callback handed to a command list
  or to DirectStorage must do the same, and must check the singleton with `is_good()` if it touches one.
- Slots freed by `hide`/`remove` go through `retiring` and are reusable only after the frame's list completes. Never
  push a slot straight to `free_slots`.
- A table write and the data it points at must be recorded in the same `prepare()` list, data first. `show()` is what
  makes a page visible; nothing else may write `page_slots` for a streamed page.
- `page_slots` values are dword offsets into `pages`, not slot numbers.
- UAV tile data and indirect-argument data must not share a D3D12 buffer (existing rule; relevant if you are tempted
  to reuse `stream_out`).
- Non-uniform bindless indexing needs `NonUniformResourceIndex()` (AMD).
- A new member in a Prism table that is filled by hand in several places needs every fill site updated, or the table
  asserts at bind time.
