#pragma once
struct IndirectArgs_cb
{
	uint data[5];
};
struct IndirectArgs
{
	IndirectArgs_cb cb;
	uint GetData(int i) { return cb.data[i]; }
};
 const IndirectArgs CreateIndirectArgs(IndirectArgs_cb cb)
{
	const IndirectArgs result = {cb
	};
	return result;
}
