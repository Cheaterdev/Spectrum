#pragma once
namespace PSOS
{
	struct DepthDraw: public PSOBase
	{
		struct Keys {
			
			GEN_DEF_COMP(Keys);
			private:
			SERIALIZE()
			{
			}
		 };
		GEN_GRAPHICS_PSO(DepthDraw)
		
		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{
			
			SimplePSO mpso("DepthDraw");
			if(f) f(mpso,key);
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.pixel.file_name = "";
			mpso.pixel.entry_point = "";
			mpso.pixel.flags = 0;
			mpso.mesh.file_name = "shaders/mesh_shader.hlsl";
			mpso.mesh.entry_point = "VS";
			mpso.mesh.flags = 0;
			mpso.amplification.file_name = "shaders/mesh_shader.hlsl";
			mpso.amplification.entry_point = "AS";
			mpso.amplification.flags = 0;
			
			mpso.rtv_formats = {  };
			mpso.blend = {  };
			mpso.ds  = HAL::Format::D32_FLOAT;
			mpso.cull  = HAL::CullMode::Front;
			return mpso;
		}
		private:
		SERIALIZE()
		{
			ar&NVP(wrap(psos));
		}
	};
}
