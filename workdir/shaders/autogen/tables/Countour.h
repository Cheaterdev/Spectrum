#pragma once
struct Countour
{
	float4 color; // float4
	uint tex; // Texture2D<float4>
	float4 GetColor() { return color; }
	Texture2D<float4> GetTex() { return ResourceDescriptorHeap[tex]; }
};
