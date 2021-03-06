#pragma once
namespace PSOS
{
	struct CanvasBack: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(CanvasBack)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("CanvasBack");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/gui/ninepatch.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/gui/canvas.hlsl";
			mpso.pixel.entry_point = "PS";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM };
			mpso.blend = { Render::Blend::AlphaBlend };
			mpso.enable_depth  = false;
			mpso.cull  = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
			mpso.topology  = D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			return mpso;
		}
	};
}
