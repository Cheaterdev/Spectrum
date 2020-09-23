#pragma once
namespace PSOS
{
	struct DrawSelected: public PSOBase
	{
		struct Keys{

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(DrawSelected)
		
		SimplePSO init_pso(Keys & key)
		{
						SimplePSO mpso("DrawSelected");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/triangle.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/stencil.hlsl";
			mpso.pixel.entry_point = "PS_RESULT";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R8_SNORM };
			mpso.blend = {  };
			mpso.enable_depth  = false;
			mpso.cull  = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
			mpso.depth_func  = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_ALWAYS;
			return mpso;
		}
	};
}
