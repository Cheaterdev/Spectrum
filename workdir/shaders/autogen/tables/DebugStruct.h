#pragma once
struct DebugStruct_cb
{
	uint4 v; // uint4
};
struct DebugStruct
{
	DebugStruct_cb cb;
	uint4 GetV() { return cb.v; }

};
 const DebugStruct CreateDebugStruct(DebugStruct_cb cb)
{
	const DebugStruct result = {cb
	};
	return result;
}
