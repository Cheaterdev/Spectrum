#pragma once
namespace PSOS
{
	struct MipMapping: public PSOBase
	{
		struct Keys {
			KeyValue<int,NonNullable,0,1,2,3> NonPowerOfTwo;
KeyValue<NoValue,Nullable> Gamma;

			GEN_DEF_COMP(Keys);
			private:
			SERIALIZE()
			{
				ar&NVP(NonPowerOfTwo);
				ar&NVP(Gamma);
			}
		 };
		GEN_COMPUTE_PSO(MipMapping,NonPowerOfTwo,Gamma)
		GEN_KEY(NonPowerOfTwo,true);
GEN_KEY(Gamma,true);

		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{
			static const ShaderDefine<&Keys::NonPowerOfTwo, &SimpleComputePSO::compute> NonPowerOfTwo = "NON_POWER_OF_TWO";
static const ShaderDefine<&Keys::Gamma, &SimpleComputePSO::compute> Gamma = "CONVERT_TO_SRGB";

			SimplePSO mpso("MipMapping");
			if(f) f(mpso,key);
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.compute.file_name = "shaders/GenerateMips.hlsl";
			mpso.compute.entry_point = "CS";
			mpso.compute.flags = 0;
			NonPowerOfTwo.Apply(mpso, key);
Gamma.Apply(mpso, key);

			return mpso;
		}
		private:
		SERIALIZE()
		{
			ar&NVP(wrap(psos));
		}
	};
}
