#pragma once
namespace PSOS
{
	struct BRDF: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_COMPUTE_PSO(BRDF)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("BRDF");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.compute.file_name = "shaders/BRDF.hlsl";
			mpso.compute.entry_point = "CS";
			mpso.compute.flags = 0;
			
			return mpso;
		}
	};
}
