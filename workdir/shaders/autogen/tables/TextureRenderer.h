#pragma once
struct TextureRenderer
{
	uint texture; // Texture2D<float4>
	Texture2D<float4> GetTexture() { return ResourceDescriptorHeap[texture]; }
};
