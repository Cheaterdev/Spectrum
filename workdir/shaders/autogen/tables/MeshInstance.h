#pragma once
struct MeshInstance_cb
{
	uint vertex_offset; // uint
	uint index_offset; // uint
};
struct MeshInstance
{
	MeshInstance_cb cb;
	uint GetVertex_offset() { return cb.vertex_offset; }
	uint GetIndex_offset() { return cb.index_offset; }

};
 const MeshInstance CreateMeshInstance(MeshInstance_cb cb)
{
	const MeshInstance result = {cb
	};
	return result;
}
