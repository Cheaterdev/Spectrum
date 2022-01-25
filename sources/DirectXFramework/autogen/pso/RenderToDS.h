#pragma once
namespace PSOS
{
	struct RenderToDS: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(RenderToDS)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("RenderToDS");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/depth_render.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/depth_render.hlsl";
			mpso.pixel.entry_point = "PS";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = {  };
			mpso.blend = {  };
			mpso.ds  = Graphics::Format::D32_FLOAT;
			mpso.cull  = Graphics::CullMode::None;
			mpso.depth_func  = Graphics::ComparisonFunc::ALWAYS;
			return mpso;
		}
	};
}
