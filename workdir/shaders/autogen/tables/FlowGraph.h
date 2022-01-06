#pragma once
struct FlowGraph
{
	float4 size; // float4
	float4 offset_size; // float4
	float2 inv_pixel; // float2
	float4 GetSize() { return size; }
	float4 GetOffset_size() { return offset_size; }
	float2 GetInv_pixel() { return inv_pixel; }
};
