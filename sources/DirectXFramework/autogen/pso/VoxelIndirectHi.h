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
			
			mpso.rtv_formats = { DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT };
			mpso.blend = { Graphics::Blend::Additive, Graphics::Blend::None };
			mpso.enable_stencil  = true;
			mpso.enable_depth  = false;
			mpso.stencil_func  = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_EQUAL;
			mpso.stencil_pass_op  = D3D12_STENCIL_OP::D3D12_STENCIL_OP_KEEP;
			mpso.ds  = DXGI_FORMAT_D24_UNORM_S8_UINT;
			mpso.stencil_read_mask  = 1;
			mpso.stencil_write_mask  = 1;
			return mpso;
		}
	};
}
