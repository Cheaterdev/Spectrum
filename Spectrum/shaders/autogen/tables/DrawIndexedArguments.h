#pragma once
struct DrawIndexedArguments_cb
{
	uint data[5];
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
