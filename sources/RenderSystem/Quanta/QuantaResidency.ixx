export module Graphics:QuantaResidency;

import Core;
import HAL;
import :Quanta;

// Quanta on the GPU, resident-only: every page of every acquired mesh sits in one shared buffer for as long as
// the mesh is in use, with no eviction. This is the step before streaming -- the tables and addressing are
// what streaming will keep, only "which pages are in the buffer" becomes dynamic.
//
// Three buffers of dwords, shared by all meshes (virtual_gpu_buffer, like universal_nodes_manager):
//   pages   PageDwords per page; a group's blob starts at group.page + group.offset_in_page / 4
//   groups  Quanta::Group records, `page` rewritten to the page's dword offset in `pages`
//   nodes   Quanta::Node records, `first_group` rewritten to an index into the shared `groups`
// Shaders read them as plain dwords and lay the structs over them.

export namespace Quanta
{
	constexpr uint32_t PageDwords  = PageBytes / sizeof(uint32_t);
	constexpr uint32_t GroupDwords = sizeof(Group) / sizeof(uint32_t);
	constexpr uint32_t NodeDwords  = sizeof(Node) / sizeof(uint32_t);
	static_assert(sizeof(Group) % sizeof(uint32_t) == 0 && sizeof(Node) % sizeof(uint32_t) == 0);

	// One mesh's copy in the shared buffers. Frees its ranges when the last user lets go.
	class Resident
	{
		friend class Residency;

		TypedHandle<uint> page_handle;
		TypedHandle<uint> group_handle;
		TypedHandle<uint> node_handle;

	public:
		std::shared_ptr<Mesh> mesh;

		uint64_t page_offset() const { return page_handle.get_offset(); }                    // dwords into `pages`
		uint64_t group_offset() const { return group_handle.get_offset() / GroupDwords; }   // records into `groups`
		uint64_t node_offset() const { return node_handle.get_offset() / NodeDwords; }      // records into `nodes`

		~Resident();
	};

	class Residency : public Singleton<Residency>
	{
		friend class Resident;

		static constexpr size_t MaxPageDwords  = (1024ull * 1024 * 1024) / sizeof(uint32_t);
		static constexpr size_t MaxGroupDwords = (64ull * 1024 * 1024) / sizeof(uint32_t);
		static constexpr size_t MaxNodeDwords  = (16ull * 1024 * 1024) / sizeof(uint32_t);

		std::mutex m;
		std::unordered_map<const Mesh*, std::weak_ptr<Resident>> residents;
		std::unordered_map<const Mesh*, std::shared_ptr<Resident>> pinned;

	public:
		HAL::virtual_gpu_buffer<uint> pages;
		HAL::virtual_gpu_buffer<uint> groups;
		HAL::virtual_gpu_buffer<uint> nodes;

		Residency();

		// The mesh's GPU copy, uploading it on first use. The upload is queued: prepare() records it.
		std::shared_ptr<Resident> acquire(const std::shared_ptr<Mesh>& mesh);

		// acquire(), kept alive until the singleton goes: for a user (a render pass) with no object of its own
		// to own the copy, so it is not freed and re-uploaded every frame.
		std::shared_ptr<Resident> pin(const std::shared_ptr<Mesh>& mesh);

		// Maps newly allocated ranges and records the queued uploads. Once per frame, before the buffers are read.
		void prepare(HAL::CommandList::ptr& list);
	};
}
