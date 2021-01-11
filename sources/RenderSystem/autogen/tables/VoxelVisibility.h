#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelVisibility
	{
		struct SRV
		{
			DX12::HLSL::Texture3D<uint> visibility;
		} &srv;
		struct UAV
		{
			DX12::HLSL::AppendStructuredBuffer<uint4> visible_tiles;
		} &uav;
		DX12::HLSL::Texture3D<uint>& GetVisibility() { return srv.visibility; }
		DX12::HLSL::AppendStructuredBuffer<uint4>& GetVisible_tiles() { return uav.visible_tiles; }
		VoxelVisibility(SRV&srv,UAV&uav) :srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}
