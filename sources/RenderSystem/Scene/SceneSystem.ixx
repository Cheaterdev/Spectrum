export module Graphics:SceneSystem;

import <RenderSystem.h>;
import :Context;
import :MeshRenderer;
import :FrameGraphContext;
import FrameGraph;
import HAL;


export class SceneSystem
{
	std::shared_ptr<Graphics::OVRContext> vr_context;

public:
	template<typename TPipeline>
	explicit SceneSystem(TPipeline& pipeline, std::shared_ptr<Graphics::OVRContext> vr)
		: vr_context(std::move(vr))
	{
		pipeline.scene.setup_func = [](auto& data, FrameGraph::TaskBuilder& builder) -> bool
		{
			auto& frame = builder.graph->get_context<ViewportInfo>();
			auto  size  = frame.frame_size;

			builder.need(data.scene, FrameGraph::ResourceFlags::PixelRead);

			GBufferViewDesc::create(size, data.gbuffer, builder);
			GBufferViewDesc::create_mips(size, data.gbuffer, builder);
			GBufferViewDesc::create_quality(size, data.gbuffer, builder);

			builder.create(data.gbuffer.GBuffer_HiZ,
			               { ivec3(size / 8, 1), HAL::Format::R32_TYPELESS, 1 },
			               FrameGraph::ResourceFlags::DepthStencil);
			builder.create(data.gbuffer.GBuffer_HiZ_UAV,
			               { ivec3(size / 8, 1), HAL::Format::R32_FLOAT, 1 },
			               FrameGraph::ResourceFlags::UnorderedAccess);

			builder.create(data.gbuffer.GBuffer_NormalsPrev,
			               { ivec3(size, 0), HAL::Format::R8G8B8A8_UNORM, 1, 1 },
			               FrameGraph::ResourceFlags::Static);
			builder.create(data.gbuffer.GBuffer_SpecularPrev,
			               { ivec3(size, 0), HAL::Format::R8G8B8A8_UNORM, 1, 1 },
			               FrameGraph::ResourceFlags::Static);

			return true;
		};

		pipeline.scene.render_func = [this](auto& data, FrameGraph::FrameContext& context)
		{
			auto& scene_ctx    = context.graph->get_context<SceneInfo>();
			auto& camera_ctx   = context.graph->get_context<CameraInfo>();
			auto& time_ctx     = context.graph->get_context<TimeInfo>();
			auto& command_list = context.get_list();

			MeshRenderContext::ptr ctx(new MeshRenderContext());
			ctx->current_time = static_cast<size_t>(time_ctx.time);
			ctx->priority     = TaskPriority::HIGH;
			ctx->list         = command_list;
			ctx->eye_context  = vr_context;
			ctx->cam          = camera_ctx.cam;

			command_list->get_graphics().set_signature(Layouts::DefaultLayout);
			command_list->get_compute().set_signature(Layouts::DefaultLayout);

			GBuffer gbuffer = GBufferViewDesc::actualize(data.gbuffer);
			gbuffer.HalfBuffer.hiZ_depth     = *(data.gbuffer.GBuffer_HiZ);
			gbuffer.HalfBuffer.hiZ_depth_uav = *(data.gbuffer.GBuffer_HiZ_UAV);

			command_list->clear_uav(gbuffer.depth_mips.rwTexture2D, vec4(0, 0, 0, 0));

			{
				RT::GBuffer rtv;
				rtv.GetAlbedo()   = gbuffer.albedo.renderTarget;
				rtv.GetNormals()  = gbuffer.normals.renderTarget;
				rtv.GetSpecular() = gbuffer.specular.renderTarget;
				rtv.GetMotion()   = gbuffer.speed.renderTarget;
				rtv.GetDepth()    = gbuffer.depth.depthStencil;
				gbuffer.compiled  = rtv.compile(*command_list);
			}

			{
				RT::DepthOnly rtv;
				rtv.GetDepth()              = gbuffer.HalfBuffer.hiZ_depth.depthStencil;
				gbuffer.HalfBuffer.compiled = rtv.compile(*command_list);
			}

			ctx->g_buffer = &gbuffer;

			command_list->get_graphics().set_rtv(
				gbuffer.compiled, HAL::RTOptions::Default | HAL::RTOptions::ClearAll);

			context.graph->set_slot(SlotID::FrameInfo, command_list->get_graphics());
			context.graph->set_slot(SlotID::FrameInfo, command_list->get_compute());

			scene_ctx.renderer->render(ctx, scene_ctx.scene);

			command_list->get_copy().copy_texture(
				gbuffer.depth_mips.resource->get_ptr(), 0,
				gbuffer.depth.resource->get_ptr(), 0);
		};
	}
};
