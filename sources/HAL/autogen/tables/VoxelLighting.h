#pragma once
#include "PSSMDataGlobal.h"
#include "VoxelTilingParams.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelLighting
	{
		HLSL::Texture3D<float4> albedo;
		HLSL::Texture3D<float4> normals;
		HLSL::Texture3D<float4> lower;
		HLSL::TextureCube<float4> tex_cube;
		HLSL::RWTexture3D<float4> output;
		PSSMDataGlobal pssmGlobal;
		VoxelTilingParams params;
		HLSL::RWTexture3D<float4>& GetOutput() { return output; }
		HLSL::Texture3D<float4>& GetAlbedo() { return albedo; }
		HLSL::Texture3D<float4>& GetNormals() { return normals; }
		HLSL::Texture3D<float4>& GetLower() { return lower; }
		HLSL::TextureCube<float4>& GetTex_cube() { return tex_cube; }
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
		struct Compiled
		{
			uint albedo; // Texture3D<float4>
			uint normals; // Texture3D<float4>
			uint lower; // Texture3D<float4>
			uint tex_cube; // TextureCube<float4>
			uint output; // RWTexture3D<float4>
			PSSMDataGlobal::Compiled pssmGlobal; // PSSMDataGlobal
			VoxelTilingParams::Compiled params; // VoxelTilingParams
		};
	};
	#pragma pack(pop)
}
