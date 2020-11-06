#pragma once
struct DebugStruct_cb
{
	uint meshes_count;
};
struct DebugStruct
{
	DebugStruct_cb cb;
	uint GetMeshes_count() { return cb.meshes_count; }

};
 const DebugStruct CreateDebugStruct(DebugStruct_cb cb)
{
	const DebugStruct result = {cb
	};
	return result;
}
