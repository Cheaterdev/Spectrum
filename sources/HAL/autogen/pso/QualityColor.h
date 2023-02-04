#pragma once
namespace PSOS
{
	struct QualityColor: public PSOBase
	{
		struct Keys {
			
			GEN_DEF_COMP(Keys);
			private:
			SERIALIZE()
			{
			}
		 };
		GEN_GRAPHICS_PSO(QualityColor)
		
		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{
			
			SimplePSO mpso("QualityColor");
			if(f) f(mpso,key);
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/gbuffer_quality.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = HAL::ShaderOptions::None;
			mpso.pixel.file_name = "shaders/gbuffer_quality.hlsl";
			mpso.pixel.entry_point = "PS";
			mpso.pixel.flags = HAL::ShaderOptions::None;
			
			mpso.rtv_formats = { HAL::Format::R8G8_UNORM };
			mpso.blend = {  };
			mpso.depth_write  = false;
			return mpso;
		}
		private:
		SERIALIZE()
		{
			ar&NVP(wrap(psos));
		}
	};
}
