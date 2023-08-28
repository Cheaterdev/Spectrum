export module HAL:Autogen.Tables.DenoiserShadow_Filter;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct DenoiserShadow_Filter
	{
		static constexpr SlotID ID = SlotID::DenoiserShadow_Filter;
		float4x4 ProjectionInverse;
		int2 BufferDimensions;
		float2 InvBufferDimensions;
		float DepthSimilaritySigma;
		HLSL::Texture2D<float> t2d_DepthBuffer;
		HLSL::Texture2D<float16_t3> t2d_NormalBuffer;
		HLSL::StructuredBuffer<uint> sb_tileMetaData;
		float4x4& GetProjectionInverse() { return ProjectionInverse; }
		int2& GetBufferDimensions() { return BufferDimensions; }
		float2& GetInvBufferDimensions() { return InvBufferDimensions; }
		float& GetDepthSimilaritySigma() { return DepthSimilaritySigma; }
		HLSL::Texture2D<float>& GetT2d_DepthBuffer() { return t2d_DepthBuffer; }
		HLSL::Texture2D<float16_t3>& GetT2d_NormalBuffer() { return t2d_NormalBuffer; }
		HLSL::StructuredBuffer<uint>& GetSb_tileMetaData() { return sb_tileMetaData; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(ProjectionInverse);
			compiler.compile(BufferDimensions);
			compiler.compile(InvBufferDimensions);
			compiler.compile(DepthSimilaritySigma);
			compiler.compile(t2d_DepthBuffer);
			compiler.compile(t2d_NormalBuffer);
			compiler.compile(sb_tileMetaData);
		}
		struct Compiled
		{
			float4x4 ProjectionInverse; // float4x4
			int2 BufferDimensions; // int2
			float2 InvBufferDimensions; // float2
			float DepthSimilaritySigma; // float
			uint t2d_DepthBuffer; // Texture2D<float>
			uint t2d_NormalBuffer; // Texture2D<float16_t3>
			uint sb_tileMetaData; // StructuredBuffer<uint>
		};
	};
	#pragma pack(pop)
}
