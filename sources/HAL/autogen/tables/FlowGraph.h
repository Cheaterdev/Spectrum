#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct FlowGraph
	{
		float4 size;
		float4 offset_size;
		float2 inv_pixel;
		float4& GetSize() { return size; }
		float4& GetOffset_size() { return offset_size; }
		float2& GetInv_pixel() { return inv_pixel; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(size);
			compiler.compile(offset_size);
			compiler.compile(inv_pixel);
		}
		using Compiled = FlowGraph;
		};
		#pragma pack(pop)
	}
