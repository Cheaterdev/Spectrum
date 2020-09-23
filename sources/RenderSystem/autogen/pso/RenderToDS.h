#pragma once
namespace PSOS
{
	struct RenderToDS: public PSOBase
	{
		struct Keys{

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
			mpso.ds  = DXGI_FORMAT_D32_FLOAT;
			mpso.cull  = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
			mpso.depth_func  = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_ALWAYS;
			return mpso;
		}
	};
}
