#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct InitDispatch
	{
		Render::HLSL::RWStructuredBuffer<uint> counter;
		Render::HLSL::RWStructuredBuffer<DispatchArguments> dispatch_data;
		Render::HLSL::RWStructuredBuffer<uint>& GetCounter() { return counter; }
		Render::HLSL::RWStructuredBuffer<DispatchArguments>& GetDispatch_data() { return dispatch_data; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(counter);
			compiler.compile(dispatch_data);
		}
	};
	#pragma pack(pop)
}
