#pragma once
struct GBufferDownsample_srv
{
	uint normals; // Texture2D<float4>
	uint depth; // Texture2D<float>
};
struct GBufferDownsample
{
	GBufferDownsample_srv srv;
	Texture2D<float4> GetNormals() { return ResourceDescriptorHeap[srv.normals]; }
	Texture2D<float> GetDepth() { return ResourceDescriptorHeap[srv.depth]; }

};
 const GBufferDownsample CreateGBufferDownsample(GBufferDownsample_srv srv)
{
	const GBufferDownsample result = {srv
	};
	return result;
}
