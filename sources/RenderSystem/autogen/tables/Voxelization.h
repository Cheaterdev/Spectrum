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
			DX12::HLSL::RWTexture3D<float4> albedo;
			DX12::HLSL::RWTexture3D<float4> normals;
			DX12::HLSL::RWTexture3D<uint> visibility;
		} &uav;
		DX12::HLSL::RWTexture3D<float4>& GetAlbedo() { return uav.albedo; }
		DX12::HLSL::RWTexture3D<float4>& GetNormals() { return uav.normals; }
		DX12::HLSL::RWTexture3D<uint>& GetVisibility() { return uav.visibility; }
		VoxelInfo MapInfo() { return VoxelInfo(cb.info); }
		Voxelization(CB&cb,UAV&uav) :cb(cb),uav(uav){}
	};
	#pragma pack(pop)
}
