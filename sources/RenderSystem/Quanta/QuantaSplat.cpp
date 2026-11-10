module Graphics:QuantaSplat;

import :QuantaSplat;

import RenderSystem;
import Graphics;
import HAL;
import Core;

#include "../FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;
using namespace HAL;

namespace
{
	// No owning Quanta object exists yet, so a free-standing context, as DDGIGraph.cpp does for its toggles.
	VariableContext& quanta_context()
	{
		static auto ctx = VariableContext::create(L"Quanta");
		return *ctx;
	}

	Variable<bool> g_quanta_splat = { false, "Quanta splat", &quanta_context() };
	Variable<bool> g_quanta_draw = { false, "Quanta draw", &quanta_context() };
	Variable<float> g_shadow_lod_pixels = { 1.0f, "Shadow LOD error (px)", &quanta_context(), 0.25f, 16.0f };
	Variable<bool> g_rt_proxy = { true, "RT from Quanta cut", &quanta_context() };
	Variable<int> g_rt_triangles = { 100000, "RT triangle budget", &quanta_context(), 1000, 4000000 };
}

namespace Quanta
{
	bool splat_enabled()
	{
		return g_quanta_splat;
	}

	void set_splat_enabled(bool enabled)
	{
		g_quanta_splat = enabled;
	}

	bool draw_enabled()
	{
		return g_quanta_draw;
	}

	void set_draw_enabled(bool enabled)
	{
		g_quanta_draw = enabled;
	}

	float shadow_lod_pixels()
	{
		return g_shadow_lod_pixels;
	}

	bool rt_proxy_enabled()
	{
		return g_rt_proxy;
	}

	uint32_t rt_triangle_budget()
	{
		return static_cast<uint32_t>(g_rt_triangles);
	}
}

FrameGraph::SetupResult PassDefault<Passes::Dev::QuantaSplat>::setup(
	Passes::Dev::QuantaSplat::Context& data, FrameGraph::TaskBuilder& builder)
{
	if (!Quanta::splat_enabled())
		return FrameGraph::SetupResult::Disabled;
	return FrameGraph::SetupResult::NeedsRender;
}

void PassDefault<Passes::Dev::QuantaSplat>::render(
	Passes::Dev::QuantaSplat::Context& data, FrameContext& context)
{
	auto& scene = context.graph->get_context<SceneInfo>().scene;
	if (!scene)
		return;

	auto& residency = Quanta::Residency::get();

	{
		PROFILE(L"quanta_splat_upload");
		for (MeshAssetInstance* instance : scene->mesh_objects)
			for (auto& quanta : instance->mesh_asset->quanta)
				if (quanta)
					residency.pin(quanta);

		auto list = context.get_list();
		residency.prepare(list);
	}

	auto& compute = context.get_list()->get_compute();
	compute.set_signature(Layouts::DefaultLayout);
	context.graph->set_slot(SlotID::FrameInfo, compute);
	compute.set_pipeline<PSOS::Dev::QuantaSplat>();

	{
		PROFILE(L"quanta_splat_dispatch");
		for (MeshAssetInstance* instance : scene->mesh_objects)
		{
			auto& asset_quanta = instance->mesh_asset->quanta;

			for (size_t i = 0; i < instance->rendering.size(); i++)
			{
				const int mesh_id = instance->nodes[i].asset_node->mesh_id;
				if (mesh_id < 0 || size_t(mesh_id) >= asset_quanta.size() || !asset_quanta[mesh_id])
					continue;

				auto resident = residency.pin(asset_quanta[mesh_id]);

				Slots::Dev::QuantaSplatData params;
				params.GetPages() = residency.pages.buffer.structuredBuffer;
				params.GetGroups() = residency.groups.buffer.structuredBuffer;
				params.GetNodes() = residency.nodes.buffer.structuredBuffer;
				params.GetPage_slots() = residency.page_slots.buffer.structuredBuffer;
				params.GetGroup_offset() = static_cast<uint>(resident->group_offset());
				params.GetDepth() = data.GBuffer_DepthMips->texture2D;
				params.GetTarget() = data.ResultTexture->rwTexture2D;
				params.GetWorld() = instance->rendering[i].global_mat;
				params.GetNode_offset() = static_cast<uint>(resident->node_offset());
				params.GetNode_count() = static_cast<uint>(resident->mesh->nodes.size());
				compute.set(params);

				compute.dispatch(uint3(resident->mesh->nodes.size() * 32, 1, 1), uint3(32, 1, 1));
			}
		}
	}
}
