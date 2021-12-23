#pragma once
struct MeshInfo_cb
{
	uint node_offset; // uint
	uint vertex_offset; // uint
	uint meshlet_offset; // uint
	uint meshlet_count; // uint
	uint meshlet_unique_offset; // uint
	uint meshlet_vertex_offset; // uint
};
struct MeshInfo
{
	MeshInfo_cb cb;
	uint GetNode_offset() { return cb.node_offset; }
	uint GetVertex_offset() { return cb.vertex_offset; }
	uint GetMeshlet_offset() { return cb.meshlet_offset; }
	uint GetMeshlet_count() { return cb.meshlet_count; }
	uint GetMeshlet_unique_offset() { return cb.meshlet_unique_offset; }
	uint GetMeshlet_vertex_offset() { return cb.meshlet_vertex_offset; }

};
 const MeshInfo CreateMeshInfo(MeshInfo_cb cb)
{
	const MeshInfo result = {cb
	};
	return result;
}
