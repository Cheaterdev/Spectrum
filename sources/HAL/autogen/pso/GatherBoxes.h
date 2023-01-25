#pragma once
namespace PSOS
{
	struct GatherBoxes: public PSOBase
	{
		struct Keys {
			KeyValue<NoValue,Nullable> CheckFrustum;

			GEN_DEF_COMP(Keys);
			private:
			SERIALIZE()
			{
				ar&NVP(CheckFrustum);
			}
		 };
		GEN_COMPUTE_PSO(GatherBoxes,CheckFrustum)
		GEN_KEY(CheckFrustum,true);

		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{
			static const ShaderDefine<&Keys::CheckFrustum, &SimpleComputePSO::compute> CheckFrustum = "CHECK_FRUSTUM";

			SimplePSO mpso("GatherBoxes");
			if(f) f(mpso,key);
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.compute.file_name = "shaders/gather_pipeline.hlsl";
			mpso.compute.entry_point = "CS_boxes";
			mpso.compute.flags = 0;
			CheckFrustum.Apply(mpso, key);

			return mpso;
		}
		private:
		SERIALIZE()
		{
			ar&NVP(wrap(psos));
		}
	};
}
