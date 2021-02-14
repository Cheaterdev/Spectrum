#pragma once
namespace PSOS
{
	struct FrameClassificationInitDispatch: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_COMPUTE_PSO(FrameClassificationInitDispatch)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("FrameClassificationInitDispatch");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.compute.file_name = "shaders/FrameClassificationInitDispatch.hlsl";
			mpso.compute.entry_point = "CS";
			mpso.compute.flags = 0;
			
			return mpso;
		}
	};
}
