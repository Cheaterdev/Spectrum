#pragma once
namespace PSOS
{
	struct CanvasLines: public PSOBase
	{
		struct Keys {

 		GEN_DEF_COMP(Keys) };
		GEN_GRAPHICS_PSO(CanvasLines)
		
		SimplePSO init_pso(Keys & key)
		{
			
			SimplePSO mpso("CanvasLines");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex.file_name = "shaders/gui/flow_line.hlsl";
			mpso.vertex.entry_point = "VS";
			mpso.vertex.flags = 0;
			mpso.pixel.file_name = "shaders/gui/flow_line.hlsl";
			mpso.pixel.entry_point = "PS";
			mpso.pixel.flags = 0;
			mpso.geometry.file_name = "shaders/gui/flow_line.hlsl";
			mpso.geometry.entry_point = "GS";
			mpso.geometry.flags = 0;
			mpso.domain.file_name = "shaders/gui/flow_line.hlsl";
			mpso.domain.entry_point = "DS";
			mpso.domain.flags = 0;
			mpso.hull.file_name = "shaders/gui/flow_line.hlsl";
			mpso.hull.entry_point = "HS";
			mpso.hull.flags = 0;
			
			mpso.rtv_formats = { Graphics::Format::R8G8B8A8_UNORM };
			mpso.blend = { Graphics::Blends::AlphaBlend };
			mpso.enable_depth  = false;
			mpso.cull  = Graphics::CullMode::None;
			mpso.topology  = D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
			return mpso;
		}
	};
}
