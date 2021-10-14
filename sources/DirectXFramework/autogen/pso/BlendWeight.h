#pragma once
namespace PSOS
{
	struct BlendWeight: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(BlendWeight)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("BlendWeight");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/SMAA.hlsl";
			mpso.vertex.entry_point = "DX10_SMAABlendingWeightCalculationVS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/SMAA.hlsl";
			mpso.pixel.entry_point = "DX10_SMAABlendingWeightCalculationPS";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM };
			mpso.blend = {  };
			return mpso;
		}
	};
}
