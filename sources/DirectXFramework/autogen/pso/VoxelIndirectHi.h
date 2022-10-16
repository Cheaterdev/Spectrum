#pragma once
namespace PSOS
{
	struct VoxelIndirectHi: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(VoxelIndirectHi)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("VoxelIndirectHi");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/voxel_screen.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/voxel_screen.hlsl";
			mpso.pixel.entry_point = "PS";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { Graphics::Format::R16G16B16A16_FLOAT, Graphics::Format::R11G11B10_FLOAT };
			mpso.blend = { HAL::Blends::Additive, HAL::Blends::None };
			mpso.enable_stencil  = true;
			mpso.enable_depth  = false;
			mpso.stencil_func  = Graphics::ComparisonFunc::EQUAL;
			mpso.stencil_pass_op  = Graphics::StencilOp::Keep;
			mpso.ds  = Graphics::Format::D24_UNORM_S8_UINT;
			mpso.stencil_read_mask  = 1;
			mpso.stencil_write_mask  = 1;
			return mpso;
		}
	};
}
