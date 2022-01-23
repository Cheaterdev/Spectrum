#pragma once
namespace PSOS
{
	struct QualityToStencilREfl: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(QualityToStencilREfl)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("QualityToStencilREfl");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/gbuffer_quality.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/gbuffer_quality.hlsl";
			mpso.pixel.entry_point = "PS_STENCIL";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = {  };
			mpso.blend = {  };
			mpso.enable_stencil  = true;
			mpso.enable_depth  = false;
			mpso.stencil_func  = Graphics::ComparisonFunc::ALWAYS;
			mpso.stencil_pass_op  = Graphics::StencilOp::Replace;
			mpso.ds  = DXGI_FORMAT_D24_UNORM_S8_UINT;
			mpso.stencil_read_mask  = 2;
			mpso.stencil_write_mask  = 2;
			return mpso;
		}
	};
}
