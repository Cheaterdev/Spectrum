#pragma once
namespace PSOS
{
	struct SimpleRect: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(SimpleRect)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("SimpleRect");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/gui/rect.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/gui/rect.hlsl";
			mpso.pixel.entry_point = "PS_COLOR";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { HAL::Format::R8G8B8A8_UNORM };
			mpso.blend = { HAL::Blends::AlphaBlend };
			mpso.cull  = HAL::CullMode::None;
			return mpso;
		}
	};
}
