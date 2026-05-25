export module HAL:Autogen.PSO.DenoiserShadow_TileClassification;

import Core;
import :PSO;
import :Enums;
import :Types;
import <HAL.h>;

export namespace PSOS
{
	struct DenoiserShadow_TileClassification: public PSOBase
	{
		struct Keys {
			GEN_DEF_COMP(Keys);
		private:
			SERIALIZE()
			{
			}
		};

		GEN_COMPUTE_PSO(DenoiserShadow_TileClassification)


		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{


			SimplePSO mpso("DenoiserShadow_TileClassification");
			if(f) f(mpso,key);

			mpso.root_signature = Layouts::DefaultLayout;

			mpso.compute.file_name = "shaders/denoiser/tile_classification_d3d12.hlsl";
			mpso.compute.entry_point = "main";
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