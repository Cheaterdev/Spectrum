#pragma once
namespace PSOS
{
	struct DrawBox: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(DrawBox)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("DrawBox");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/triangle_stencil.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/triangle_stencil.hlsl";
			mpso.pixel.entry_point = "PS";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { HAL::Format::R16G16B16A16_FLOAT };
			mpso.blend = { HAL::Blends::Additive };
			mpso.enable_depth  = false;
			mpso.cull  = HAL::CullMode::None;
			return mpso;
		}
	};
}