#pragma once
namespace PSOS
{
	struct NinePatch: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(NinePatch)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("NinePatch");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/gui/ninepatch.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/gui/ninepatch.hlsl";
			mpso.pixel.entry_point = "PS";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM };
			mpso.blend = { Graphics::Blend::AlphaBlend };
			return mpso;
		}
	};
}
