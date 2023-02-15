#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelOutput
	{
		HLSL::Texture2D<float2> blueNoise;
		HLSL::RWTexture2D<float4> noise;
		HLSL::RWTexture2D<float> frames;
		HLSL::RWTexture2D<float4> DirAndPdf;
		HLSL::RWTexture2D<float4>& GetNoise() { return noise; }
		HLSL::RWTexture2D<float>& GetFrames() { return frames; }
		HLSL::RWTexture2D<float4>& GetDirAndPdf() { return DirAndPdf; }
		HLSL::Texture2D<float2>& GetBlueNoise() { return blueNoise; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(blueNoise);
			compiler.compile(noise);
			compiler.compile(frames);
			compiler.compile(DirAndPdf);
		}
		struct Compiled
		{
			uint blueNoise; // Texture2D<float2>
			uint noise; // RWTexture2D<float4>
			uint frames; // RWTexture2D<float>
			uint DirAndPdf; // RWTexture2D<float4>
		};
	};
	#pragma pack(pop)
}
