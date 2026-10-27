export module Graphics:QuantaResidency;

import Core;
import HAL;
import :Quanta;

// Quanta on the GPU: every mesh's group and node tables sit in shared buffers for as long as the mesh is in use,
// and its pages go through a page table, so which pages are in the pages buffer can change at runtime.
//
// Four buffers of dwords, shared by all meshes (virtual_gpu_buffer, like universal_nodes_manager):
//   pages       a group's blob starts at page_slots[group.page] + group.offset_in_page / 4
//   groups      Quanta::Group records; `page` is a page id, `family` holds global group indices
//   nodes       Quanta::Node records, `first_group` rewritten to an index into the shared `groups`
//   page_slots  per page id: the dword offset of the page in `pages`, or InvalidIndex while it is not resident
// Shaders read them as plain dwords and lay the structs over them.
//
// A page id is global: each mesh owns a consecutive range of them (page_id_base() + the mesh-local page).
//
// Two ways to hold a mesh:
//   resident  (the default) every page is in `pages` for as long as the mesh is in use;
//   streamed  (set_streaming(true) before the mesh is acquired) only the root pages are. The draw reports the
//             pages it wanted but lacked and the ones it reached (QuantaFeedback in meshrender.prism); the
//             feedback is read back each frame, and a streamer thread turns it into loads and evictions in a
//             fixed pool of page slots. Loads become visible to the draw only through the page table, written
//             under the lock prepare() takes, so a frame sees either all of a prepare's changes or none.

export namespace Quanta
{
	constexpr uint32_t PageDwords  = PageBytes / sizeof(uint32_t);
	constexpr uint32_t GroupDwords = sizeof(Group) / sizeof(uint32_t);
	constexpr uint32_t NodeDwords  = sizeof(Node) / sizeof(uint32_t);
	static_assert(sizeof(Group) % sizeof(uint32_t) == 0 && sizeof(Node) % sizeof(uint32_t) == 0);

	// Whether meshes acquired from now on are streamed, and how many page slots the shared pool has (0 keeps the
	// current size; the pool is committed when the first streamed mesh is acquired, so set it before).
	bool streaming_enabled();
	void set_streaming(bool enabled, uint32_t pool_pages = 0);

	// One page of a streamed mesh.
	struct PageState
	{
		enum class Status : uint8_t { Missing, Loading, Resident };

		Status   status = Status::Missing;
		uint32_t slot = InvalidIndex;            // pool slot while Loading / Resident
		uint32_t last_touched = 0;               // the last epoch the draw reached this page
		uint32_t resident_children = 0;          // shown pages that list this one as a parent
		std::vector<uint32_t> parents;           // pages that must be shown before this one (mesh-local)
	};

	// One mesh's copy in the shared buffers. Frees its ranges when the last user lets go.
	class Resident
	{
		friend class Residency;

		TypedHandle<uint> page_handle;       // the page slots of a mesh resident in one piece
		TypedHandle<uint> page_id_handle;
		TypedHandle<uint> group_handle;
		TypedHandle<uint> node_handle;

		bool streamed = false;

	public:
		std::shared_ptr<Mesh> mesh;
		std::vector<PageState> states;       // streamed meshes only; guarded by the Residency lock

		bool is_streamed() const { return streamed; }
		uint64_t page_offset() const { return page_handle.get_offset(); }                    // dwords into `pages` (resident meshes)
		uint64_t page_id_base() const { return page_id_handle.get_offset(); }                // page ids, one per page
		uint64_t group_offset() const { return group_handle.get_offset() / GroupDwords; }   // records into `groups`
		uint64_t node_offset() const { return node_handle.get_offset() / NodeDwords; }      // records into `nodes`

		~Resident();
	};

	struct StreamStats
	{
		uint32_t pool_pages = 0;
		uint32_t free_pages = 0;
		uint32_t loading_pages = 0;
		uint32_t wanted = 0;        // pages the latest snapshot asked for that were not there
		uint64_t loads = 0;
		uint64_t file_loads = 0;    // of those, read from the asset file by DirectStorage
		uint64_t evictions = 0;
		uint64_t snapshots = 0;
	};

	class Residency : public Singleton<Residency>
	{
		friend class Resident;

		static constexpr size_t MaxPageDwords  = (1024ull * 1024 * 1024) / sizeof(uint32_t);
		static constexpr size_t MaxGroupDwords = (64ull * 1024 * 1024) / sizeof(uint32_t);
		static constexpr size_t MaxNodeDwords  = (16ull * 1024 * 1024) / sizeof(uint32_t);
		static constexpr size_t MaxPageIds     = 1u << 18;

		struct PageRef
		{
			Resident* resident = nullptr;
			uint32_t  page = 0;
		};

		struct Loading
		{
			Resident* resident;
			uint32_t  page;
			bool      from_file = false;   // a DirectStorage read into the slot, done when `waiter` is
			HAL::FenceWaiter waiter;
		};

		// The feedback of one frame as the GPU left it: two dwords per page id.
		struct Snapshot
		{
			uint32_t epoch = 0;
			std::vector<uint32_t> data;
		};

		// Guards everything below up to the streamer's own queue: tables, states, the pool, the page-table writes.
		std::mutex m;
		std::unordered_map<const Mesh*, std::weak_ptr<Resident>> residents;
		std::unordered_map<const Mesh*, std::shared_ptr<Resident>> pinned;

		uint32_t current_epoch = 1;
		uint32_t max_page_id = 0;                 // one past the highest page id handed out: the feedback to read back

		std::vector<PageRef> page_refs;           // by page id, streamed meshes only
		std::vector<Resident*> streamed_list;
		std::vector<TypedHandle<uint>> pool;      // the slots, allocated once
		std::vector<uint32_t> free_slots;
		std::vector<uint32_t> retiring;           // hidden since the last prepare, free once the frames that may read them are done
		size_t retiring_inflight = 0;             // slots handed to a list's completion callback
		std::vector<Loading> loading;

		bool pool_mapped = false;                 // the pool's tiles are mapped on the GPU: DirectStorage may write them
		bool pool_map_pending = false;
		uint32_t pool_generation = 0;             // bumped when the pool is rebuilt: slots of an older one mean nothing

		uint64_t loads = 0;
		uint64_t file_loads = 0;
		uint64_t evictions = 0;
		uint64_t snapshots_done = 0;
		uint32_t last_wanted = 0;

		std::mutex snapshot_mutex;
		std::condition_variable wake;
		std::deque<Snapshot> snapshots;
		bool stopping = false;
		std::thread streamer;

		void release(Resident& resident);
		void ensure_pool();
		void find_parents(Resident& resident);
		uint32_t take_slot();
		Loading upload_page(Resident& resident, uint32_t page, uint32_t slot, bool allow_file);
		void run_streamer();
		void process(const Snapshot& snapshot);
		void publish();
		void evict(size_t count, uint32_t epoch);

	public:
		HAL::virtual_gpu_buffer<uint> pages;
		HAL::virtual_gpu_buffer<uint> groups;
		HAL::virtual_gpu_buffer<uint> nodes;
		HAL::virtual_gpu_buffer<uint> page_slots;

		// What the Quanta draw reports (QuantaFeedback in meshrender.prism), two dwords per page id.
		HAL::StructuredBufferView<uint> feedback;

		Residency();
		~Residency() override;

		// The mesh's GPU copy, uploading it on first use. The upload is queued: prepare() records it.
		std::shared_ptr<Resident> acquire(const std::shared_ptr<Mesh>& mesh);

		// acquire(), kept alive until the singleton goes: for a user (a render pass) with no object of its own
		// to own the copy, so it is not freed and re-uploaded every frame.
		std::shared_ptr<Resident> pin(const std::shared_ptr<Mesh>& mesh);

		// Makes one page of a resident (not streamed) mesh visible to, or hides it from, the shaders. Queued like
		// the uploads, recorded by prepare(); a test hook for what the streamer does on its own.
		void set_page_present(const Resident& resident, uint32_t page, bool present);

		// Identifies the frame the draws record: written into the feedback so stale entries can be told apart.
		uint32_t epoch() const { return current_epoch; }

		StreamStats stats();

		// Maps newly allocated ranges and records the queued uploads, retires hidden slots and reads the feedback
		// back. Once per frame, before the buffers are read.
		void prepare(HAL::CommandList::ptr& list);
	};
}
