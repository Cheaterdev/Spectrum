export module HAL:Autogen.Tables.GBufferDownsample;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct GBufferDownsample
	{
		static constexpr SlotID ID = SlotID::GBufferDownsample;
		HLSL::Texture2D<float4> normals;
		HLSL::Texture2D<float> depth;
		HLSL::Texture2D<float4>& GetNormals() { return normals; }
		HLSL::Texture2D<float>& GetDepth() { return depth; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(normals);
			compiler.compile(depth);
		}
		struct Compiled
		{
			uint normals; // Texture2D<float4>
			uint depth; // Texture2D<float>
		};
	};
	#pragma pack(pop)
}
