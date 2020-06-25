#pragma once
struct DrawIndexedArguments_cb
{
	uint4 data;
	uint G;
	
};
struct DrawIndexedArguments
{
	DrawIndexedArguments_cb cb;
	uint GetData(int i) { return cb.data[i]; }
};
 const DrawIndexedArguments CreateDrawIndexedArguments(DrawIndexedArguments_cb cb)
{
	const DrawIndexedArguments result = {cb
	};
	return result;
}
