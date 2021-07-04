#pragma once
namespace PSOS
{
	struct Reflection: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(Reflection)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("Reflection");
			mpso.root_signature = Layouts::;
			mpso.raygen.file_name = "shaders/rayracing.hlsl";
			mpso.raygen.entry_point = "MyRaygenShaderReflection";
			mpso.raygen.flags = 0;
			
			return mpso;
		}
	};
}
