#pragma once
namespace PSOS
{
	struct VoxelDebug: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(VoxelDebug)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("VoxelDebug");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/voxel_screen.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/voxel_screen_debug.hlsl";
			mpso.pixel.entry_point = "Debug";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { Graphics::Format::R16G16B16A16_FLOAT };
			mpso.blend = {  };
			mpso.enable_depth  = false;
			return mpso;
		}
	};
}
