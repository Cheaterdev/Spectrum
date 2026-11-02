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
// need themselves); the backend is called with it held.

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
		struct Load
		{
			bool from_file = false;   // done when `waiter` is; otherwise the bytes are queued and show with the next prepare()
			HAL::FenceWaiter waiter;
		};

		virtual ~Backend() = default;

		// Appends `count` slots to the pool; slot numbers continue from the last one.
		virtual void allocate_pool(uint32_t count) = 0;

		// Starts putting a page's bytes into its slot. allow_file is false for a page that is shown at once.
		virtual Load begin_load(Resource& resource, uint32_t page, uint32_t slot, bool allow_file) = 0;

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
			Backend::Load load;
		};

		Backend& backend;
		Policy policy;

		std::mutex m;
		uint32_t current_epoch = 1;

		std::vector<PageRef> page_refs;           // by page id, streamed resources only
		std::vector<Resource*> resources;
		size_t pool_size = 0;
		std::vector<uint32_t> free_slots;
		std::vector<uint32_t> retiring;           // hidden since the last prepare, free once the frames that may read them are done
		size_t retiring_inflight = 0;             // slots handed to a list's completion callback
		std::vector<Loading> loading;
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

		std::vector<Victim> victims;              // the latest request set's candidates; guarded by m
		uint64_t dropped_requests = 0;

		// Slots the pages of streamed resources may hold together, roots included; pinned resources are outside it.
		uint32_t budget = 0xFFFFFFFFu;
		uint32_t streamed_in_use = 0;

		void sync_flags(Resource& resource, uint32_t page);
		uint32_t take_slot();
		void run();
		void process(const Requests& requests);
		void publish();
		void evict(size_t count, uint32_t epoch);

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

		// The pool has at least `count` slots: it grows, and never shrinks.
		void ensure_pool(uint32_t count);

		// How many slots streamed resources may hold: past it, wanted pages load only by evicting others. The pool
		// itself can be larger (it only grows), so this is what makes a smaller setting take effect.
		void set_budget(uint32_t slots) { budget = slots; }

		// At least `count` slots are free now. Slots a released resource gave back are not, until the frames that may
		// read them are done, so a resource arriving right after another may need the pool to grow past its budget.
		void ensure_free(uint32_t count);

		// Takes the resource's pages under management (its states are sized), or lets go of them. A resource that is
		// not `streamed` is pinned: all its pages are loaded with load_root() and none is ever evicted.
		void add(Resource& resource);
		void remove(Resource& resource);

		// Loads a page into a slot and counts it shown at once: a root page. Returns the slot.
		uint32_t load_root(Resource& resource, uint32_t page);

		// Identifies the frame the draws record; advanced once per frame by the owner before prepare().
		uint32_t epoch() const { return current_epoch; }
		void advance_epoch() { current_epoch++; }

		bool pool_mapped() const { return pool_mapped_flag; }

		// Once per frame, after the backend's own buffers are prepared: applies what changed to the generations and
		// hands the list the callbacks that map the pool and free retired slots.
		void prepare(HAL::CommandList::ptr& list);

		Stats stats();
	};
}
