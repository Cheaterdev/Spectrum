#pragma once
struct BoxInfo_cb
{
	uint node_offset;
	uint mesh_id;
};
struct BoxInfo
{
	BoxInfo_cb cb;
	uint GetNode_offset() { return cb.node_offset; }
	uint GetMesh_id() { return cb.mesh_id; }

};
 const BoxInfo CreateBoxInfo(BoxInfo_cb cb)
{
	const BoxInfo result = {cb
	};
	return result;
}
