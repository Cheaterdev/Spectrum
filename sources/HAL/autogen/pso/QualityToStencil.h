#pragma once
namespace PSOS
{
	struct QualityToStencil: public PSOBase
	{
		struct Keys {
			
			GEN_DEF_COMP(Keys);
			private:
			SERIALIZE()
			{
			}
		 };
		GEN_GRAPHICS_PSO(QualityToStencil)
		
		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{
			
			SimplePSO mpso("QualityToStencil");
			if(f) f(mpso,key);
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/gbuffer_quality.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = HAL::ShaderOptions::None;
			mpso.pixel.file_name = "shaders/gbuffer_quality.hlsl";
			mpso.pixel.entry_point = "PS_STENCIL";
			mpso.pixel.flags = HAL::ShaderOptions::None;
			
			mpso.rtv_formats = {  };
			mpso.blend = {  };
			mpso.enable_stencil  = true;
			mpso.enable_depth  = false;
			mpso.stencil_func  = HAL::ComparisonFunc::ALWAYS;
			mpso.stencil_pass_op  = HAL::StencilOp::Replace;
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
