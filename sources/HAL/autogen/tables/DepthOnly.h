#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct DepthOnly
	{
		static constexpr SlotID ID = SlotID::DepthOnly;
		HLSL::Texture2D<float> depth;
		HLSL::Texture2D<float>& GetDepth() { return depth; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(depth);
		}
		struct Compiled
		{
			uint depth; // DepthStencil<float>
		};
	};
	#pragma pack(pop)
}
