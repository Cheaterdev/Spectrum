#pragma once
namespace PSOS
{
	struct GBufferDraw: public PSOBase
	{
		struct Keys {
			
			GEN_DEF_COMP(Keys);
			private:
			SERIALIZE()
			{
			}
		 };
		GEN_GRAPHICS_PSO(GBufferDraw)
		
		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{
			
			SimplePSO mpso("GBufferDraw");
			if(f) f(mpso,key);
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.mesh.file_name = "shaders/mesh_shader.hlsl";
			mpso.mesh.entry_point = "VS";
			mpso.mesh.flags = HAL::ShaderOptions::None;
			mpso.amplification.file_name = "shaders/mesh_shader.hlsl";
			mpso.amplification.entry_point = "AS";
			mpso.amplification.flags = HAL::ShaderOptions::None;
			
			mpso.rtv_formats = { HAL::Format::R8G8B8A8_UNORM, HAL::Format::R8G8B8A8_UNORM, HAL::Format::R8G8B8A8_UNORM, HAL::Format::R16G16_FLOAT };
			mpso.blend = {  };
			mpso.ds  = HAL::Format::D32_FLOAT;
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
