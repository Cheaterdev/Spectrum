#pragma once
namespace PSOS
{
	struct FontRender: public PSOBase
	{
		struct Keys {
			
			GEN_DEF_COMP(Keys);
			private:
			SERIALIZE()
			{
			}
		 };
		GEN_GRAPHICS_PSO(FontRender)
		
		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{
			
			SimplePSO mpso("FontRender");
			if(f) f(mpso,key);
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/font/vsSimple.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/font/psSimple.hlsl";
			mpso.pixel.entry_point = "PS";
			mpso.pixel.flags = 0;
			mpso.geometry.file_name = "shaders/font/gsSimple.hlsl";
			mpso.geometry.entry_point = "GS";
			mpso.geometry.flags = 0;
			
			mpso.rtv_formats = { HAL::Format::R8G8B8A8_UNORM };
			mpso.blend = {  };
			mpso.topology  = HAL::PrimitiveTopologyType::POINT;
			mpso.enable_depth  = false;
			return mpso;
		}
		private:
		SERIALIZE()
		{
			ar&NVP(wrap(psos));
		}
	};
}
