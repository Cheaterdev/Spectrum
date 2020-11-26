#pragma once
namespace PSOS
{
	struct Lighting: public PSOBase
	{
		struct Keys {
KeyValue<NoValue,Nullable> SecondBounce;

 		GEN_DEF_COMP(Keys) };
		GEN_COMPUTE_PSO(Lighting,SecondBounce)
		GEN_KEY(SecondBounce,true);

		SimplePSO init_pso(Keys & key)
		{
			static const ShaderDefine<&Keys::SecondBounce, &SimpleComputePSO::compute> SecondBounce = "SECOND_BOUNCE";

			SimplePSO mpso("Lighting");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.compute.file_name = "shaders/voxel_lighting.hlsl";
			mpso.compute.entry_point = "CS";
			mpso.compute.flags = 0;
			SecondBounce.Apply(mpso, key);

			return mpso;
		}
	};
}
