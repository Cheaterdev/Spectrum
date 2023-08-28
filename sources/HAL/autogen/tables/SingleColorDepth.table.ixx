export module HAL:Autogen.Tables.SingleColorDepth;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct SingleColorDepth
	{
		static constexpr SlotID ID = SlotID::SingleColorDepth;
		HLSL::Texture2D<float4> color;
		HLSL::Texture2D<float> depth;
		HLSL::Texture2D<float4>& GetColor() { return color; }
		HLSL::Texture2D<float>& GetDepth() { return depth; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(color);
			compiler.compile(depth);
		}
		struct Compiled
		{
			uint color; // RenderTarget<float4>
			uint depth; // DepthStencil<float>
		};
	};
	#pragma pack(pop)
}
