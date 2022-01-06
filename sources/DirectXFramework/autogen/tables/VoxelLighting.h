#pragma once
#include "PSSMDataGlobal.h"
#include "VoxelTilingParams.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelLighting
	{
		Render::HLSL::Texture3D<float4> albedo;
		Render::HLSL::Texture3D<float4> normals;
		Render::HLSL::Texture3D<float4> lower;
		Render::HLSL::TextureCube<float4> tex_cube;
		Render::HLSL::RWTexture3D<float4> output;
		PSSMDataGlobal pssmGlobal;
		VoxelTilingParams params;
		Render::HLSL::RWTexture3D<float4>& GetOutput() { return output; }
		Render::HLSL::Texture3D<float4>& GetAlbedo() { return albedo; }
		Render::HLSL::Texture3D<float4>& GetNormals() { return normals; }
		Render::HLSL::Texture3D<float4>& GetLower() { return lower; }
		Render::HLSL::TextureCube<float4>& GetTex_cube() { return tex_cube; }
		PSSMDataGlobal& GetPssmGlobal() { return pssmGlobal; }
		VoxelTilingParams& GetParams() { return params; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(albedo);
			compiler.compile(normals);
			compiler.compile(lower);
			compiler.compile(tex_cube);
			compiler.compile(output);
			compiler.compile(pssmGlobal);
			compiler.compile(params);
		}
	};
	#pragma pack(pop)
}
