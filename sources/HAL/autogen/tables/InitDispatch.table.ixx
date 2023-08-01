export module HAL:Autogen.Tables.InitDispatch;
import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct InitDispatch
	{
		static constexpr SlotID ID = SlotID::InitDispatch;
		HLSL::RWStructuredBuffer<uint> counter;
		HLSL::RWStructuredBuffer<DispatchArguments> dispatch_data;
		HLSL::RWStructuredBuffer<uint>& GetCounter() { return counter; }
		HLSL::RWStructuredBuffer<DispatchArguments>& GetDispatch_data() { return dispatch_data; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(counter);
			compiler.compile(dispatch_data);
		}
		struct Compiled
		{
			uint counter; // RWStructuredBuffer<uint>
			uint dispatch_data; // RWStructuredBuffer<DispatchArguments>
		};
	};
	#pragma pack(pop)
}
