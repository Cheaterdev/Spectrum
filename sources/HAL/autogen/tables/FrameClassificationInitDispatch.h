#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct FrameClassificationInitDispatch
	{
		HLSL::StructuredBuffer<uint> hi_counter;
		HLSL::StructuredBuffer<uint> low_counter;
		HLSL::RWStructuredBuffer<DispatchArguments> hi_dispatch_data;
		HLSL::RWStructuredBuffer<DispatchArguments> low_dispatch_data;
		HLSL::StructuredBuffer<uint>& GetHi_counter() { return hi_counter; }
		HLSL::StructuredBuffer<uint>& GetLow_counter() { return low_counter; }
		HLSL::RWStructuredBuffer<DispatchArguments>& GetHi_dispatch_data() { return hi_dispatch_data; }
		HLSL::RWStructuredBuffer<DispatchArguments>& GetLow_dispatch_data() { return low_dispatch_data; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(hi_counter);
			compiler.compile(low_counter);
			compiler.compile(hi_dispatch_data);
			compiler.compile(low_dispatch_data);
		}
	};
	#pragma pack(pop)
}
