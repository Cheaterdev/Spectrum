#pragma once
namespace PSOS
{
	struct Sky: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(Sky)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("Sky");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/sky.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/sky.hlsl";
			mpso.pixel.entry_point = "PS";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT };
			mpso.blend = {  };
			return mpso;
		}
	};
}
