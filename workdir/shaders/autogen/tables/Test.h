#pragma once
#include "MeshInstanceInfo.h"
struct Test
{
	float data[16]; // float
	uint instances; // StructuredBuffer<MeshInstanceInfo>
	uint tex; // Texture2D<float4>
	float GetData(int i) { return data[i]; }
	StructuredBuffer<MeshInstanceInfo> GetInstances(int i) { 
	StructuredBuffer<uint> indirection = ResourceDescriptorHeap[instances]; 
	uint id = indirection.Load(i);
	return ResourceDescriptorHeap[id]; }
	Texture2D<float4> GetTex(int i) { 
	StructuredBuffer<uint> indirection = ResourceDescriptorHeap[tex]; 
	uint id = indirection.Load(i);
	return ResourceDescriptorHeap[id]; }
};
