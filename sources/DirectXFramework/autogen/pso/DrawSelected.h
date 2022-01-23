#pragma once
namespace PSOS
{
	struct DrawSelected: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(DrawSelected)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("DrawSelected");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.mesh.file_name = "shaders/mesh_shader.hlsl";
			mpso.mesh.entry_point = "VS";
			mpso.mesh.flags = 0;
			mpso.amplification.file_name = "shaders/mesh_shader.hlsl";
			mpso.amplification.entry_point = "AS";
			mpso.amplification.flags = 0;
			mpso.pixel.file_name = "shaders/stencil.hlsl";
			mpso.pixel.entry_point = "PS_RESULT";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R8_SNORM };
			mpso.blend = {  };
			mpso.enable_depth  = false;
			mpso.cull  = Graphics::CullMode::None;
			mpso.depth_func  = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_ALWAYS;
			return mpso;
		}
	};
}
