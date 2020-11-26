#pragma once
namespace PSOS
{
	struct PSSMApply: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(PSSMApply)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("PSSMApply");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/PSSM.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/PSSM.hlsl";
			mpso.pixel.entry_point = "PS_RESULT";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT };
			mpso.blend = {  };
			return mpso;
		}
	};
}
