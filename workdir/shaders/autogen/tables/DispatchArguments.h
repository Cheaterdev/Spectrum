#pragma once
struct DispatchArguments_cb
{
	uint3 counts;
};
struct DispatchArguments
{
	DispatchArguments_cb cb;
	uint3 GetCounts() { return cb.counts; }

};
 const DispatchArguments CreateDispatchArguments(DispatchArguments_cb cb)
{
	const DispatchArguments result = {cb
	};
	return result;
}
