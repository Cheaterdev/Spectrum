#pragma once
namespace PSOS
{
	struct Blending: public PSOBase
	{
		struct Keys {
			
			GEN_DEF_COMP(Keys);
			private:
			SERIALIZE()
			{
			}
		 };
		GEN_GRAPHICS_PSO(Blending)
		
		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{
			
			SimplePSO mpso("Blending");
			if(f) f(mpso,key);
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/SMAA.hlsl";
			mpso.vertex.entry_point = "DX10_SMAANeighborhoodBlendingVS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/SMAA.hlsl";
			mpso.pixel.entry_point = "DX10_SMAANeighborhoodBlendingPS";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { HAL::Format::R16G16B16A16_FLOAT };
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
