#pragma once
namespace PSOS
{
	struct GatherMeshes: public PSOBase
	{
		struct Keys {
			KeyValue<NoValue,Nullable> Invisible;

			GEN_DEF_COMP(Keys);
			private:
			SERIALIZE()
			{
				ar&NVP(Invisible);
			}
		 };
		GEN_COMPUTE_PSO(GatherMeshes,Invisible)
		GEN_KEY(Invisible,true);

		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{
			static const ShaderDefine<&Keys::Invisible, &SimpleComputePSO::compute> Invisible = "INVISIBLE";

			SimplePSO mpso("GatherMeshes");
			if(f) f(mpso,key);
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.compute.file_name = "shaders/gather_pipeline.hlsl";
			mpso.compute.entry_point = "CS_meshes_from_boxes";
			mpso.compute.flags = 0;
			Invisible.Apply(mpso, key);

			return mpso;
		}
		private:
		SERIALIZE()
		{
			ar&NVP(wrap(psos));
		}
	};
}
