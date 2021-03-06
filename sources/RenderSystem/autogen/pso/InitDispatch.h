#pragma once
namespace PSOS
{
	struct InitDispatch: public PSOBase
	{
		struct Keys {
KeyValue<NoValue,Nullable> CheckFrustum;

 		GEN_DEF_COMP(Keys) };
		GEN_COMPUTE_PSO(InitDispatch,CheckFrustum)
		GEN_KEY(CheckFrustum,true);

		SimplePSO init_pso(Keys & key)
		{
			static const ShaderDefine<&Keys::CheckFrustum, &SimpleComputePSO::compute> CheckFrustum = "CHECK_FRUSTUM";

			SimplePSO mpso("InitDispatch");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.compute.file_name = "shaders/occluder_cs_dispatch_init.hlsl";
			mpso.compute.entry_point = "CS";
			mpso.compute.flags = 0;
			CheckFrustum.Apply(mpso, key);

			return mpso;
		}
	};
}
