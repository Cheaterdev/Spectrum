#pragma once
namespace PSOS
{
	struct VoxelZero: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_COMPUTE_PSO(VoxelZero)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("VoxelZero");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.compute.file_name = "shaders/voxel_zero.hlsl";
			mpso.compute.entry_point = "CS";
			mpso.compute.flags = 0;
			
			return mpso;
		}
	};
}
