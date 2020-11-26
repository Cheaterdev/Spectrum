#pragma once
namespace PSOS
{
	struct SkyCube: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(SkyCube)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("SkyCube");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/sky.hlsl";
			mpso.vertex.entry_point = "VS_Cube";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/sky.hlsl";
			mpso.pixel.entry_point = "PS_Cube";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT };
			mpso.blend = {  };
			return mpso;
		}
	};
}
