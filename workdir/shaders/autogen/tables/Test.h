#pragma once
struct Test
{
	float data[16]; // float
	uint tex; // Texture2D<float4>
	float GetData(int i) { return data[i]; }
	Texture2D<float4> GetTex(int i) { return ResourceDescriptorHeap[tex + i]; }
};
