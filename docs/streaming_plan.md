# Streaming rework plan (Quanta pages + texture tiles)

This is a work order for an implementing model. Do the tasks in order. Each task is small enough to build and test on
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

## Order and checkpoints

| Step | Task | Risk | Needs regen / project gen |
|---|---|---|---|
| 1 | 1.1 root budget | low | no |
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
