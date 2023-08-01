export module HAL:Autogen.Tables.DenoiserDownsample;
import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct DenoiserDownsample
	{
		static constexpr SlotID ID = SlotID::DenoiserDownsample;
		HLSL::Texture2D<float4> color;
		HLSL::Texture2D<float> depth;
		HLSL::Texture2D<float4>& GetColor() { return color; }
		HLSL::Texture2D<float>& GetDepth() { return depth; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(color);
			compiler.compile(depth);
		}
		struct Compiled
		{
			uint color; // Texture2D<float4>
			uint depth; // Texture2D<float>
		};
	};
	#pragma pack(pop)
}
