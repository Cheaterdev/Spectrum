#pragma once
struct MaterialInfo
{
	MaterialCB data; // MaterialCB
	uint textures; // Texture2D<float4>
	MaterialCB GetData() { return data; }
	Texture2D<float4> GetTextures(int i) { 
	StructuredBuffer<uint> indirection = ResourceDescriptorHeap[textures]; 
	uint id = indirection.Load(i);
	return ResourceDescriptorHeap[id]; }
};
