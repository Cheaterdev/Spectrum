export module Graphics:QuantaResidency;

import Core;
import HAL;
import :Quanta;
import :Streaming;

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
// Every mesh's pages sit in slots of one pool inside `pages`, and the Streaming::Streamer manages them. Two policies:
//   pinned    (the default) every page is loaded when the mesh is acquired and stays for as long as it is in use;
//   streamed  (set_streaming(true) before the mesh is acquired) only the root pages are. The draw reports the
//             pages it wanted but lacked and the ones it reached (QuantaFeedback in meshrender.prism); the
//             feedback is read back each frame, and a Streaming::Streamer turns it into loads and evictions in a
//             fixed pool of page slots. Residency is that streamer's backend: it owns the buffers, writes the page
//             table and reads the pages from the asset file. Loads become visible to the draw only through the page
//             table, written under the lock prepare() takes, so a frame sees either all of a prepare's changes or none.

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

	using PageState = Streaming::PageState;
	using StreamStats = Streaming::Stats;

	// One mesh's copy in the shared buffers. Frees its ranges when the last user lets go.
	class Resident : public Streaming::Resource
	{
		friend class Residency;

		TypedHandle<uint> page_id_handle;
		TypedHandle<uint> group_handle;
		TypedHandle<uint> node_handle;

	public:
		std::shared_ptr<Mesh> mesh;

		bool is_streamed() const { return streamed; }

		uint64_t page_id_base() const override { return page_id_handle.get_offset(); }       // page ids, one per page
		uint64_t group_offset() const { return group_handle.get_offset() / GroupDwords; }   // records into `groups`
		uint64_t node_offset() const { return node_handle.get_offset() / NodeDwords; }      // records into `nodes`

		// A mesh that is not streamed is pinned: every page of it is a root.
		bool is_root(uint32_t page) const override { return !streamed || mesh->pages[page].root; }

		~Resident() override;
	};

	class Residency : public Singleton<Residency>, private Streaming::Backend
	{
		friend class Resident;

		// Virtual reservations: memory is committed only for what is allocated, so these are address space, not VRAM.
		// The pages buffer stays one page under 4 GB so a byte range into it fits 32 bits (Vulkan's storage buffer limit).
		static constexpr size_t MaxPageDwords  = (4096ull * 1024 * 1024 - PageBytes) / sizeof(uint32_t);
		// A family's first group is packed into FamilyCountShift bits, so the shared group table cannot hold more.
		static constexpr size_t MaxGroups      = size_t(1) << FamilyCountShift;
		static constexpr size_t MaxGroupDwords = MaxGroups * GroupDwords;
		static constexpr size_t MaxNodes       = MaxGroups / 8;
		static constexpr size_t MaxNodeDwords  = MaxNodes * NodeDwords;
		static constexpr size_t MaxPageIds     = 1u << 20;
		static constexpr size_t MaxPoolPages   = MaxPageDwords / PageDwords;

		std::shared_ptr<Streaming::Streamer> streamer;

		// The streamer's: guards everything below as well as its own state, tables, the pool and the page-table writes.
		std::mutex& m;
		std::unordered_map<const Mesh*, std::weak_ptr<Resident>> residents;
		std::unordered_map<const Mesh*, std::shared_ptr<Resident>> pinned;

		uint32_t max_page_id = 0;                 // one past the highest page id handed out: the feedback to read back

		// The slots, as dword offsets into `pages`; the streamer grows it as pinned and streamed meshes arrive. Each growth
		// is one range of `pages` (kept in pool_ranges) cut into PageDwords-sized slots. Slots are never freed one by one:
		// that is the demand pool, which only the texture backend uses (release_slot / restore_slot are not overridden here).
		std::vector<uint64_t> pool;
		std::vector<TypedHandle<uint>> pool_ranges;

		// What the pool has to hold: every page of the pinned meshes, the streamed meshes' roots (Streamer::root_slots),
		// plus the budget the streamed ones share for the rest of their pages.
		uint32_t pinned_pages = 0;
		uint32_t streamed_meshes = 0;

		// What the shared tables hold now, against the caps above: a mesh that does not fit is refused before anything changes.
		uint64_t page_ids_in_use = 0;
		uint64_t groups_in_use = 0;
		uint64_t nodes_in_use = 0;

		// The slots the streamed meshes' non-root pages share: the setting, cut to what the pages buffer has left after
		// the pinned pages and the streamed roots.
		uint32_t streaming_budget() const;

		// Root pages that acquire_many() read ahead, in parallel, for plan_load() to take instead of reading the
		// file one page at a time. Sorted by (mesh, page) for lookup; guarded by m; empty outside an acquire_many().
		std::vector<PageFetch> prefetched;

		// The flags of page ids changed since the last prepare(). Ranges (a mesh's whole run of page ids, zeroed when it
		// is acquired) are written first, then the single entries, so a single recorded after a range wins over it.
		// A range drops the singles under it: they belong to whoever held those ids before.
		struct FlagWrite
		{
			uint32_t first;
			std::vector<uint32_t> dwords;   // two per page id
		};
		std::vector<FlagWrite> flag_writes;
		std::vector<std::pair<uint32_t, std::array<uint32_t, 2>>> flag_singles;   // page id, its two dwords

		void release(Resident& resident);
		void find_parents(Resident& resident);

		// Streaming::Backend
		void allocate_pool(uint32_t count) override;
		Streaming::Backend::LoadPlan plan_load(Streaming::Resource& resource, uint32_t page, uint32_t slot, bool allow_file) override;
		void start_loads(std::span<Streaming::Backend::LoadPlan> plans) override;
		void finish_loads(std::span<Streaming::Backend::LoadPlan> plans) override;
		void write_page(uint64_t dword_offset, const std::vector<uint8_t>& bytes);
		void show(Streaming::Resource& resource, uint32_t page, uint32_t slot) override;
		void hide(Streaming::Resource& resource, uint32_t page) override;
		void set_flags(Streaming::Resource& resource, uint32_t page, uint32_t flags, uint32_t shown_epoch) override;

	public:
		HAL::virtual_gpu_buffer<uint> pages;
		HAL::virtual_gpu_buffer<uint> groups;
		HAL::virtual_gpu_buffer<uint> nodes;
		HAL::virtual_gpu_buffer<uint> page_slots;

		// What the Quanta draw reports (QuantaFeedback in meshrender.prism), two dwords per page id. It stays on the
		// GPU: prepare() compacts it into stream_out, which is what is read back.
		HAL::StructuredBufferView<uint> feedback;

		// Per page id, two dwords the streamer writes: Streaming::Flag* bits, and the epoch the page was shown at.
		HAL::StructuredBufferView<uint> page_flags;
		HAL::StructuredBufferView<uint> stream_out;

		Residency();
		~Residency() override;

		// The mesh's GPU copy, uploading it on first use. The upload is queued: prepare() records it.
		std::shared_ptr<Resident> acquire(const std::shared_ptr<Mesh>& mesh);

		// acquire() for a whole set (an instance's parts), much cheaper than one call each: the root pages are read
		// with one file open per asset file, decompressed in parallel on the thread pool, and the page pool grows once. Aligned with
		// `meshes`; a mesh that is refused (out of room, as acquire() throws for it) comes back null and is logged.
		std::vector<std::shared_ptr<Resident>> acquire_many(const std::vector<std::shared_ptr<Mesh>>& meshes);

		// acquire(), kept alive until the singleton goes: for a user (a render pass) with no object of its own
		// to own the copy, so it is not freed and re-uploaded every frame.
		std::shared_ptr<Resident> pin(const std::shared_ptr<Mesh>& mesh);

		// Makes one page of a pinned (not streamed) mesh visible to, or hides it from, the shaders. Queued like
		// the uploads, recorded by prepare(); a test hook for what the streamer does on its own.
		void set_page_present(const Resident& resident, uint32_t page, bool present);

		// Where a loaded page sits in `pages`, in dwords.
		uint64_t page_dword_offset(const Resident& resident, uint32_t page);

		// Identifies the frame the draws record: written into the feedback so stale entries can be told apart.
		uint32_t epoch() const { return streamer->epoch(); }

		StreamStats stats();

		// Maps newly allocated ranges and records the queued uploads, retires hidden slots and reads the feedback
		// back. Once per frame, before the buffers are read.
		void prepare(HAL::CommandList::ptr& list);
	};
}
