#pragma once
struct DrawIndexedArguments
{
	uint data[5]; // uint
	uint GetData(int i) { return data[i]; }
};
