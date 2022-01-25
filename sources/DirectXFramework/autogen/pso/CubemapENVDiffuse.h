#pragma once
namespace PSOS
{
	struct CubemapENVDiffuse: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(CubemapENVDiffuse)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("CubemapENVDiffuse");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/cubemap_down.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/cubemap_down.hlsl";
			mpso.pixel.entry_point = "PS_Diffuse";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { Graphics::Format::R11G11B10_FLOAT };
			mpso.blend = {  };
			return mpso;
		}
	};
}
