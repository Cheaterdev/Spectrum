export module Graphics:Sky;

import <RenderSystem.h>;
import :FrameGraphContext;
import FrameGraph;
import HAL;


export class SkyRender
{
	HAL::Texture::ptr transmittance;
	HAL::Texture::ptr irradiance;
	HAL::Texture::ptr inscatter;

	float3 dir = float3(0, 0, -1);

public:
	using ptr = std::shared_ptr<SkyRender>;

	// Default constructor: loads sky lookup textures from disk.
	// Defined in Sky.cpp so the EngineAsset declarations stay out of the interface.
	SkyRender();

	// Template constructor: wires CubeSky and Sky pass funcs onto any pipeline
	// that exposes matching pass members.  Delegates to SkyRender() first so
	// the lookup textures are always ready.
	template<typename TPipeline>
	explicit SkyRender(TPipeline& pipeline) : SkyRender()
	{
		// ---- CubeSky -------------------------------------------------------
		// Renders the atmospheric sky into a static cubemap, but only when the
		// sun direction has changed enough to warrant a re-bake.

		pipeline.cubeSky.setup_func = [this](auto& data, FrameGraph::TaskBuilder& builder) -> bool
		{
			auto& sky = builder.graph->get_context<SkyInfo>();

			builder.create(data.sky_cubemap,
			               { ivec3(256, 256, 0), HAL::Format::R11G11B10_FLOAT, 1, 0 },
			               FrameGraph::ResourceFlags::UnorderedAccess |
			               FrameGraph::ResourceFlags::RenderTarget    |
			               FrameGraph::ResourceFlags::Static);

			bool changed = ((sky.sunDir - dir).length() > 0.001f);
			if (changed)
			{
				data.sky_cubemap.changed();
				dir = sky.sunDir;
			}
			return changed;
		};

		pipeline.cubeSky.render_func = [this](auto& data, FrameGraph::FrameContext& context)
		{
			auto& sky      = context.graph->get_context<SkyInfo>();
			auto& graphics = context.get_list()->get_graphics();

			graphics.set_pipeline<PSOS::SkyCube>();
			graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE,
			                      HAL::PrimitiveTopologyFeed::STRIP);

			{
				Slots::SkyData skydata;
				skydata.GetInscatter()    = inscatter->texture_3d().texture3D;
				skydata.GetIrradiance()   = irradiance->texture_2d().texture2D;
				skydata.GetTransmittance()= transmittance->texture_2d().texture2D;
				skydata.GetSunDir()       = sky.sunDir;
				graphics.set(skydata);
			}

			context.graph->set_slot(SlotID::FrameInfo, graphics);

			for (unsigned int i = 0; i < 6; i++)
			{
				HAL::TextureViewDesc subres;
				subres.ArraySize       = 1;
				subres.FirstArraySlice = i;
				subres.MipLevels       = 1;
				subres.MipSlice        = 0;

				auto face = data.sky_cubemap->resource->create_view<HAL::Texture2DView>(
					graphics.get_base(), subres);

				Slots::SkyFace skyFace;
				skyFace.GetFace() = i;
				graphics.set(skyFace);

				{
					RT::SingleColor rt;
					rt.GetColor() = face.renderTarget;
					graphics.set_rtv(rt);
				}

				graphics.draw(4);
			}
		};

		// ---- Sky -----------------------------------------------------------
		// Full-screen sky pass that composites over the GBuffer depth.

		pipeline.sky.setup_func = [](auto& data, FrameGraph::TaskBuilder& builder) -> bool
		{
			builder.need(data.GBuffer_Depth,  FrameGraph::ResourceFlags::PixelRead);
			builder.need(data.ResultTexture,  FrameGraph::ResourceFlags::RenderTarget);
			return true;
		};

		pipeline.sky.render_func = [this](auto& data, FrameGraph::FrameContext& context)
		{
			auto& sky      = context.graph->get_context<SkyInfo>();
			auto& graphics = context.get_list()->get_graphics();

			graphics.set_pipeline<PSOS::Sky>();
			graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE,
			                      HAL::PrimitiveTopologyFeed::STRIP);

			{
				RT::SingleColor rt;
				rt.GetColor() = data.ResultTexture->renderTarget;
				graphics.set_rtv(rt);
			}

			{
				Slots::SkyData skydata;
				skydata.GetInscatter()    = inscatter->texture_3d().texture3D;
				skydata.GetIrradiance()   = irradiance->texture_2d().texture2D;
				skydata.GetTransmittance()= transmittance->texture_2d().texture2D;
				skydata.GetDepthBuffer()  = data.GBuffer_Depth->texture2D;
				skydata.GetSunDir()       = sky.sunDir;
				graphics.set(skydata);
			}

			context.graph->set_slot(SlotID::FrameInfo, graphics);

			graphics.draw(4);
		};
	}
};
