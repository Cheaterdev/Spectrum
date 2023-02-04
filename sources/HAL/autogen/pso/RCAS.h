#pragma once
namespace PSOS
{
	struct RCAS: public PSOBase
	{
		struct Keys {
			KeyValue<NoValue,Nullable> cas;

			GEN_DEF_COMP(Keys);
			private:
			SERIALIZE()
			{
				ar&NVP(cas);
			}
		 };
		GEN_COMPUTE_PSO(RCAS,cas)
		GEN_KEY(cas,true);

		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{
			static const ShaderDefine<&Keys::cas, &SimpleComputePSO::compute> cas = "RCAS";

			SimplePSO mpso("RCAS");
			if(f) f(mpso,key);
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.compute.file_name = "shaders/FSR.hlsl";
			mpso.compute.entry_point = "CS";
			mpso.compute.flags = HAL::ShaderOptions::None;
			cas.Apply(mpso, key);

			return mpso;
		}
		private:
		SERIALIZE()
		{
			ar&NVP(wrap(psos));
		}
	};
}
