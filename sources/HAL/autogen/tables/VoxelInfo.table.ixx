export module HAL:Autogen.Tables.VoxelInfo;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelInfo
	{
		static constexpr SlotID ID = SlotID::VoxelInfo;
		float4 min;
		float4 size;
		uint4 voxels_per_tile;
		uint4 voxel_tiles_count;
		float4& GetMin() { return min; }
		float4& GetSize() { return size; }
		uint4& GetVoxels_per_tile() { return voxels_per_tile; }
		uint4& GetVoxel_tiles_count() { return voxel_tiles_count; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(min);
			compiler.compile(size);
			compiler.compile(voxels_per_tile);
			compiler.compile(voxel_tiles_count);
		}
		using Compiled = VoxelInfo;
		};
		#pragma pack(pop)
	}
