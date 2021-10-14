#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct FlowGraph
	{
		struct CB
		{
			float4 size;
			float4 offset_size;
			float2 inv_pixel;
		} &cb;
		float4& GetSize() { return cb.size; }
		float4& GetOffset_size() { return cb.offset_size; }
		float2& GetInv_pixel() { return cb.inv_pixel; }
		FlowGraph(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
