#pragma once
namespace PSOS
{
	struct DenoiserHistoryFix: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_COMPUTE_PSO(DenoiserHistoryFix)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("DenoiserHistoryFix");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.compute.file_name = "shaders/DenoiserHistoryFix.hlsl";
			mpso.compute.entry_point = "CS";
			mpso.compute.flags = 0;
			
			return mpso;
		}
	};
}
