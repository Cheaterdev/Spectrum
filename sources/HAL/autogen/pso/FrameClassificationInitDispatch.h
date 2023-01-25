#pragma once
namespace PSOS
{
	struct FrameClassificationInitDispatch: public PSOBase
	{
		struct Keys {
			
			GEN_DEF_COMP(Keys);
			private:
			SERIALIZE()
			{
			}
		 };
		GEN_COMPUTE_PSO(FrameClassificationInitDispatch)
		
		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{
			
			SimplePSO mpso("FrameClassificationInitDispatch");
			if(f) f(mpso,key);
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.compute.file_name = "shaders/FrameClassificationInitDispatch.hlsl";
			mpso.compute.entry_point = "CS";
			mpso.compute.flags = 0;
			
			return mpso;
		}
		private:
		SERIALIZE()
		{
			ar&NVP(wrap(psos));
		}
	};
}
