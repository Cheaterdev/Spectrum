#pragma once
namespace PSOS
{
	struct Shadow: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(Shadow)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("Shadow");
			mpso.root_signature = Layouts::;
			mpso.raygen.file_name = "shaders/rayracing.hlsl";
			mpso.raygen.entry_point = "ShadowRaygenShader";
			mpso.raygen.flags = 0;
			
			return mpso;
		}
	};
}
