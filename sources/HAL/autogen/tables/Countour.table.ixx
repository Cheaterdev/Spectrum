export module HAL:Autogen.Tables.Countour;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct Countour
	{
		static constexpr SlotID ID = SlotID::Countour;
		float4 color;
		HLSL::Texture2D<float4> tex;
		float4& GetColor() { return color; }
		HLSL::Texture2D<float4>& GetTex() { return tex; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(color);
			compiler.compile(tex);
		}
		struct Compiled
		{
			float4 color; // float4
			uint tex; // Texture2D<float4>
		};
	};
	#pragma pack(pop)
}
