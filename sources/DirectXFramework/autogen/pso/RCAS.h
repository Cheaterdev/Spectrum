#pragma once
namespace PSOS
{
	struct RCAS: public PSOBase
	{
		struct Keys {
KeyValue<NoValue,Nullable> cas;

 		GEN_DEF_COMP(Keys) };
		GEN_COMPUTE_PSO(RCAS,cas)
		GEN_KEY(cas,true);

		SimplePSO init_pso(Keys & key)
		{
			static const ShaderDefine<&Keys::cas, &SimpleComputePSO::compute> cas = "RCAS";

			SimplePSO mpso("RCAS");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.compute.file_name = "shaders/FSR.hlsl";
			mpso.compute.entry_point = "CS";
			mpso.compute.flags = 0;
			cas.Apply(mpso, key);

			return mpso;
		}
	};
}
