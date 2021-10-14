#pragma once
namespace PSOS
{
	struct VoxelReflectionResize: public PSOBase
	{
		struct Keys{

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(VoxelReflectionResize)
		
		SimplePSO init_pso(Keys & key)
		{
						SimplePSO mpso("VoxelReflectionResize");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/voxel_screen.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/voxel_screen_reflection.hlsl";
			mpso.pixel.entry_point = "PS_resize";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT };
			mpso.blend = {  };
			mpso.enable_stencil  = true;
			mpso.enable_depth  = false;
			mpso.stencil_func  = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_ALWAYS;
			mpso.stencil_pass_op  = D3D12_STENCIL_OP::D3D12_STENCIL_OP_KEEP;
			mpso.ds  = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
			mpso.stencil_read_mask  = 255;
			mpso.stencil_write_mask  = 255;
			return mpso;
		}
	};
}
