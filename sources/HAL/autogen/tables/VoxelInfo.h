#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelInfo
	{
		float4 min;
		float4 size;
		uint4 voxels_per_tile;
		uint4 voxel_tiles_count;
		float4& GetMin() { return min; }
		float4& GetSize() { return size; }
		uint4& GetVoxels_per_tile() { return voxels_per_tile; }
		uint4& GetVoxel_tiles_count() { return voxel_tiles_count; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(min);
			compiler.compile(size);
			compiler.compile(voxels_per_tile);
			compiler.compile(voxel_tiles_count);
		}
	};
	#pragma pack(pop)
}