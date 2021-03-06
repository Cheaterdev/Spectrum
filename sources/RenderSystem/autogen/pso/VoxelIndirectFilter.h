#pragma once
namespace PSOS
{
	struct VoxelIndirectFilter: public PSOBase
	{
		struct Keys {
KeyValue<NoValue,Nullable> Blur;
KeyValue<NoValue,Nullable> Reflection;

 		GEN_DEF_COMP(Keys) };
		GEN_COMPUTE_PSO(VoxelIndirectFilter,Blur,Reflection)
		GEN_KEY(Blur,true);
GEN_KEY(Reflection,true);

		SimplePSO init_pso(Keys & key)
		{
			static const ShaderDefine<&Keys::Blur, &SimpleComputePSO::compute> Blur = "ENABLE_BLUR";
static const ShaderDefine<&Keys::Reflection, &SimpleComputePSO::compute> Reflection = "REFLECTION";

			SimplePSO mpso("VoxelIndirectFilter");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.compute.file_name = "shaders/voxel_screen_blur.hlsl";
			mpso.compute.entry_point = "PS";
			mpso.compute.flags = 0;
			Blur.Apply(mpso, key);
Reflection.Apply(mpso, key);

			return mpso;
		}
	};
}
