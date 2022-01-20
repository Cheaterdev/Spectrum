#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelOutput
	{
		HLSL::RWTexture2D<float4> noise;
		HLSL::RWTexture2D<float> frames;
		HLSL::RWTexture2D<float4> DirAndPdf;
		HLSL::RWTexture2D<float4>& GetNoise() { return noise; }
		HLSL::RWTexture2D<float>& GetFrames() { return frames; }
		HLSL::RWTexture2D<float4>& GetDirAndPdf() { return DirAndPdf; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(noise);
			compiler.compile(frames);
			compiler.compile(DirAndPdf);
		}
	};
	#pragma pack(pop)
}
