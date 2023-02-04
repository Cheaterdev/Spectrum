#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct DenoiserShadow_Fileter
	{
		float4x4 ProjectionInverse;
		int2 BufferDimensions;
		float2 InvBufferDimensions;
		float DepthSimilaritySigma;
		HLSL::Texture2D<float> t2d_DepthBuffer;
		HLSL::Texture2D<float16_t3> t2d_NormalBuffer;
		HLSL::StructuredBuffer<uint> sb_tileMetaData;
		HLSL::Texture2D<float16_t2> rqt2d_input;
		HLSL::RWTexture2D<float2> rwt2d_history;
		HLSL::RWTexture2D<unormfloat4> rwt2d_output;
		float4x4& GetProjectionInverse() { return ProjectionInverse; }
		int2& GetBufferDimensions() { return BufferDimensions; }
		float2& GetInvBufferDimensions() { return InvBufferDimensions; }
		float& GetDepthSimilaritySigma() { return DepthSimilaritySigma; }
		HLSL::Texture2D<float>& GetT2d_DepthBuffer() { return t2d_DepthBuffer; }
		HLSL::Texture2D<float16_t3>& GetT2d_NormalBuffer() { return t2d_NormalBuffer; }
		HLSL::StructuredBuffer<uint>& GetSb_tileMetaData() { return sb_tileMetaData; }
		HLSL::Texture2D<float16_t2>& GetRqt2d_input() { return rqt2d_input; }
		HLSL::RWTexture2D<float2>& GetRwt2d_history() { return rwt2d_history; }
		HLSL::RWTexture2D<unormfloat4>& GetRwt2d_output() { return rwt2d_output; }
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
			compiler.compile(rqt2d_input);
			compiler.compile(rwt2d_history);
			compiler.compile(rwt2d_output);
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
			uint rqt2d_input; // Texture2D<float16_t2>
			uint rwt2d_history; // RWTexture2D<float2>
			uint rwt2d_output; // RWTexture2D<unormfloat4>
		};
	};
	#pragma pack(pop)
}
