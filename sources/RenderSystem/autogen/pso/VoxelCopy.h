#pragma once
namespace PSOS
{
	struct VoxelCopy: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_COMPUTE_PSO(VoxelCopy)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("VoxelCopy");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.compute.file_name = "shaders/voxel_copy.hlsl";
			mpso.compute.entry_point = "CS";
			mpso.compute.flags = 0;
			
			return mpso;
		}
	};
}
