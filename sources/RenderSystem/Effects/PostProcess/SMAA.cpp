module Graphics:SMAA;


import :FrameGraphContext;

import HAL;

#define STRINGIFY(x) #x
#define PPCAT_NX(A, B) A ## B

#define SMAA_TEXTURE(name) \
    EngineAsset<TextureAsset> name(PPCAT_NX(L,#name), [] { \
		return new TextureAsset(to_path(L"textures/" STRINGIFY(name) ".dds")); \
    })

namespace EngineAssets
{
	SMAA_TEXTURE(AreaTex);
	SMAA_TEXTURE(SearchTex);
}

SMAA::SMAA()
{
	area_tex   = EngineAssets::AreaTex.get_asset()->get_texture();
	search_tex = EngineAssets::SearchTex.get_asset()->get_texture();

	// ---- Pass function members ------------------------------------------------

	m_smaa_setup = [this](Passes::SMAA::Context& data, FrameGraph::TaskBuilder& builder) -> bool
	{
		auto& frame = builder.graph->get_context<ViewportInfo>();
		builder.need(data.ResultTexture, FrameGraph::ResourceFlags::ComputeRead);
		builder.create(data.SMAA_edges,
			{ ivec3(frame.frame_size, 0), HAL::Format::R8G8_UNORM, 1, 1 },
			FrameGraph::ResourceFlags::UnorderedAccess);
		builder.create(data.SMAA_blend,
			{ ivec3(frame.frame_size, 0), HAL::Format::R8G8B8A8_UNORM, 1, 1 },
			FrameGraph::ResourceFlags::UnorderedAccess);
		builder.recreate(data.ResultTextureNew, FrameGraph::ResourceFlags::UnorderedAccess);
		return true;
	};

	m_smaa_render = [this](Passes::SMAA::Context& data, FrameGraph::FrameContext& context)
	{
		auto& frame   = context.graph->get_context<ViewportInfo>();
		auto& compute = context.get_list()->get_compute();
		ivec2 size    = frame.frame_size;

		// SMAA_Global (Instance0) is bound once, like the original single
		// graphics.set(slot_global) before the first draw: SMAA_RT_METRICS is
		// referenced by all three shaders (SMAA_impl.hlsl reads it at global
		// scope), and the same root-signature slot persists across the pipeline
		// switches below since none of them rebind Instance0.
		compute.set_pipeline<PSOS::EdgeDetectCompute>();
		{
			Slots::SMAA_Global slot_global;
			slot_global.GetColorTex()         = data.ResultTexture->texture2D;
			slot_global.GetSubsampleIndices() = float4(0, 0, 0, 0);
			slot_global.GetSMAA_RT_METRICS()  = float4(1.0f / size.x, 1.0f / size.y, size);
			slot_global.GetEdgesOut()         = data.SMAA_edges->rwTexture2D;
			compute.set(slot_global);
		}
		compute.dispatch(size, ivec2{ 16, 16 });

		// Instance1 is reused between SMAA_Weights and SMAA_Blend, same as the
		// original code's two graphics.set() calls for the same slot.
		compute.set_pipeline<PSOS::BlendWeightCompute>();
		{
			Slots::SMAA_Weights slot_edges;
			slot_edges.GetSearchTex() = search_tex->texture_2d().texture2D;
			slot_edges.GetAreaTex()   = area_tex->texture_2d().texture2D;
			slot_edges.GetEdgesTex()  = data.SMAA_edges->texture2D;
			slot_edges.GetBlendOut()  = data.SMAA_blend->rwTexture2D;
			compute.set(slot_edges);
		}
		compute.dispatch(size, ivec2{ 16, 16 });

		compute.set_pipeline<PSOS::BlendingCompute>();
		{
			Slots::SMAA_Blend slot_blend;
			slot_blend.GetBlendTex()  = data.SMAA_blend->texture2D;
			slot_blend.GetResultOut() = data.ResultTextureNew->rwTexture2D;
			compute.set(slot_blend);
		}
		compute.dispatch(size, ivec2{ 16, 16 });
	};
}
