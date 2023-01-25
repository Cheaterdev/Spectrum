#pragma once
namespace PSOS
{
	struct VoxelDownsample: public PSOBase
	{
		struct Keys {
			KeyValue<int,NonNullable,1,2,3> Count;

			GEN_DEF_COMP(Keys);
			private:
			SERIALIZE()
			{
				ar&NVP(Count);
			}
		 };
		GEN_COMPUTE_PSO(VoxelDownsample,Count)
		GEN_KEY(Count,true);

		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{
			static const ShaderDefine<&Keys::Count, &SimpleComputePSO::compute> Count = "COUNT";

			SimplePSO mpso("VoxelDownsample");
			if(f) f(mpso,key);
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.compute.file_name = "shaders/voxel_mipmap.hlsl";
			mpso.compute.entry_point = "CS";
			mpso.compute.flags = 0;
			Count.Apply(mpso, key);

			return mpso;
		}
		private:
		SERIALIZE()
		{
			ar&NVP(wrap(psos));
		}
	};
}
