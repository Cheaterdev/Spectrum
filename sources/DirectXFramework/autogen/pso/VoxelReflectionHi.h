#pragma once
namespace PSOS
{
	struct VoxelReflectionHi: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(VoxelReflectionHi)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("VoxelReflectionHi");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/voxel_screen.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/voxel_screen_reflection.hlsl";
			mpso.pixel.entry_point = "PS";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { Graphics::Format::R16G16B16A16_FLOAT };
			mpso.blend = { Graphics::Blends::Additive };
			mpso.enable_stencil  = true;
			mpso.enable_depth  = false;
			mpso.stencil_func  = Graphics::ComparisonFunc::ALWAYS;
			mpso.stencil_pass_op  = Graphics::StencilOp::Keep;
			mpso.ds  = Graphics::Format::D24_UNORM_S8_UINT;
			mpso.stencil_read_mask  = 2;
			mpso.stencil_write_mask  = 2;
			return mpso;
		}
	};
}
