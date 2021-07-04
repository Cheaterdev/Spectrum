#pragma once
namespace PSOS
{
	struct ShadowPass: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(ShadowPass)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("ShadowPass");
			mpso.root_signature = Layouts::;
			mpso.miss.file_name = "shaders/rayracing.hlsl";
			mpso.miss.entry_point = "ShadowMissShader";
			mpso.miss.flags = 0;
			mpso.closest_hit.file_name = "shaders/rayracing.hlsl";
			mpso.closest_hit.entry_point = "ShadowClosestHitShader";
			mpso.closest_hit.flags = 0;
			
			return mpso;
		}
	};
}
