export module HAL:Autogen.Tables.SingleColor;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct SingleColor
	{
		static constexpr SlotID ID = SlotID::SingleColor;
		HLSL::Texture2D<float4> color;
		HLSL::Texture2D<float4>& GetColor() { return color; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(color);
		}
		struct Compiled
		{
			uint color; // RenderTarget<float4>
		};
	};
	#pragma pack(pop)
}
