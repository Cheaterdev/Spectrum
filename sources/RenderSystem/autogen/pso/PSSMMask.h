#pragma once
namespace PSOS
{
	struct PSSMMask: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(PSSMMask)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("PSSMMask");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/PSSM.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/PSSM.hlsl";
			mpso.pixel.entry_point = "PS";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R8_UNORM };
			mpso.blend = {  };
			return mpso;
		}
	};
}
