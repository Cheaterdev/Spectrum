#pragma once
namespace PSOS
{
	struct GBufferDownsample: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(GBufferDownsample)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("GBufferDownsample");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/Downsample.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/Downsample.hlsl";
			mpso.pixel.entry_point = "PS";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { HAL::Format::R32_FLOAT, HAL::Format::R8G8B8A8_UNORM };
			mpso.blend = {  };
			return mpso;
		}
	};
}