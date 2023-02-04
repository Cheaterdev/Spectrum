#pragma once
namespace PSOS
{
	struct DenoiserShadow_Filter: public PSOBase
	{
		struct Keys {
			KeyValue<int,NonNullable,0,1,2> Pass;

			GEN_DEF_COMP(Keys);
			private:
			SERIALIZE()
			{
				ar&NVP(Pass);
			}
		 };
		GEN_COMPUTE_PSO(DenoiserShadow_Filter,Pass)
		GEN_KEY(Pass,true);

		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{
			static const ShaderDefine<&Keys::Pass, &SimpleComputePSO::compute> Pass = "Pass";

			SimplePSO mpso("DenoiserShadow_Filter");
			if(f) f(mpso,key);
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.compute.file_name = "shaders/denoiser/filter_soft_shadows_pass_d3d12.hlsl";
			mpso.compute.entry_point = "main";
			mpso.compute.flags = HAL::ShaderOptions::FP16;
			Pass.Apply(mpso, key);

			return mpso;
		}
		private:
		SERIALIZE()
		{
			ar&NVP(wrap(psos));
		}
	};
}
