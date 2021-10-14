#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct MeshInfo
	{
		struct CB
		{
			uint node_offset;
			uint vertex_offset;
			uint meshlet_offset;
			uint meshlet_count;
			uint meshlet_unique_offset;
			uint meshlet_vertex_offset;
		} &cb;
		uint& GetNode_offset() { return cb.node_offset; }
		uint& GetVertex_offset() { return cb.vertex_offset; }
		uint& GetMeshlet_offset() { return cb.meshlet_offset; }
		uint& GetMeshlet_count() { return cb.meshlet_count; }
		uint& GetMeshlet_unique_offset() { return cb.meshlet_unique_offset; }
		uint& GetMeshlet_vertex_offset() { return cb.meshlet_vertex_offset; }
		MeshInfo(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
