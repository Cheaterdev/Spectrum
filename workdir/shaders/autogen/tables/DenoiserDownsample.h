#pragma once
struct DenoiserDownsample_srv
{
	uint color; // Texture2D<float4>
	uint depth; // Texture2D<float>
};
struct DenoiserDownsample
{
	DenoiserDownsample_srv srv;
	Texture2D<float4> GetColor() { return ResourceDescriptorHeap[srv.color]; }
	Texture2D<float> GetDepth() { return ResourceDescriptorHeap[srv.depth]; }

};
 const DenoiserDownsample CreateDenoiserDownsample(DenoiserDownsample_srv srv)
{
	const DenoiserDownsample result = {srv
	};
	return result;
}
