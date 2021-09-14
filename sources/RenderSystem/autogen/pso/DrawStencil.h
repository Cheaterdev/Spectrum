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
			mpso.ds  = DXGI_FORMAT_D32_FLOAT;
			mpso.cull  = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
			mpso.depth_func  = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS;
			return mpso;
		}
	};
}
