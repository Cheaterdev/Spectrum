#pragma once
namespace PSOS
{
	struct DenoiserDownsample: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(DenoiserDownsample)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("DenoiserDownsample");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/DenoiserDownsample.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/DenoiserDownsample.hlsl";
			mpso.pixel.entry_point = "PS";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT };
			mpso.blend = {  };
			return mpso;
		}
	};
}
