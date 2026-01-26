export module HAL:Autogen.Tables.FrameGraph_Debug_Texture2D;

import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;
import <HAL.h>;
export namespace Table
{
	#pragma pack(push, 1)

	struct FrameGraph_Debug_Texture2D
	{
		static constexpr SlotID ID = SlotID::FrameGraph_Debug_Texture2D;
		float2 scale;
		float2 offset;
		HLSL::Texture2D<float4> source;
		HLSL::Texture2D<float4>& GetSource() { return source; }
		float2& GetScale() { return scale; }
		float2& GetOffset() { return offset; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(scale);
			compiler.compile(offset);
			compiler.compile(source);
		}
		struct Compiled
		{
			float2 scale; // float2
			float2 offset; // float2
			uint source; // Texture2D<float4>
		};

	};
	#pragma pack(pop)
}

