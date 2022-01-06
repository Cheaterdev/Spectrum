#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct PickerBuffer
	{
		Render::HLSL::RWStructuredBuffer<uint> viewBuffer;
		Render::HLSL::RWStructuredBuffer<uint>& GetViewBuffer() { return viewBuffer; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(viewBuffer);
		}
	};
	#pragma pack(pop)
}
