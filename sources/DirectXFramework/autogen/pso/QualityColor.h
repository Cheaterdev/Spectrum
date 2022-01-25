#pragma once
namespace PSOS
{
	struct QualityColor: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(QualityColor)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("QualityColor");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/gbuffer_quality.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/gbuffer_quality.hlsl";
			mpso.pixel.entry_point = "PS";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { Graphics::Format::R8G8_UNORM };
			mpso.blend = {  };
			mpso.depth_write  = false;
			return mpso;
		}
	};
}
