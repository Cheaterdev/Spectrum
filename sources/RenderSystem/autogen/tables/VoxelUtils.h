#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelUtils
	{
		struct CB
		{
			uint groupCount;
			uint4 voxels_per_tile;
		} &cb;
		struct SRV
		{
			DX12::HLSL::Texture3D<float4> Source[2];
			DX12::HLSL::StructuredBuffer<int3> visibility;
		} &srv;
		struct UAV
		{
			DX12::HLSL::RWTexture3D<float4> Target[2];
		} &uav;
		DX12::HLSL::RWTexture3D<float4>* GetTarget() { return uav.Target; }
		DX12::HLSL::Texture3D<float4>* GetSource() { return srv.Source; }
		DX12::HLSL::StructuredBuffer<int3>& GetVisibility() { return srv.visibility; }
		uint& GetGroupCount() { return cb.groupCount; }
		uint4& GetVoxels_per_tile() { return cb.voxels_per_tile; }
		VoxelUtils(CB&cb,SRV&srv,UAV&uav) :cb(cb),srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}
