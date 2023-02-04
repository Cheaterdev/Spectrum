#pragma once
namespace PSOS
{
	struct VoxelIndirectUpsample: public PSOBase
	{
		struct Keys {
			
			GEN_DEF_COMP(Keys);
			private:
			SERIALIZE()
			{
			}
		 };
		GEN_GRAPHICS_PSO(VoxelIndirectUpsample)
		
		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{
			
			SimplePSO mpso("VoxelIndirectUpsample");
			if(f) f(mpso,key);
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/voxel_screen.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = HAL::ShaderOptions::None;
			mpso.pixel.file_name = "shaders/voxel_screen.hlsl";
			mpso.pixel.entry_point = "PS_Resize";
			mpso.pixel.flags = HAL::ShaderOptions::None;
			
			mpso.rtv_formats = { HAL::Format::R16G16B16A16_FLOAT, HAL::Format::R11G11B10_FLOAT };
			mpso.blend = { HAL::Blends::Additive, HAL::Blends::None };
			mpso.enable_stencil  = true;
			mpso.enable_depth  = false;
			mpso.stencil_func  = HAL::ComparisonFunc::EQUAL;
			mpso.stencil_pass_op  = HAL::StencilOp::Keep;
			mpso.ds  = HAL::Format::D24_UNORM_S8_UINT;
			mpso.stencil_read_mask  = 1;
			mpso.stencil_write_mask  = 1;
			return mpso;
		}
		private:
		SERIALIZE()
		{
			ar&NVP(wrap(psos));
		}
	};
}
