#pragma once
namespace PSOS
{
	struct GatherMeshes: public PSOBase
	{
		struct Keys{
KeyValue<NoValue,Nullable> Invisible;

 		GEN_DEF_COMP(Keys) };
		GEN_COMPUTE_PSO(GatherMeshes,Invisible)
		GEN_KEY(Invisible,true);

		SimplePSO init_pso(Keys & key)
		{
			static const ShaderDefine<&Keys::Invisible, &SimpleComputePSO::compute> Invisible = "INVISIBLE";
			SimplePSO mpso("GatherMeshes");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.compute.file_name = "shaders/gather_pipeline.hlsl";
			mpso.compute.entry_point = "CS_meshes_from_boxes";
			mpso.compute.flags = 0;
			Invisible.Apply(mpso, key);

			return mpso;
		}
	};
}
