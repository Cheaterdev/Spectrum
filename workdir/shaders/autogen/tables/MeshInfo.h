#pragma once
struct MeshInfo
{
	uint vertex_offset_local; // uint
	uint meshlet_offset_local; // uint
	uint node_offset; // uint
	uint meshlet_count; // uint
	uint GetVertex_offset_local() { return vertex_offset_local; }
	uint GetMeshlet_offset_local() { return meshlet_offset_local; }
	uint GetNode_offset() { return node_offset; }
	uint GetMeshlet_count() { return meshlet_count; }
};
