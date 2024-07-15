#pragma once
#include "sig_hlsl.hlsl"
struct BoxInfo
{
	uint node_offset; // uint
	uint mesh_id; // uint
	uint GetNode_offset() { return node_offset; }
	uint GetMesh_id() { return mesh_id; }
};