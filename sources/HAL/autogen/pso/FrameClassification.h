#pragma once
namespace PSOS
{
	struct FrameClassification: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_COMPUTE_PSO(FrameClassification)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("FrameClassification");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.compute.file_name = "shaders/FrameClassification.hlsl";
			mpso.compute.entry_point = "CS";
			mpso.compute.flags = 0;
			
			return mpso;
		}
	};
}
