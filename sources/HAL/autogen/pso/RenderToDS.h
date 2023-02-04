#pragma once
namespace PSOS
{
	struct RenderToDS: public PSOBase
	{
		struct Keys {
			
			GEN_DEF_COMP(Keys);
			private:
			SERIALIZE()
			{
			}
		 };
		GEN_GRAPHICS_PSO(RenderToDS)
		
		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{
			
			SimplePSO mpso("RenderToDS");
			if(f) f(mpso,key);
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/depth_render.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = HAL::ShaderOptions::None;
			mpso.pixel.file_name = "shaders/depth_render.hlsl";
			mpso.pixel.entry_point = "PS";
			mpso.pixel.flags = HAL::ShaderOptions::None;
			
			mpso.rtv_formats = {  };
			mpso.blend = {  };
			mpso.ds  = HAL::Format::D32_FLOAT;
			mpso.cull  = HAL::CullMode::None;
			mpso.depth_func  = HAL::ComparisonFunc::ALWAYS;
			return mpso;
		}
		private:
		SERIALIZE()
		{
			ar&NVP(wrap(psos));
		}
	};
}
