#pragma once
namespace PSOS
{
	struct DenoiserHistoryFix: public PSOBase
	{
		struct Keys {
			
			GEN_DEF_COMP(Keys);
			private:
			SERIALIZE()
			{
			}
		 };
		GEN_COMPUTE_PSO(DenoiserHistoryFix)
		
		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{
			
			SimplePSO mpso("DenoiserHistoryFix");
			if(f) f(mpso,key);
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.compute.file_name = "shaders/DenoiserHistoryFix.hlsl";
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
