export module HAL:Autogen.Tables.FontRenderingConstants;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct FontRenderingConstants
	{
		static constexpr SlotID ID = SlotID::FontRenderingConstants;
		float4x4 TransformMatrix;
		float4 ClipRect;
		float4x4& GetTransformMatrix() { return TransformMatrix; }
		float4& GetClipRect() { return ClipRect; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(TransformMatrix);
			compiler.compile(ClipRect);
		}
		using Compiled = FontRenderingConstants;
		};
		#pragma pack(pop)
	}
