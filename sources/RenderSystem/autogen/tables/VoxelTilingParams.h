#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelTilingParams
	{
		struct CB
		{
			uint4 voxels_per_tile;
		} &cb;
		struct SRV
		{
			DX12::HLSL::StructuredBuffer<int3> tiles;
		} &srv;
		uint4& GetVoxels_per_tile() { return cb.voxels_per_tile; }
		DX12::HLSL::StructuredBuffer<int3>& GetTiles() { return srv.tiles; }
		VoxelTilingParams(CB&cb,SRV&srv) :cb(cb),srv(srv){}
	};
	#pragma pack(pop)
}
