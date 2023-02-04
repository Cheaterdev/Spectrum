#pragma once
namespace PSOS
{
	struct CanvasLines: public PSOBase
	{
		struct Keys {
			
			GEN_DEF_COMP(Keys);
			private:
			SERIALIZE()
			{
			}
		 };
		GEN_GRAPHICS_PSO(CanvasLines)
		
		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{
			
			SimplePSO mpso("CanvasLines");
			if(f) f(mpso,key);
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/gui/flow_line.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = HAL::ShaderOptions::None;
			mpso.pixel.file_name = "shaders/gui/flow_line.hlsl";
			mpso.pixel.entry_point = "PS";
			mpso.pixel.flags = HAL::ShaderOptions::None;
			mpso.geometry.file_name = "shaders/gui/flow_line.hlsl";
			mpso.geometry.entry_point = "GS";
			mpso.geometry.flags = HAL::ShaderOptions::None;
			mpso.domain.file_name = "shaders/gui/flow_line.hlsl";
			mpso.domain.entry_point = "DS";
			mpso.domain.flags = HAL::ShaderOptions::None;
			mpso.hull.file_name = "shaders/gui/flow_line.hlsl";
			mpso.hull.entry_point = "HS";
			mpso.hull.flags = HAL::ShaderOptions::None;
			
			mpso.rtv_formats = { HAL::Format::R8G8B8A8_UNORM };
			mpso.blend = { HAL::Blends::AlphaBlend };
			mpso.enable_depth  = false;
			mpso.cull  = HAL::CullMode::None;
			mpso.topology  = HAL::PrimitiveTopologyType::PATCH;
			return mpso;
		}
		private:
		SERIALIZE()
		{
			ar&NVP(wrap(psos));
		}
	};
}
