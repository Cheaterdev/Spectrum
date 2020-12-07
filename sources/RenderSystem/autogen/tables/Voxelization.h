#pragma once
#include "VoxelInfo.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct Voxelization
	{
		struct CB
		{
			VoxelInfo::CB info;
		} &cb;
		struct UAV
		{
			Render::HLSL::RWTexture3D<float4> albedo;
			Render::HLSL::RWTexture3D<float4> normals;
			Render::HLSL::RWByteAddressBuffer visibility;
		} &uav;
		Render::HLSL::RWTexture3D<float4>& GetAlbedo() { return uav.albedo; }
		Render::HLSL::RWTexture3D<float4>& GetNormals() { return uav.normals; }
		Render::HLSL::RWByteAddressBuffer& GetVisibility() { return uav.visibility; }
		VoxelInfo MapInfo() { return VoxelInfo(cb.info); }
		Voxelization(CB&cb,UAV&uav) :cb(cb),uav(uav){}
	};
	#pragma pack(pop)
}
