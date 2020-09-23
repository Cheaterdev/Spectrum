#pragma once
namespace PSOS
{
	struct DrawAxis: public PSOBase
	{
		struct Keys{

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(DrawAxis)
		
		SimplePSO init_pso(Keys & key)
		{
						SimplePSO mpso("DrawAxis");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/triangle.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/stencil.hlsl";
			mpso.pixel.entry_point = "PS_COLOR";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT };
			mpso.blend = {  };
			mpso.enable_depth  = false;
			mpso.cull  = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
			return mpso;
		}
	};
}