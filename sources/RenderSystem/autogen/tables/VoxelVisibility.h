#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelVisibility
	{
		struct SRV
		{
			Render::HLSL::Texture3D<uint> visibility;
		} &srv;
		struct UAV
		{
			Render::HLSL::AppendStructuredBuffer<uint4> visible_tiles;
		} &uav;
		Render::HLSL::Texture3D<uint>& GetVisibility() { return srv.visibility; }
		Render::HLSL::AppendStructuredBuffer<uint4>& GetVisible_tiles() { return uav.visible_tiles; }
		VoxelVisibility(SRV&srv,UAV&uav) :srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}
