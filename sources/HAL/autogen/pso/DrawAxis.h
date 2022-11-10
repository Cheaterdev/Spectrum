#pragma once
namespace PSOS
{
	struct DrawAxis: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(DrawAxis)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("DrawAxis");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.mesh.file_name = "shaders/mesh_shader.hlsl";
			mpso.mesh.entry_point = "VS";
			mpso.mesh.flags = 0;
			mpso.amplification.file_name = "shaders/mesh_shader.hlsl";
			mpso.amplification.entry_point = "AS";
			mpso.amplification.flags = 0;
			mpso.pixel.file_name = "shaders/stencil.hlsl";
			mpso.pixel.entry_point = "PS_COLOR";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { HAL::Format::R16G16B16A16_FLOAT };
			mpso.blend = {  };
			mpso.enable_depth  = false;
			mpso.cull  = HAL::CullMode::None;
			return mpso;
		}
	};
}
