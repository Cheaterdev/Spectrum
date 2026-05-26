export module HAL:Autogen.Tables.GBufferDownsampleRT;

import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;
import <HAL.h>;
export namespace Table
{
	#pragma pack(push, 1)

	struct GBufferDownsampleRT
	{
		static constexpr SlotID ID = SlotID::GBufferDownsampleRT;
		HLSL::Texture2D<float> depth;
		HLSL::Texture2D<float4> color;
		HLSL::Texture2D<float>& GetDepth() { return depth; }
		HLSL::Texture2D<float4>& GetColor() { return color; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(depth);
			compiler.compile(color);
		}
		struct Compiled
		{
			uint depth; // RenderTarget<float>
			uint color; // RenderTarget<float4>

			
			private:
			SERIALIZE()
			{
			}


		};

		static std::string get_typename()
		{
			return "Tables::GBufferDownsampleRT";
		}
		private:
		SERIALIZE()
		{
		}

	};
	#pragma pack(pop)
}

