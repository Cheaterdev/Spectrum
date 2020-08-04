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
		using SRV = Empty;
		struct UAV
		{
			Render::Handle albedo;
			Render::Handle normals;
			Render::Handle visibility;
			VoxelInfo::UAV info;
		} &uav;
		using SMP = Empty;
		Render::Handle& GetAlbedo() { return uav.albedo; }
		Render::Handle& GetNormals() { return uav.normals; }
		Render::Handle& GetVisibility() { return uav.visibility; }
		VoxelInfo MapInfo() { return VoxelInfo(cb.info); }
		Voxelization(CB&cb,UAV&uav) :cb(cb),uav(uav){}
	};
	#pragma pack(pop)
}
