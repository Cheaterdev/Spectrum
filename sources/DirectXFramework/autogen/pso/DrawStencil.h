#pragma once
namespace PSOS
{
	struct DrawStencil: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(DrawStencil)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("DrawStencil");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.mesh.file_name = "shaders/mesh_shader.hlsl";
			mpso.mesh.entry_point = "VS";
			mpso.mesh.flags = 0;
			mpso.amplification.file_name = "shaders/mesh_shader.hlsl";
			mpso.amplification.entry_point = "AS";
			mpso.amplification.flags = 0;
			mpso.pixel.file_name = "shaders/stencil.hlsl";
			mpso.pixel.entry_point = "PS";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = {  };
			mpso.blend = {  };
			mpso.ds  = Graphics::Format::D32_FLOAT;
			mpso.cull  = Graphics::CullMode::None;
			mpso.depth_func  = Graphics::ComparisonFunc::LESS;
			return mpso;
		}
	};
}
