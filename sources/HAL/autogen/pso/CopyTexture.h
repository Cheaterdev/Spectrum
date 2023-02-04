#pragma once
namespace PSOS
{
	struct CopyTexture: public PSOBase
	{
		struct Keys {
			KeyValue<Underlying<HAL::Format>,NonNullable,ALL_RT_FORMATS> Format;

			GEN_DEF_COMP(Keys);
			private:
			SERIALIZE()
			{
				ar&NVP(Format);
			}
		 };
		GEN_GRAPHICS_PSO(CopyTexture,Format)
		GEN_KEY(Format,true);

		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{
			static const ShaderDefine<&Keys::Format, &SimpleGraphicsPSO::pixel> Format = "Format";

			SimplePSO mpso("CopyTexture");
			if(f) f(mpso,key);
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/copy_texture.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = HAL::ShaderOptions::None;
			mpso.pixel.file_name = "shaders/copy_texture.hlsl";
			mpso.pixel.entry_point = "PS";
			mpso.pixel.flags = HAL::ShaderOptions::None;
			Format.Apply(mpso, key);

			mpso.rtv_formats = { Format.get_value(mpso, key) };
			mpso.blend = {  };
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
