#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelTilingParams
	{
		uint4 voxels_per_tile;
		HLSL::StructuredBuffer<int3> tiles;
		uint4& GetVoxels_per_tile() { return voxels_per_tile; }
		HLSL::StructuredBuffer<int3>& GetTiles() { return tiles; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(voxels_per_tile);
			compiler.compile(tiles);
		}
	};
	#pragma pack(pop)
}
