#pragma once
namespace PSOS
{
	struct DrawBox: public PSOBase
	{
		struct Keys {
			
			GEN_DEF_COMP(Keys);
			private:
			SERIALIZE()
			{
			}
		 };
		GEN_GRAPHICS_PSO(DrawBox)
		
		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{
			
			SimplePSO mpso("DrawBox");
			if(f) f(mpso,key);
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/triangle_stencil.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = HAL::ShaderOptions::None;
			mpso.pixel.file_name = "shaders/triangle_stencil.hlsl";
			mpso.pixel.entry_point = "PS";
			mpso.pixel.flags = HAL::ShaderOptions::None;
			
			mpso.rtv_formats = { HAL::Format::R16G16B16A16_FLOAT };
			mpso.blend = { HAL::Blends::Additive };
			mpso.enable_depth  = false;
			mpso.cull  = HAL::CullMode::None;
			return mpso;
		}
		private:
		SERIALIZE()
		{
			ar&NVP(wrap(psos));
		}
	};
}
