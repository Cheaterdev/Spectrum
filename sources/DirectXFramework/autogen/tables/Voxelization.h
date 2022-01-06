#pragma once
#include "VoxelInfo.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct Voxelization
	{
		Render::HLSL::RWTexture3D<float4> albedo;
		Render::HLSL::RWTexture3D<float4> normals;
		Render::HLSL::RWTexture3D<uint> visibility;
		VoxelInfo info;
		Render::HLSL::RWTexture3D<float4>& GetAlbedo() { return albedo; }
		Render::HLSL::RWTexture3D<float4>& GetNormals() { return normals; }
		Render::HLSL::RWTexture3D<uint>& GetVisibility() { return visibility; }
		VoxelInfo& MapInfo() { return info; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(albedo);
			compiler.compile(normals);
			compiler.compile(visibility);
			compiler.compile(info);
		}
	};
	#pragma pack(pop)
}
