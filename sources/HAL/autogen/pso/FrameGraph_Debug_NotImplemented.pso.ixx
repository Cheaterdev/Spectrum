export module HAL:Autogen.PSO.FrameGraph_Debug_NotImplemented;

import Core;
import :PSO;
import :Enums;
import :Types;
import <HAL.h>;

export namespace PSOS
{
	struct FrameGraph_Debug_NotImplemented: public PSOBase
	{
		struct Keys {
			GEN_DEF_COMP(Keys);
		private:
			SERIALIZE()
			{
			}
		};

		GEN_COMPUTE_PSO(FrameGraph_Debug_NotImplemented)


		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{


			SimplePSO mpso("FrameGraph_Debug_NotImplemented");
			if(f) f(mpso,key);

			mpso.root_signature = Layouts::DefaultLayout;

			mpso.compute.file_name = "shaders/framegraph/draw_not_implemented.hlsl";
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