#pragma once
namespace PSOS
{
	struct DownsampleDepth: public PSOBase
	{
		struct Keys{

 		GEN_DEF_COMP(Keys) };
		GEN_COMPUTE_PSO(DownsampleDepth)
		
		SimplePSO init_pso(Keys & key)
		{
						SimplePSO mpso("DownsampleDepth");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.compute.file_name = "shaders/downsample_depth.hlsl";
			mpso.compute.entry_point = "CS";
			mpso.compute.flags = 0;
			
			return mpso;
		}
	};
}
