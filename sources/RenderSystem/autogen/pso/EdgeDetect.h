#pragma once
namespace PSOS
{
	struct EdgeDetect: public PSOBase
	{
		struct Keys{

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(EdgeDetect)
		
		SimplePSO init_pso(Keys & key)
		{
						SimplePSO mpso("EdgeDetect");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/SMAA.hlsl";
			mpso.vertex.entry_point = "DX10_SMAAEdgeDetectionVS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/SMAA.hlsl";
			mpso.pixel.entry_point = "DX10_SMAALumaEdgeDetectionPS";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R8G8_UNORM };
			mpso.blend = {  };
			return mpso;
		}
	};
}
