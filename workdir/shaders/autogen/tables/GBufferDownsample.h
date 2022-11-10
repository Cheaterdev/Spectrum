#pragma once
struct GBufferDownsample
{
	uint normals; // Texture2D<float4>
	uint depth; // Texture2D<float>
	Texture2D<float4> GetNormals() { return ResourceDescriptorHeap[normals]; }
	Texture2D<float> GetDepth() { return ResourceDescriptorHeap[depth]; }
};
