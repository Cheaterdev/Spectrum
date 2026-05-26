export module HAL:Autogen.PSO.WorkGR;

import Core;
import :PSO;
import :Enums;
import :Types;
import <HAL.h>;

export namespace PSOS
{
	struct WorkGR: public PSOBase
	{
		struct Keys {
			GEN_DEF_COMP(Keys);
		private:
			SERIALIZE()
			{
			}
		};

		GEN_WORKGRAPH_PSO(WorkGR)


		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{


			SimplePSO mpso("WorkGR");
			if(f) f(mpso,key);

			mpso.root_signature = Layouts::DefaultLayout;

			mpso.shader.file_name = "shaders/workgraph_test.hlsl";
			mpso.shader.entry_point = "";
			mpso.shader.flags = HAL::ShaderOptions::None;
			
			return mpso;
		}

		private:
		SERIALIZE()
		{
			ar&NVP(wrap(psos));
		}
	};
}