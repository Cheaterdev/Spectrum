#pragma once
namespace PSOS
{
	struct BlendWeight: public PSOBase
	{
		struct Keys {
			
			GEN_DEF_COMP(Keys);
			private:
			SERIALIZE()
			{
			}
		 };
		GEN_GRAPHICS_PSO(BlendWeight)
		
		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{
			
			SimplePSO mpso("BlendWeight");
			if(f) f(mpso,key);
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/SMAA.hlsl";
			mpso.vertex.entry_point = "DX10_SMAABlendingWeightCalculationVS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/SMAA.hlsl";
			mpso.pixel.entry_point = "DX10_SMAABlendingWeightCalculationPS";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { HAL::Format::R8G8B8A8_UNORM };
			mpso.blend = {  };
			return mpso;
		}
		private:
		SERIALIZE()
		{
			ar&NVP(wrap(psos));
		}
	};
}
