#pragma once

namespace PSOS
{
	struct FrameGraph_Debug_Texture2D: public PSOBase
	{
		struct Keys {
			GEN_DEF_COMP(Keys);
		private:
			SERIALIZE()
			{
			}
		};

		GEN_COMPUTE_PSO(FrameGraph_Debug_Texture2D)


		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{


			SimplePSO mpso("FrameGraph_Debug_Texture2D");
			if(f) f(mpso,key);

			mpso.root_signature = Layouts::DefaultLayout;

			mpso.compute.file_name = "shaders/framegraph/draw_texture_2d.hlsl";
			mpso.compute.entry_point = "CS";
			mpso.compute.flags = HAL::ShaderOptions::None;
			
			return mpso;
		}

		private:
		SERIALIZE()
		{
			ar&NVP(wrap(psos));
		}
	};
}