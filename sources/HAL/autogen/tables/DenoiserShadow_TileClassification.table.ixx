export module HAL:Autogen.Tables.DenoiserShadow_TileClassification;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct DenoiserShadow_TileClassification
	{
		static constexpr SlotID ID = SlotID::DenoiserShadow_TileClassification;
		float3 Eye;
		int FirstFrame;
		int2 BufferDimensions;
		float2 InvBufferDimensions;
		float4x4 ProjectionInverse;
		float4x4 ReprojectionMatrix;
		float4x4 ViewProjectionInverse;
		float DepthSimilaritySigma;
		HLSL::Texture2D<float> t2d_depth;
		HLSL::Texture2D<float2> t2d_velocity;
		HLSL::Texture2D<float3> t2d_normal;
		HLSL::Texture2D<float2> t2d_history;
		HLSL::Texture2D<float> t2d_previousDepth;
		HLSL::StructuredBuffer<uint> sb_raytracerResult;
		HLSL::Texture2D<float3> t2d_previousMoments;
		HLSL::RWStructuredBuffer<uint> rwsb_tileMetaData;
		HLSL::RWTexture2D<float2> rwt2d_reprojectionResults;
		HLSL::RWTexture2D<float3> rwt2d_momentsBuffer;
		float3& GetEye() { return Eye; }
		int& GetFirstFrame() { return FirstFrame; }
		int2& GetBufferDimensions() { return BufferDimensions; }
		float2& GetInvBufferDimensions() { return InvBufferDimensions; }
		float4x4& GetProjectionInverse() { return ProjectionInverse; }
		float4x4& GetReprojectionMatrix() { return ReprojectionMatrix; }
		float4x4& GetViewProjectionInverse() { return ViewProjectionInverse; }
		float& GetDepthSimilaritySigma() { return DepthSimilaritySigma; }
		HLSL::Texture2D<float>& GetT2d_depth() { return t2d_depth; }
		HLSL::Texture2D<float2>& GetT2d_velocity() { return t2d_velocity; }
		HLSL::Texture2D<float3>& GetT2d_normal() { return t2d_normal; }
		HLSL::Texture2D<float2>& GetT2d_history() { return t2d_history; }
		HLSL::Texture2D<float>& GetT2d_previousDepth() { return t2d_previousDepth; }
		HLSL::StructuredBuffer<uint>& GetSb_raytracerResult() { return sb_raytracerResult; }
		HLSL::RWStructuredBuffer<uint>& GetRwsb_tileMetaData() { return rwsb_tileMetaData; }
		HLSL::RWTexture2D<float2>& GetRwt2d_reprojectionResults() { return rwt2d_reprojectionResults; }
		HLSL::Texture2D<float3>& GetT2d_previousMoments() { return t2d_previousMoments; }
		HLSL::RWTexture2D<float3>& GetRwt2d_momentsBuffer() { return rwt2d_momentsBuffer; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(Eye);
			compiler.compile(FirstFrame);
			compiler.compile(BufferDimensions);
			compiler.compile(InvBufferDimensions);
			compiler.compile(ProjectionInverse);
			compiler.compile(ReprojectionMatrix);
			compiler.compile(ViewProjectionInverse);
			compiler.compile(DepthSimilaritySigma);
			compiler.compile(t2d_depth);
			compiler.compile(t2d_velocity);
			compiler.compile(t2d_normal);
			compiler.compile(t2d_history);
			compiler.compile(t2d_previousDepth);
			compiler.compile(sb_raytracerResult);
			compiler.compile(t2d_previousMoments);
			compiler.compile(rwsb_tileMetaData);
			compiler.compile(rwt2d_reprojectionResults);
			compiler.compile(rwt2d_momentsBuffer);
		}
		struct Compiled
		{
			float3 Eye; // float3
			int FirstFrame; // int
			int2 BufferDimensions; // int2
			float2 InvBufferDimensions; // float2
			float4x4 ProjectionInverse; // float4x4
			float4x4 ReprojectionMatrix; // float4x4
			float4x4 ViewProjectionInverse; // float4x4
			float DepthSimilaritySigma; // float
			uint t2d_depth; // Texture2D<float>
			uint t2d_velocity; // Texture2D<float2>
			uint t2d_normal; // Texture2D<float3>
			uint t2d_history; // Texture2D<float2>
			uint t2d_previousDepth; // Texture2D<float>
			uint sb_raytracerResult; // StructuredBuffer<uint>
			uint t2d_previousMoments; // Texture2D<float3>
			uint rwsb_tileMetaData; // RWStructuredBuffer<uint>
			uint rwt2d_reprojectionResults; // RWTexture2D<float2>
			uint rwt2d_momentsBuffer; // RWTexture2D<float3>
		};
	};
	#pragma pack(pop)
}
