#pragma once
namespace PSOS
{
	struct CubemapENV: public PSOBase
	{
		struct Keys {
			KeyValue<int,NonNullable,1,8,32,64,128> Level;

			GEN_DEF_COMP(Keys);
			private:
			SERIALIZE()
			{
				ar&NVP(Level);
			}
		 };
		GEN_GRAPHICS_PSO(CubemapENV,Level)
		GEN_KEY(Level,false);

		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{
			static const ShaderDefine<&Keys::Level, &SimpleGraphicsPSO::pixel> Level = "NumSamples";

			SimplePSO mpso("CubemapENV");
			if(f) f(mpso,key);
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/cubemap_down.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/cubemap_down.hlsl";
			mpso.pixel.entry_point = "PS";
			mpso.pixel.flags = 0;
			Level.Apply(mpso, key);

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
