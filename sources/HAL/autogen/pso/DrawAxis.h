#pragma once
namespace PSOS
{
	struct DrawAxis: public PSOBase
	{
		struct Keys {
			
			GEN_DEF_COMP(Keys);
			private:
			SERIALIZE()
			{
			}
		 };
		GEN_GRAPHICS_PSO(DrawAxis)
		
		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{
			
			SimplePSO mpso("DrawAxis");
			if(f) f(mpso,key);
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.mesh.file_name = "shaders/mesh_shader.hlsl";
			mpso.mesh.entry_point = "VS";
			mpso.mesh.flags = HAL::ShaderOptions::None;
			mpso.amplification.file_name = "shaders/mesh_shader.hlsl";
			mpso.amplification.entry_point = "AS";
			mpso.amplification.flags = HAL::ShaderOptions::None;
			mpso.pixel.file_name = "shaders/stencil.hlsl";
			mpso.pixel.entry_point = "PS_COLOR";
			mpso.pixel.flags = HAL::ShaderOptions::None;
			
			mpso.rtv_formats = { HAL::Format::R16G16B16A16_FLOAT };
			mpso.blend = {  };
			mpso.enable_depth  = false;
			mpso.cull  = HAL::CullMode::None;
			return mpso;
		}
		private:
		SERIALIZE()
		{
			ar&NVP(wrap(psos));
		}
	};
}
