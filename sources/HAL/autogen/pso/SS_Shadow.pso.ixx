export module HAL:Autogen.PSO.SS_Shadow;

import Core;
import :PSO;
import :Enums;
import :Types;


export namespace PSOS
{
	struct SS_Shadow: public PSOBase
	{
		struct Keys {
			GEN_DEF_COMP(Keys);
		private:
			SERIALIZE()
			{
			}
		};

		GEN_COMPUTE_PSO(SS_Shadow)


		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{


			SimplePSO mpso("SS_Shadow");
			if(f) f(mpso,key);

			mpso.root_signature = Layouts::DefaultLayout;

			mpso.compute.file_name = "shaders/SS_Shadow.hlsl";
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