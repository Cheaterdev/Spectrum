#pragma once
namespace PSOS
{
	struct DrawStencil: public PSOBase
	{
		struct Keys {
			
			GEN_DEF_COMP(Keys);
			private:
			SERIALIZE()
			{
			}
		 };
		GEN_GRAPHICS_PSO(DrawStencil)
		
		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{
			
			SimplePSO mpso("DrawStencil");
			if(f) f(mpso,key);
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.mesh.file_name = "shaders/mesh_shader.hlsl";
			mpso.mesh.entry_point = "VS";
			mpso.mesh.flags = HAL::ShaderOptions::None;
			mpso.amplification.file_name = "shaders/mesh_shader.hlsl";
			mpso.amplification.entry_point = "AS";
			mpso.amplification.flags = HAL::ShaderOptions::None;
			mpso.pixel.file_name = "shaders/stencil.hlsl";
			mpso.pixel.entry_point = "PS";
			mpso.pixel.flags = HAL::ShaderOptions::None;
			
			mpso.rtv_formats = {  };
			mpso.blend = {  };
			mpso.ds  = HAL::Format::D32_FLOAT;
			mpso.cull  = HAL::CullMode::None;
			mpso.depth_func  = HAL::ComparisonFunc::LESS;
			return mpso;
		}
		private:
		SERIALIZE()
		{
			ar&NVP(wrap(psos));
		}
	};
}
