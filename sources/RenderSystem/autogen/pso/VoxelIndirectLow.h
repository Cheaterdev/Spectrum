#pragma once
namespace PSOS
{
	struct VoxelIndirectLow: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_COMPUTE_PSO(VoxelIndirectLow)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("VoxelIndirectLow");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.compute.file_name = "shaders/voxel_screen.hlsl";
			mpso.compute.entry_point = "CS";
			mpso.compute.flags = 0;
			
			return mpso;
		}
	};
}
