export module HAL:Autogen.PSO.FrameGraph_Debug_TextureCube;

import Core;
import :PSO;
import :Enums;
import :Types;


export namespace PSOS
{
	struct FrameGraph_Debug_TextureCube: public PSOBase
	{
		struct Keys {
			GEN_DEF_COMP(Keys);
		private:
			SERIALIZE()
			{
			}
		};

		GEN_COMPUTE_PSO(FrameGraph_Debug_TextureCube)


		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{


			SimplePSO mpso("FrameGraph_Debug_TextureCube");
			if(f) f(mpso,key);

			mpso.root_signature = Layouts::DefaultLayout;

			mpso.compute.file_name = "shaders/framegraph/draw_texture_cube.hlsl";
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