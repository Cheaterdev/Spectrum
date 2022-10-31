#pragma once
namespace PSOS
{
	struct CopyTexture: public PSOBase
	{
		struct Keys {
KeyValue<Underlying<HAL::Format>,NonNullable,ALL_RT_FORMATS> Format;

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(CopyTexture,Format)
		GEN_KEY(Format,true);

		SimplePSO init_pso(Keys & key)
		{
			static const ShaderDefine<&Keys::Format> Format = "Format";

			SimplePSO mpso("CopyTexture");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/copy_texture.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/copy_texture.hlsl";
			mpso.pixel.entry_point = "PS";
			mpso.pixel.flags = 0;
			Format.Apply(mpso, key);

			mpso.rtv_formats = { Format.get_value(mpso, key) };
			mpso.blend = {  };
			mpso.enable_depth  = false;
			return mpso;
		}
	};
}
