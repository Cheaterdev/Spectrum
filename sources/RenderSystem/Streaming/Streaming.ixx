export module Graphics:Streaming;

import Core;
import HAL;

// The resource-independent half of a page streamer. What it is made of does not matter to it: a resource is a run of
// consecutive page ids, each page sits in one slot of a fixed pool or in none, and the GPU reports which pages it
// wanted and which it reached. From those reports the streamer, on its own thread:
//   - loads the wanted pages that are missing, most wanted first, each behind the pages it depends on (its parents);
//   - shows a loaded page only once its parents are shown, and hides pages, least recently reached first, when the
//     pool is full;
//   - hands a freed slot back only after the frames that may still read it are done.
// What a page is, where a slot lives, how bytes get into it and how the shaders find it is the Backend's business.
//
// Every method needs the streamer's mutex() held unless it says otherwise (submit, stop and stats take what they
// need themselves); the backend is called with it held, except Backend::start_loads.

export namespace Streaming
{
	constexpr uint32_t InvalidSlot = 0xFFFFFFFFu;

	// One page of a streamed resource.
	struct PageState
	{
		enum class Status : uint8_t { Missing, Loading, Resident };

		Status   status = Status::Missing;
		uint32_t slot = InvalidSlot;             // pool slot while Loading / Resident
		uint32_t last_touched = 0;               // the last epoch the draw reached this page
		uint32_t resident_children = 0;          // shown pages that list this one as a parent
		std::vector<uint32_t> parents;           // pages that must be shown before this one (resource-local)
	};

	// A run of page ids the streamer manages together. The client fills `states` (and each page's parents) before add().
	class Resource
	{
	public:
		std::vector<PageState> states;       // guarded by the streamer's mutex
		// Whether its pages load on demand and can be evicted. Otherwise every page is a root: loaded at once, kept.
		bool streamed = false;
		uint32_t loading_pages = 0;          // of its pages, how many the streamer has loading; guarded by its mutex
		std::atomic<uint32_t> generation{ 0 };

		virtual ~Resource() = default;

		virtual uint64_t page_id_base() const = 0;
		// A root page is always resident, never evicted: what a missing page falls back to.
		virtual bool is_root(uint32_t page) const = 0;

		// Bumped by the prepare() that applies a change to which pages the draw can see. Whoever caches what the
		// draw produced redraws when it moves.
		uint32_t cut_generation() const { return generation.load(std::memory_order_relaxed); }
	};

	// What the GPU asked for in one frame, whatever way it was transported.
	struct Want
	{
		uint32_t id;
		uint32_t priority;   // higher is loaded first
	};

	// A resident page the GPU found idle: not reached for a while, and flagged evictable when it looked.
	struct Victim
	{
		uint32_t id;
		uint32_t last_touched;   // the last epoch the draw reached the page, or it was shown
	};

	struct Requests
	{
		uint32_t epoch = 0;
		std::vector<Want> wanted;     // pages the draw wanted this epoch and that were neither resident nor loading
		std::vector<Victim> victims;  // candidates to hide when the pool is full; any order
		uint32_t dropped = 0;         // wanted pages that did not fit in the request list
	};

	// What the GPU is told about a page (Backend::set_flags), so that it can compact the feedback itself.
	constexpr uint32_t FlagLoading   = 1;   // on its way: no need to ask for it again
	constexpr uint32_t FlagEvictable = 2;   // resident, not a root, and no shown page depends on it

	struct Stats
	{
		uint32_t pool_pages = 0;
		uint32_t free_pages = 0;
		uint32_t loading_pages = 0;
		uint32_t wanted = 0;        // pages the latest snapshot asked for that were not there
		uint64_t loads = 0;
		uint64_t file_loads = 0;    // of those, read from the asset file by DirectStorage
		uint64_t evictions = 0;
		uint64_t snapshots = 0;
		uint64_t dropped = 0;       // wanted pages the GPU's request list had no room for
	};

	struct Policy
	{
		// Pages loaded per request set at most: a burst of requests would otherwise stall the frame.
		size_t max_loads_per_snapshot = 64;
		// A page the draw last reached this many frames ago is fair game for eviction.
		uint32_t eviction_age = 3;
	};

	class Backend
	{
	public:
		// One page on its way into its slot: made by plan_load, started by start_loads.
		struct LoadPlan
		{
			bool from_file = false;   // done when `waiter` is; otherwise the bytes are queued and show with the next prepare()
			HAL::FenceWaiter waiter;

			// For a load that needs a step of its own before its read can start (a tile mapped first): polled instead of
			// `waiter`, true when the load is done. It runs on the streamer thread with the streamer's mutex held.
			std::function<bool()> poll;

			// The backend's own, for start_loads and finish_loads. It must keep alive everything the IO touches (the GPU
			// resource, the file, the mesh): the resource can be removed from the streamer while the IO is in flight.
			std::shared_ptr<void> backend_data;
		};

		virtual ~Backend() = default;

		// Appends `count` slots to the pool; slot numbers continue from the last one.
		virtual void allocate_pool(uint32_t count) = 0;

		// A backend whose pool follows demand (Streamer::use_demand_pool) gets slots in small chunks as loads need them,
		// gives a free slot's memory back, and takes memory for it again when the pool grows past the slots that are left.
		// The slot number stays reserved meanwhile.
		virtual void release_slot(uint32_t slot) { ASSERT(false); }
		virtual void restore_slot(uint32_t slot) { ASSERT(false); }

		// Putting a page's bytes into its slot, in three steps. allow_file is false for a page that is shown at once (a root).
		//   plan_load     mutex held: decides where the bytes come from and takes what the IO needs. Must not touch the
		//                 disk, decompress, or execute a command list.
		//   start_loads   mutex not held, on the streamer thread: starts the IO of a batch of plans; may set their waiter
		//                 and poll. Must not touch what the mutex guards. (load_root calls it with the mutex held.)
		//   finish_loads  mutex held, right after: puts what start_loads produced into guarded state (queued uploads,
		//                 ops). Gets only the plans whose resource is still under management.
		virtual LoadPlan plan_load(Resource& resource, uint32_t page, uint32_t slot, bool allow_file) = 0;
		virtual void start_loads(std::span<LoadPlan> plans) = 0;
		virtual void finish_loads(std::span<LoadPlan> plans) {}

		// Makes the loaded page visible to the shaders, or hides it. These write the page table.
		virtual void show(Resource& resource, uint32_t page, uint32_t slot) = 0;
		virtual void hide(Resource& resource, uint32_t page) = 0;

		// The page's Flag* bits, and the epoch it was last shown at, for the GPU's compaction.
		virtual void set_flags(Resource& resource, uint32_t page, uint32_t flags, uint32_t shown_epoch) = 0;
	};

	// Created with std::make_shared: the completion callbacks it hands to command lists hold it weakly.
	class Streamer : public std::enable_shared_from_this<Streamer>
	{
		struct PageRef
		{
			Resource* resource = nullptr;
			uint32_t  page = 0;
		};

		struct Loading
		{
			Resource* resource;
			uint32_t  page;
			uint64_t  ticket;          // which plan it is of the batch process() has in start_loads
			bool      started;         // false while that batch is in start_loads: not shown before its plan is back
			Backend::LoadPlan load;
		};

		// The slot of a page whose resource was removed while it was loading: its IO may still write the slot, so it is
		// retired only once that IO is done (publish), never straight away.
		struct Orphan
		{
			uint32_t slot;
			uint64_t ticket;
			bool     started;
			Backend::LoadPlan load;
		};

		Backend& backend;
		Policy policy;

		std::mutex m;
		uint32_t current_epoch = 1;

		std::vector<PageRef> page_refs;           // by page id, streamed resources only
		std::vector<Resource*> resources;
		size_t pool_size = 0;                     // slot numbers handed out; released_slots are among them without memory
		std::vector<uint32_t> released_slots;
		bool on_demand = false;
		std::vector<uint32_t> free_slots;
		std::vector<uint32_t> retiring;           // hidden since the last prepare, free once the frames that may read them are done
		size_t retiring_inflight = 0;             // slots handed to a list's completion callback
		std::vector<Loading> loading;             // any order
		std::vector<Orphan> orphans;
		uint64_t next_ticket = 0;
		std::unordered_set<Resource*> changed;    // resources whose visible pages changed since the last prepare

		bool pool_mapped_flag = false;            // the pool's memory is ready on the GPU: DirectStorage may write the slots
		bool pool_map_pending = false;
		uint32_t pool_version = 0;                // bumped when the pool grows: a mapping callback is only good for the version it saw

		uint64_t loads = 0;
		uint64_t file_loads = 0;
		uint64_t evictions = 0;
		uint64_t requests_done = 0;
		uint32_t last_wanted = 0;

		std::mutex queue_mutex;
		std::condition_variable wake;
		std::deque<Requests> queue;
		bool stopping = false;
		std::thread thread;
		std::atomic<bool> has_loading{ false };   // loading or orphans is non-empty (set under m, read without it by run())

		std::vector<Victim> victims;              // the latest request set's candidates; guarded by m
		uint64_t dropped_requests = 0;

		// Slots the non-root pages of streamed resources may hold together. Roots (streamed_roots) and pinned resources
		// are outside it: they cannot be evicted, so counting them would leave no room once there are enough of them.
		uint32_t budget = 0xFFFFFFFFu;
		uint32_t streamed_in_use = 0;             // slots held by streamed resources, roots included
		uint32_t streamed_roots = 0;              // of those, the roots'

		uint32_t evictable_in_use() const { return streamed_in_use - streamed_roots; }

		void sync_flags(Resource& resource, uint32_t page);
		uint32_t take_slot();
		void grow(uint32_t count);
		void run();
		void process(const Requests& requests);
		bool plan_loads(const Requests& requests, std::vector<Backend::LoadPlan>& plans, uint64_t& first_ticket);
		void finish_loads(std::vector<Backend::LoadPlan>& plans, uint64_t first_ticket);
		void publish();
		// `any` also takes pages the draw still wants: for a pool that is now over its budget (it was lowered).
		void evict(size_t count, uint32_t epoch, bool any = false);

	public:
		Streamer(Backend& backend, uint32_t max_page_ids, Policy policy = {});
		~Streamer();

		std::mutex& mutex() { return m; }

		// Any thread, no lock needed.
		void submit(Requests&& requests);
		void stop();

		// Starts the streamer thread (once).
		void start();
		bool running() const { return thread.joinable(); }

		// The pool has at least `count` slots with memory: it grows (shrinking is trim_pool()).
		void ensure_pool(uint32_t count);

		// How many slots the non-root pages of streamed resources may hold: past it, wanted pages load only by evicting
		// others. The pool holds the roots and pinned pages on top of it.
		void set_budget(uint32_t slots) { budget = slots; }

		// Slots the roots of streamed resources hold: always resident, outside the budget.
		uint32_t root_slots() const { return streamed_roots; }

		// The pool holds memory only for what is loaded, plus a little slack: slots are made in chunks when a load finds
		// none free (up to the budget), and free slots beyond the slack are given back. Off by default, where the owner
		// sizes the pool with ensure_pool(). Call trim_pool() after lowering the budget; it also runs by itself whenever
		// evicted slots become free.
		void use_demand_pool() { on_demand = true; }
		void trim_pool();

		// At least `count` slots are free now. Slots a released resource gave back are not, until the frames that may
		// read them are done, so a resource arriving right after another may need the pool to grow past its budget.
		void ensure_free(uint32_t count);

		// Takes the resource's pages under management (its states are sized), or lets go of them. A resource that is
		// not `streamed` is pinned: all its pages are loaded with load_root() and none is ever evicted.
		void add(Resource& resource);
		void remove(Resource& resource);

		// Loads a page into a slot and counts it shown at once: a root page. Returns the slot. All three load steps run
		// here with the mutex held, file read included: roots load where a resource is acquired or created, not on the
		// streamer thread (acquire_many reads them ahead in parallel).
		uint32_t load_root(Resource& resource, uint32_t page);

		// Identifies the frame the draws record; advanced once per frame by the owner before prepare().
		uint32_t epoch() const { return current_epoch; }
		void advance_epoch() { current_epoch++; }

		bool pool_mapped() const { return pool_mapped_flag; }

		// Once per frame, after the backend's own buffers are prepared: applies what changed to the generations and
		// hands the list the callbacks that map the pool and free retired slots.
		void prepare(HAL::CommandList::ptr& list);

		Stats stats();

		const Policy& get_policy() const { return policy; }
	};

	// The GPU side of the feedback, shared by the clients: the draws write what they wanted and reached per page id
	// into feedback() (two dwords per page id, see shaders/streaming/stream_compact.hlsl), the streamer's flags per page
	// go the other way, and prepare() compacts it on the GPU into the short lists that are read back and handed to the
	// streamer. Everything but feedback() is guarded by the streamer's mutex, like the Backend calls that feed it.
	class GpuFeedback
	{
		struct FlagWrite
		{
			uint32_t first;
			std::vector<uint32_t> dwords;   // two per page id
		};

		std::weak_ptr<Streamer> streamer;
		HAL::StructuredBufferView<uint> wanted;
		HAL::StructuredBufferView<uint> page_flags;
		HAL::StructuredBufferView<uint> stream_out;

		// Ranges (zeroed when a resource gets its page ids) are written first, then the singles, so a single recorded
		// after a range wins over it. A range drops the singles under it: they belong to whoever held those ids before.
		std::vector<FlagWrite> flag_writes;
		std::vector<std::pair<uint32_t, std::array<uint32_t, 2>>> flag_singles;   // page id, its two dwords

	public:
		GpuFeedback(HAL::Device& device, uint32_t max_page_ids, std::weak_ptr<Streamer> streamer);

		// What the draws write into; stays on the GPU.
		HAL::StructuredBufferView<uint>& feedback() { return wanted; }

		// The Flag* bits of a page id and the epoch it was shown at (Backend::set_flags).
		void set_flags(uint32_t page_id, uint32_t flags, uint32_t shown_epoch);

		// Zeroes the flags and shown epochs of `count` page ids from `first`.
		void clear_range(uint32_t first, uint32_t count);

		// Records the queued flag writes, the compaction over the first `page_count` page ids and the readback that submits
		// to the streamer. `epoch` is the latest one the feedback holds; `page_slots` is the client's page table.
		void prepare(HAL::CommandList::ptr& list, const HAL::StructuredBufferView<uint>& page_slots, uint32_t page_count,
			uint32_t epoch, uint32_t eviction_age);
	};
}
