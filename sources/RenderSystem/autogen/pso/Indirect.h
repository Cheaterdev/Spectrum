#pragma once
namespace PSOS
{
	struct Indirect: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(Indirect)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("Indirect");
			mpso.root_signature = Layouts::;
			mpso.raygen.file_name = "shaders/rayracing.hlsl";
			mpso.raygen.entry_point = "MyRaygenShader";
			mpso.raygen.flags = 0;
			
			return mpso;
		}
	};
}
