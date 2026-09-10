module Graphics:SceneSystem;


import :Context;
import :MeshRenderer;
import :FrameGraphContext;
import FrameGraph;
import HAL;

using namespace FrameGraph;

bool PassDefault<Passes::Scene>::setup(
    Passes::Scene::Context& data, FrameGraph::TaskBuilder& builder)
{
    auto& frame = builder.graph->get_context<ViewportInfo>();
    auto  size  = frame.frame_size;

    // *Prev history links (registered before the current resources are
    // created, so they get tagged is_history_current and auto-provision
    // their *Prev) -- inlined from the former GBufferViewDesc::create()/
    // create_quality() (Context.ixx, removed once GBuffer stopped being a
    // View; see pssm.sig's own comment) since link_history()/
    // bind_history_prev() have no [Always]/[Size] SIG equivalent to
    // auto-generate this from.
    builder.link_history(data.GBuffer_Normals.id,   data.GBuffer_NormalsPrev.id);
    builder.link_history(data.GBuffer_DepthMips.id, data.GBuffer_DepthPrev.id);

    builder.create(data.GBuffer_Albedo,   { ivec3(size, 0), HAL::Format::R8G8B8A8_UNORM, 1, 1 }, ResourceFlags::RenderTarget);
    builder.create(data.GBuffer_Normals,  { ivec3(size, 0), HAL::Format::R8G8B8A8_UNORM, 1, 1 }, ResourceFlags::RenderTarget | ResourceFlags::UnorderedAccess);
    builder.create(data.GBuffer_Depth,    { ivec3(size, 0), HAL::Format::R32_TYPELESS,   1, 1 }, ResourceFlags::DepthStencil);
    builder.create(data.GBuffer_Specular, { ivec3(size, 0), HAL::Format::R8G8B8A8_UNORM, 1, 1 }, ResourceFlags::RenderTarget);
    builder.create(data.GBuffer_Speed,    { ivec3(size, 0), HAL::Format::R16G16_FLOAT,   1, 1 }, ResourceFlags::RenderTarget);

    // GBuffer_DepthPrev is the previous-frame view of GBuffer_DepthMips -- the
    // history link above makes create() below provision and carry it, so it is
    // not created here.
    builder.create(data.GBuffer_DepthMips, { ivec3(size, 0), HAL::Format::R32_TYPELESS, 1, 1 }, ResourceFlags::UnorderedAccess | ResourceFlags::RenderTarget);

    // The *Prev resources are provisioned (chain-only) by the current creates
    // above; bind this context's local handles to them so actualize() can
    // dereference them (GBuffer::depth_prev_mips = *data.GBuffer_DepthPrev).
    builder.bind_history_prev(data.GBuffer_NormalsPrev);
    builder.bind_history_prev(data.GBuffer_DepthPrev);

    builder.create(data.GBuffer_Quality, { ivec3(size, 0), HAL::Format::D24_UNORM_S8_UINT, 1, 1 }, ResourceFlags::DepthStencil);

    return true;
}

void PassDefault<Passes::Scene>::render(
    Passes::Scene::Context& data, FrameGraph::FrameContext& context)
{
    auto& scene_ctx    = context.graph->get_context<SceneInfo>();
    auto& camera_ctx   = context.graph->get_context<CameraInfo>();
    auto& time_ctx     = context.graph->get_context<TimeInfo>();
    auto& command_list = context.get_list();

    MeshRenderContext::ptr ctx(new MeshRenderContext());
    ctx->current_time = static_cast<size_t>(time_ctx.time);
    ctx->priority     = TaskPriority::HIGH;
    ctx->list         = command_list;
    ctx->cam          = camera_ctx.cam;

    command_list->get_graphics().set_signature(Layouts::DefaultLayout);
    command_list->get_compute().set_signature(Layouts::DefaultLayout);

    GBuffer gbuffer = GBufferViewDesc::actualize(data);
    gbuffer.HalfBuffer.hiZ_depth     = *(data.GBuffer_HiZ);
    gbuffer.HalfBuffer.hiZ_depth_uav = *(data.GBuffer_HiZ_UAV);

    command_list->clear_uav(gbuffer.depth_mips.rwTexture2D, vec4(0, 0, 0, 0));

    // NormalsPrev / DepthPrev are history-linked and provisioned automatically —
    // not touched here. SpecularPrev history was unused (denoiser roughness-history
    // is disabled) so it's been removed along with the CopyPrev copies.

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

    // Fresh/resized HiZ allocation holds garbage — clear to far (0, reversed-Z)
    // so pass 1 of the occlusion culler can't falsely cull against it.
    if (data.GBuffer_HiZ.is_new())
        command_list->get_graphics().set_rtv(
            gbuffer.HalfBuffer.compiled, HAL::RTOptions::Default | HAL::RTOptions::ClearDepth);

    ctx->g_buffer = &gbuffer;

    command_list->get_graphics().set_rtv(
        gbuffer.compiled, HAL::RTOptions::Default | HAL::RTOptions::ClearAll);

    context.graph->set_slot(SlotID::FrameInfo, command_list->get_graphics());
    context.graph->set_slot(SlotID::FrameInfo, command_list->get_compute());

    scene_ctx.renderer->render(ctx, scene_ctx.scene);

    command_list->get_copy().copy_texture(
        gbuffer.depth_mips.resource->get_ptr(), 0,
        gbuffer.depth.resource->get_ptr(), 0);
}
