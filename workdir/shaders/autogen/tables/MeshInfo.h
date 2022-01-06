#pragma once
struct MeshInfo
{
	uint node_offset; // uint
	uint vertex_offset; // uint
	uint meshlet_offset; // uint
	uint meshlet_count; // uint
	uint meshlet_unique_offset; // uint
	uint meshlet_vertex_offset; // uint
	uint GetNode_offset() { return node_offset; }
	uint GetVertex_offset() { return vertex_offset; }
	uint GetMeshlet_offset() { return meshlet_offset; }
	uint GetMeshlet_count() { return meshlet_count; }
	uint GetMeshlet_unique_offset() { return meshlet_unique_offset; }
	uint GetMeshlet_vertex_offset() { return meshlet_vertex_offset; }
};
