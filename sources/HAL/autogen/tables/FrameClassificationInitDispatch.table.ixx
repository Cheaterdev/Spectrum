export module HAL:Autogen.Tables.FrameClassificationInitDispatch;
import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct FrameClassificationInitDispatch
	{
		static constexpr SlotID ID = SlotID::FrameClassificationInitDispatch;
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
		struct Compiled
		{
			uint hi_counter; // StructuredBuffer<uint>
			uint low_counter; // StructuredBuffer<uint>
			uint hi_dispatch_data; // RWStructuredBuffer<DispatchArguments>
			uint low_dispatch_data; // RWStructuredBuffer<DispatchArguments>
		};
	};
	#pragma pack(pop)
}
