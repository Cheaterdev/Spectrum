#pragma once
namespace PSOS
{
	struct VoxelReflectionLow: public PSOBase
	{
		struct Keys{

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(VoxelReflectionLow)
		
		SimplePSO init_pso(Keys & key)
		{
						SimplePSO mpso("VoxelReflectionLow");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/voxel_screen.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/voxel_screen_reflection.hlsl";
			mpso.pixel.entry_point = "PS_low";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = {  };
			mpso.blend = {  };
			mpso.enable_depth  = false;
			return mpso;
		}
	};
}
