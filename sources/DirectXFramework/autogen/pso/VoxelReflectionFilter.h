#pragma once
namespace PSOS
{
	struct VoxelReflectionFilter: public PSOBase
	{
		struct Keys{

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(VoxelReflectionFilter)
		
		SimplePSO init_pso(Keys & key)
		{
						SimplePSO mpso("VoxelReflectionFilter");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/voxel_screen.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/voxel_screen_blur.hlsl";
			mpso.pixel.entry_point = "PS";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = {  };
			mpso.blend = {  };
			mpso.enable_depth  = false;
			return mpso;
		}
	};
}
