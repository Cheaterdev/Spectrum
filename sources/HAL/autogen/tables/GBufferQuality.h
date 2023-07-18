#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct GBufferQuality
	{
		static constexpr SlotID ID = SlotID::GBufferQuality;
		HLSL::Texture2D<float4> ref;
		HLSL::Texture2D<float4>& GetRef() { return ref; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(ref);
		}
		struct Compiled
		{
			uint ref; // Texture2D<float4>
		};
	};
	#pragma pack(pop)
}
