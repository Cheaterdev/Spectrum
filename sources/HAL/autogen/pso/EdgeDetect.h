#pragma once
namespace PSOS
{
	struct EdgeDetect: public PSOBase
	{
		struct Keys {
			
			GEN_DEF_COMP(Keys);
			private:
			SERIALIZE()
			{
			}
		 };
		GEN_GRAPHICS_PSO(EdgeDetect)
		
		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{
			
			SimplePSO mpso("EdgeDetect");
			if(f) f(mpso,key);
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/SMAA.hlsl";
			mpso.vertex.entry_point = "DX10_SMAAEdgeDetectionVS";
			mpso.vertex.flags = HAL::ShaderOptions::None;
			mpso.pixel.file_name = "shaders/SMAA.hlsl";
			mpso.pixel.entry_point = "DX10_SMAALumaEdgeDetectionPS";
			mpso.pixel.flags = HAL::ShaderOptions::None;
			
			mpso.rtv_formats = { HAL::Format::R8G8_UNORM };
			mpso.blend = {  };
			return mpso;
		}
		private:
		SERIALIZE()
		{
			ar&NVP(wrap(psos));
		}
	};
}
