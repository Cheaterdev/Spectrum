#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelOutput
	{
		Render::HLSL::RWTexture2D<float4> noise;
		Render::HLSL::RWTexture2D<float> frames;
		Render::HLSL::RWTexture2D<float4> DirAndPdf;
		Render::HLSL::RWTexture2D<float4>& GetNoise() { return noise; }
		Render::HLSL::RWTexture2D<float>& GetFrames() { return frames; }
		Render::HLSL::RWTexture2D<float4>& GetDirAndPdf() { return DirAndPdf; }
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
