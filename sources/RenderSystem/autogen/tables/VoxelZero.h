#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelZero
	{
		struct CB
		{
			uint4 voxels_per_tile;
			uint groupCount;
		} &cb;
		struct SRV
		{
			Render::HLSL::StructuredBuffer<int3> visibility;
		} &srv;
		struct UAV
		{
			Render::HLSL::RWTexture3D<float4> Target;
		} &uav;
		uint4& GetVoxels_per_tile() { return cb.voxels_per_tile; }
		Render::HLSL::RWTexture3D<float4>& GetTarget() { return uav.Target; }
		Render::HLSL::StructuredBuffer<int3>& GetVisibility() { return srv.visibility; }
		uint& GetGroupCount() { return cb.groupCount; }
		VoxelZero(CB&cb,SRV&srv,UAV&uav) :cb(cb),srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}
