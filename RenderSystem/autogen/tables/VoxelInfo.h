#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelInfo
	{
		struct CB
		{
			float4 min;
			float4 size;
			uint4 voxels_per_tile;
			uint4 voxel_tiles_count;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		float4& GetMin() { return cb.min; }
		float4& GetSize() { return cb.size; }
		uint4& GetVoxels_per_tile() { return cb.voxels_per_tile; }
		uint4& GetVoxel_tiles_count() { return cb.voxel_tiles_count; }
		VoxelInfo(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
