#pragma once
struct DispatchMeshArguments_cb
{
	uint3 counts;
};
struct DispatchMeshArguments
{
	DispatchMeshArguments_cb cb;
	uint3 GetCounts() { return cb.counts; }

};
 const DispatchMeshArguments CreateDispatchMeshArguments(DispatchMeshArguments_cb cb)
{
	const DispatchMeshArguments result = {cb
	};
	return result;
}
