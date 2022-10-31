#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct PickerBuffer
	{
		HLSL::RWStructuredBuffer<uint> viewBuffer;
		HLSL::RWStructuredBuffer<uint>& GetViewBuffer() { return viewBuffer; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(viewBuffer);
		}
	};
	#pragma pack(pop)
}
