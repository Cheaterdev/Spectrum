#pragma once
namespace PSOS
{
	struct SkyCube: public PSOBase
	{
		struct Keys {
			
			GEN_DEF_COMP(Keys);
			private:
			SERIALIZE()
			{
			}
		 };
		GEN_GRAPHICS_PSO(SkyCube)
		
		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{
			
			SimplePSO mpso("SkyCube");
			if(f) f(mpso,key);
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/sky.hlsl";
			mpso.vertex.entry_point = "VS_Cube";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/sky.hlsl";
			mpso.pixel.entry_point = "PS_Cube";
			mpso.pixel.flags = 0;
			
			mpso.rtv_formats = { HAL::Format::R11G11B10_FLOAT };
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
