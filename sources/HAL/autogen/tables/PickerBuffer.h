#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct PickerBuffer
	{
		static constexpr SlotID ID = SlotID::PickerBuffer;
		HLSL::RWStructuredBuffer<uint> viewBuffer;
		HLSL::RWStructuredBuffer<uint>& GetViewBuffer() { return viewBuffer; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(viewBuffer);
		}
		struct Compiled
		{
			uint viewBuffer; // RWStructuredBuffer<uint>
		};
	};
	#pragma pack(pop)
}
