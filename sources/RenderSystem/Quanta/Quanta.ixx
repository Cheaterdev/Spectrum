export module Graphics:Quanta;

import Core;

// Quanta: paged, streamable cluster geometry. This partition is the CPU-side format only (no GPU, no
// MeshAsset): a mesh is a cluster-LOD DAG cut into groups, each group a self-contained blob (its clusters'
// own vertices and triangles, not indices into a shared vertex buffer), and the blobs are packed into
// fixed-size pages. Pages are the unit that gets loaded and evicted; the group and hierarchy tables stay
// resident, so a group's LOD data is known even while its blob is not.
//
// The clusterlod build that fills this lives in QuantaBuilder.cpp (a plain TU, like MeshletGeneration.cpp).
// Pack / validate / merge_spheres are defined in Quanta.cpp, not inline: the plain TU that imports this
// module hit an internal compiler error in MSVC's module reader when it had to replay their bodies.

export namespace Quanta
{
	constexpr uint32_t PageBytes           = 256 * 1024;
	constexpr uint32_t MaxClusterVertices  = 64;
	constexpr uint32_t MaxClusterTriangles = 124;
	constexpr uint32_t GroupMaxClusters    = 16;
	constexpr uint32_t NodeMaxGroups       = 32;
	constexpr uint32_t BlobAlignment       = 16;
	constexpr uint32_t InvalidIndex        = 0xFFFFFFFFu;

	// What the vertex shaders read today (mesh_vertex_input): pos, normal, uv, tangent.
	struct Vertex
	{
		float3 position;
		float3 normal;
		float2 uv;
		float4 tangent;
	};
	static_assert(sizeof(Vertex) == 12 * sizeof(float));

	// Start of a group blob. Followed by ClusterHeader[cluster_count], then each cluster's vertices and
	// triangles at the offsets its header gives (bytes from the blob start).
	struct GroupHeader
	{
		uint32_t cluster_count;
		uint32_t size_bytes;
		uint32_t reserved[2];
	};

	struct ClusterHeader
	{
		uint32_t vertex_offset;
		uint32_t triangle_offset;
		uint32_t vertex_count;
		uint32_t triangle_count;
		float4   bounding_sphere;
		uint32_t normal_cone;      // meshopt s8 axis (xyz) and cutoff (w)
		uint32_t lod_level;
		float    refined_error;    // < 0: original geometry (the finest level)
		uint32_t reserved;
		float4   refined_sphere;
	};
	static_assert(sizeof(ClusterHeader) == 64);
	static_assert(std::is_trivially_copyable_v<GroupHeader> && std::is_trivially_copyable_v<ClusterHeader>);
	static_assert(std::is_trivially_copyable_v<Vertex>);

	// Resident record of one group (the LOD test needs it while the blob is paged out).
	struct Group
	{
		float4   lod_sphere;
		float4   refined_sphere;
		float    lod_error;        // FLT_MAX where nothing coarser exists: a root group
		float    refined_error;    // min over the clusters, negative if any is original geometry
		uint32_t cluster_count;
		uint32_t depth;            // 0 is the finest level
		uint32_t page;
		uint32_t offset_in_page;
		uint32_t size_bytes;
		uint32_t reserved;
	};

	static_assert(std::is_trivially_copyable_v<Group>);

	// NodeMaxGroups consecutive groups. Enclosing sphere / max error / min refined error, so a node is only
	// skipped when every group under it would fail its own test.
	struct Node
	{
		float4   lod_sphere;
		float4   refined_sphere;
		float    lod_error;
		float    refined_error;
		uint32_t first_group;
		uint32_t group_count;
	};

	static_assert(std::is_trivially_copyable_v<Node>);

	struct Page
	{
		std::vector<uint8_t>  data;     // <= PageBytes; the runtime pads to PageBytes
		std::vector<uint32_t> groups;
		bool                  root = false;  // holds a root group: pinned, never evicted

		// Where these bytes sit in the asset file they were loaded from (Mesh::file), so a DirectStorage
		// request can read the page straight into the page pool. 0 / 0 for a mesh that was never loaded.
		uint64_t file_offset = 0;
		uint32_t file_size   = 0;
	};

	struct Mesh
	{
		std::vector<Group> groups;
		std::vector<Node>  nodes;
		std::vector<Page>  pages;

		uint32_t original_cluster_count  = 0;
		uint32_t original_triangle_count = 0;
		uint32_t max_depth               = 0;
		uint32_t root_page_count         = 0;

		std::filesystem::path file;      // the asset file the pages came from, empty if built in memory
	};

	struct ClusterInput
	{
		std::vector<uint32_t> unique_vertices;   // ids into the source vertex array
		std::vector<uint8_t>  triangles;         // 3 per triangle, indices into unique_vertices
		float4   bounding_sphere;
		uint32_t normal_cone = 0;
		uint32_t lod_level   = 0;
		float    refined_error = -1;
		float4   refined_sphere;
	};

	struct GroupInput
	{
		std::vector<ClusterInput> clusters;      // <= GroupMaxClusters
		float4   lod_sphere;
		float    lod_error = 0;
		float4   refined_sphere;
		float    refined_error = -1;
		uint32_t depth = 0;
	};

	// Smallest sphere enclosing both.
	float4 merge_spheres(const float4& a, const float4& b);

	// Serializes every group into its blob, builds the hierarchy nodes over consecutive groups, and packs
	// the blobs into pages coarsest level first, so the pages holding root groups come first and are the
	// only ones pinned.
	Mesh Pack(std::vector<GroupInput>&& inputs, const Vertex* vertices, uint32_t original_triangles);

	// Empty string when the mesh is consistent, otherwise the first problem found.
	std::string validate(const Mesh& mesh);

	struct GroupView
	{
		const GroupHeader*   header = nullptr;
		const ClusterHeader* clusters = nullptr;
		const uint8_t*       blob = nullptr;

		const Vertex*  vertices(uint32_t c) const { return reinterpret_cast<const Vertex*>(blob + clusters[c].vertex_offset); }
		const uint8_t* triangles(uint32_t c) const { return blob + clusters[c].triangle_offset; }
	};

	GroupView read_group(const Mesh& mesh, uint32_t group);

	// The on-disk form: header, the resident tables (groups, nodes, page directory), then the pages, each
	// 16-byte aligned at an offset the directory gives, so a streamer can read one page without the rest.
	// deserialize checks every size and offset and returns false on anything inconsistent.
	// base_file_offset is where `data` starts in the file it was read from: pages get their absolute
	// position from it.
	std::vector<uint8_t> serialize(const Mesh& mesh);
	bool deserialize(const uint8_t* data, size_t size, Mesh& out, uint64_t base_file_offset = 0);
}
