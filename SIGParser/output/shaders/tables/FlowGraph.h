#pragma once
struct FlowGraph_cb
{
	float4 size;
	float4 offset_size;
	float2 inv_pixel;
};
struct FlowGraph
{
	FlowGraph_cb cb;
	float4 GetSize() { return cb.size; }
	float4 GetOffset_size() { return cb.offset_size; }
	float2 GetInv_pixel() { return cb.inv_pixel; }
};
 const FlowGraph CreateFlowGraph(FlowGraph_cb cb)
{
	const FlowGraph result = {cb
	};
	return result;
}
