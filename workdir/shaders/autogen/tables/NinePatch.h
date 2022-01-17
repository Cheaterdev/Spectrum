#pragma once
#include "vertex_input.h"
struct NinePatch
{
	uint vb; // StructuredBuffer<vertex_input>
	uint textures; // Texture2D<float4>
	StructuredBuffer<vertex_input> GetVb() { return ResourceDescriptorHeap[vb]; }
	Texture2D<float4> GetTextures(int i) { 
	StructuredBuffer<uint> indirection = ResourceDescriptorHeap[textures]; 
	uint id = indirection.Load(i);
	return ResourceDescriptorHeap[id]; }
};
