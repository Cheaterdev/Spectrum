#pragma once
struct GBuffer
{
	uint albedo; // Texture2D<float4>
	uint normals; // Texture2D<float4>
	uint specular; // Texture2D<float4>
	uint depth; // Texture2D<float>
	uint motion; // Texture2D<float2>
	Texture2D<float4> GetAlbedo() { return ResourceDescriptorHeap[albedo]; }
	Texture2D<float4> GetNormals() { return ResourceDescriptorHeap[normals]; }
	Texture2D<float4> GetSpecular() { return ResourceDescriptorHeap[specular]; }
	Texture2D<float> GetDepth() { return ResourceDescriptorHeap[depth]; }
	Texture2D<float2> GetMotion() { return ResourceDescriptorHeap[motion]; }
};
