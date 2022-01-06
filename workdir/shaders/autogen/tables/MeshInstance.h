#pragma once
struct MeshInstance
{
	uint vertex_offset; // uint
	uint index_offset; // uint
	uint GetVertex_offset() { return vertex_offset; }
	uint GetIndex_offset() { return index_offset; }
};
