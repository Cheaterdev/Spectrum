#pragma once
struct DebugStruct_cb
{
	uint4 v;
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
