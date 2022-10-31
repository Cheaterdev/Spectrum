#pragma once
#include "GBuffer.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelScreen
	{
		HLSL::Texture3D<float4> voxels;
		HLSL::TextureCube<float4> tex_cube;
		HLSL::Texture2D<float4> prev_gi;
		HLSL::Texture2D<float> prev_frames;
		HLSL::Texture2D<float> prev_depth;
		GBuffer gbuffer;
		HLSL::Texture3D<float4>& GetVoxels() { return voxels; }
		HLSL::TextureCube<float4>& GetTex_cube() { return tex_cube; }
		HLSL::Texture2D<float4>& GetPrev_gi() { return prev_gi; }
		HLSL::Texture2D<float>& GetPrev_frames() { return prev_frames; }
		HLSL::Texture2D<float>& GetPrev_depth() { return prev_depth; }
		GBuffer& GetGbuffer() { return gbuffer; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(voxels);
			compiler.compile(tex_cube);
			compiler.compile(prev_gi);
			compiler.compile(prev_frames);
			compiler.compile(prev_depth);
			compiler.compile(gbuffer);
		}
	};
	#pragma pack(pop)
}
