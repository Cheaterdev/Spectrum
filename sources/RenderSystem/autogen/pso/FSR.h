#pragma once
namespace PSOS
{
	struct FSR: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_COMPUTE_PSO(FSR)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("FSR");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.compute.file_name = "shaders/FSR.hlsl";
			mpso.compute.entry_point = "CS";
			mpso.compute.flags = 0;
			
			return mpso;
		}
	};
}
