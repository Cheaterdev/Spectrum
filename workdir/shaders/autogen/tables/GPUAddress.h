#pragma once
struct GPUAddress_cb
{
	uint2 data; // uint2
};
struct GPUAddress
{
	GPUAddress_cb cb;
	uint2 GetData() { return cb.data; }

};
 const GPUAddress CreateGPUAddress(GPUAddress_cb cb)
{
	const GPUAddress result = {cb
	};
	return result;
}
