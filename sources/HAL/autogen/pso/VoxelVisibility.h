#pragma once
namespace PSOS
{
	struct VoxelVisibility: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_COMPUTE_PSO(VoxelVisibility)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("VoxelVisibility");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.compute.file_name = "shaders/voxel_visibility.hlsl";
			mpso.compute.entry_point = "CS";
			mpso.compute.flags = 0;
			
			return mpso;
		}
	};
}
