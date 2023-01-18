#pragma once
struct GBuffer
{
	uint albedo; // RenderTarget<float4>
	uint normals; // RenderTarget<float4>
	uint specular; // RenderTarget<float4>
	uint motion; // RenderTarget<float2>
	uint depth; // DepthStencil<float>
	Texture2D<float4> GetAlbedo() { return ResourceDescriptorHeap[albedo]; }
	Texture2D<float4> GetNormals() { return ResourceDescriptorHeap[normals]; }
	Texture2D<float4> GetSpecular() { return ResourceDescriptorHeap[specular]; }
	Texture2D<float2> GetMotion() { return ResourceDescriptorHeap[motion]; }
	Texture2D<float> GetDepth() { return ResourceDescriptorHeap[depth]; }
};
