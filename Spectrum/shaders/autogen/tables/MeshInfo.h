#pragma once
struct MeshInfo_cb
{
	uint texture_offset;
	uint node_offset;
	uint vertex_offset;
};
struct MeshInfo
{
	MeshInfo_cb cb;
	uint GetTexture_offset() { return cb.texture_offset; }
	uint GetNode_offset() { return cb.node_offset; }
	uint GetVertex_offset() { return cb.vertex_offset; }
};
 const MeshInfo CreateMeshInfo(MeshInfo_cb cb)
{
	const MeshInfo result = {cb
	};
	return result;
}
