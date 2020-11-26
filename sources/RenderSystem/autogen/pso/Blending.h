#pragma once
namespace PSOS
{
	struct Blending: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(Blending)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("Blending");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/SMAA.hlsl";
			mpso.vertex.entry_point = "DX10_SMAANeighborhoodBlendingVS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/SMAA.hlsl";
			mpso.pixel.entry_point = "DX10_SMAANeighborhoodBlendingPS";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT };
			mpso.blend = {  };
			return mpso;
		}
	};
}
