export module HAL:Autogen.Tables.FontRendering;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct FontRendering
	{
		static constexpr SlotID ID = SlotID::FontRendering;
		HLSL::Texture2D<float> tex0;
		HLSL::Buffer<float4> positions;
		HLSL::Texture2D<float>& GetTex0() { return tex0; }
		HLSL::Buffer<float4>& GetPositions() { return positions; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(tex0);
			compiler.compile(positions);
		}
		struct Compiled
		{
			uint tex0; // Texture2D<float>
			uint positions; // Buffer<float4>
		};
	};
	#pragma pack(pop)
}
