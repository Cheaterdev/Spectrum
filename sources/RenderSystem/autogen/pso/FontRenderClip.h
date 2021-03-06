#pragma once
namespace PSOS
{
	struct FontRenderClip: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(FontRenderClip)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("FontRenderClip");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/font/vsSimple.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/font/psSimple.hlsl";
			mpso.pixel.entry_point = "PS";
			mpso.pixel.flags = 0;
			mpso.geometry.file_name = "shaders/font/gsClip.hlsl";
			mpso.geometry.entry_point = "GS";
			mpso.geometry.flags = 0;
			
			mpso.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM };
			mpso.blend = {  };
			mpso.topology  = D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
			return mpso;
		}
	};
}
