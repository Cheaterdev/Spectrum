#pragma once
namespace PSOS
{
	struct VoxelReflectionUpsample: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(VoxelReflectionUpsample)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("VoxelReflectionUpsample");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/voxel_screen.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/voxel_screen_reflection.hlsl";
			mpso.pixel.entry_point = "PS_resize";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { HAL::Format::R16G16B16A16_FLOAT };
			mpso.blend = { HAL::Blends::Additive };
			mpso.enable_stencil  = true;
			mpso.enable_depth  = false;
			mpso.stencil_func  = HAL::ComparisonFunc::ALWAYS;
			mpso.stencil_pass_op  = HAL::StencilOp::Keep;
			mpso.ds  = HAL::Format::D24_UNORM_S8_UINT;
			mpso.stencil_read_mask  = 2;
			mpso.stencil_write_mask  = 2;
			return mpso;
		}
	};
}
