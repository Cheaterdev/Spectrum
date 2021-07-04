#pragma once
namespace PSOS
{
	struct MaterialPass: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(MaterialPass)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("MaterialPass");
			mpso.root_signature = Layouts::;
			mpso.miss.file_name = "shaders/rayracing.hlsl";
			mpso.miss.entry_point = "MyMissShader";
			mpso.miss.flags = 0;
			mpso.closest_hit.file_name = "shaders/none.hlsl";
			mpso.closest_hit.entry_point = "MyClosestHitShader";
			mpso.closest_hit.flags = 0;
			
			return mpso;
		}
	};
}
