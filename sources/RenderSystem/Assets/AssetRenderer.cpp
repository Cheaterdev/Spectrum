
module  Graphics:AssetRenderer;


import HAL;
import :MeshRenderer;
import :Sky;
import :PSSM;
import :FrameGraphContext;
import :MipMapGenerator;
import :EngineAssets;
import :BRDF;

using namespace FrameGraph;
using namespace HAL;

class SceneRenderWorkflow
{
    Pipelines::Editor::AssetPipeline pipeline;

    PSSM      pssm;
    SkyRender sky;

    Graph* m_graph = nullptr;
    ivec2  m_size  = {};

public:

    float time = 0;

    // FrameInfo::debugFlags for this graph's frame (Dev::RTXDebugFlags), set by the
    // preview before each render.
    uint32_t debug_flags = 0;

    main_renderer::ptr scene_renderer;

    SceneRenderWorkflow() : pipeline(), sky(pipeline), pssm(pipeline)
    {
        // Both setups are fully generated (AssetRenderer.prism's own [RunAlways]).
        // AssetMip goes through the pipeline now rather than a raw
        // add_library_pass in render(): the pipeline registers a pass only when
        // its render_func is set, so the wiring belongs next to assetGBuffer's.
        pipeline.assetMip.render_func = [](auto& data, FrameContext& _context) {
            MipMapGenerator::get().render_texture_2d_slow(_context.get_list()->get_graphics(), *data.swapchain, *data.ResultTexture);
            MipMapGenerator::get().generate(_context.get_list()->get_compute(), *data.swapchain);
        };

        // The baked sky as indirect light, diffuse half before the subsurface blur and
        // specular half after it (AssetRenderer.prism).
        auto sky_light = [](auto& data, FrameContext& context, bool specular)
        {
            GBuffer gbuffer = GBufferViewDesc::actualize(data);

            auto& compute = context.get_list()->get_compute();
            compute.set_signature(Layouts::DefaultLayout);
            context.graph->set_slot(SlotID::FrameInfo, compute);

            Slots::Shadows::VSM::SkyAmbientData ambient;
            gbuffer.SetTable(ambient.GetGbuffer());
            ambient.GetSky()    = data.sky_cubemap_filtered->textureCube;
            ambient.GetSky_diffuse() = data.sky_cubemap_filtered_diffuse->textureCube;
            ambient.GetResult() = data.ResultTexture->rwTexture2D;
            compute.set(ambient);

            if (specular)
                compute.set_pipeline<PSOS::Shadows::VSM::SkyAmbientSpecular>();
            else
                compute.set_pipeline<PSOS::Shadows::VSM::SkyAmbientDiffuse>();
            {
                PROFILE_GPU(L"asset_sky_light");
                compute.dispatch(context.graph->get_context<ViewportInfo>().frame_size, ivec2{ 16, 16 });
            }
        };

        pipeline.assetSkyDiffuse.render_func = [sky_light](auto& data, FrameContext& context) {
            sky_light(data, context, false);
        };

        pipeline.assetSkySpecular.render_func = [sky_light](auto& data, FrameContext& context) {
            sky_light(data, context, true);
        };

        pipeline.assetGBuffer.render_func = [this](auto& data, FrameContext& _context)
        {
            auto& command_list = _context.get_list();
            auto& frame = m_graph->get_context<ViewportInfo>();
            auto& scene = m_graph->get_context<SceneInfo>();
            auto& cam   = m_graph->get_context<CameraInfo>();

            command_list->get_graphics().set_signature(Layouts::DefaultLayout);
            command_list->get_compute().set_signature(Layouts::DefaultLayout);

            MeshRenderContext::ptr context(new MeshRenderContext());
            context->current_time = (size_t)time;
            context->priority     = TaskPriority::HIGH;
            context->list         = command_list;
            context->cam          = cam.cam;

            GBuffer gbuffer = GBufferViewDesc::actualize(data);
            gbuffer.HalfBuffer.hiZ_depth     = *data.GBuffer_HiZ;
            gbuffer.HalfBuffer.hiZ_depth_uav = *data.GBuffer_HiZ_UAV;

            {
                RT::Meshes::GBuffer rtv;
                rtv.GetAlbedo()   = gbuffer.albedo.renderTarget;
                rtv.GetNormals()  = gbuffer.normals.renderTarget;
                rtv.GetSpecular() = gbuffer.specular.renderTarget;
                rtv.GetMotion()   = gbuffer.speed.renderTarget;
                rtv.GetObject_Id() = gbuffer.object_id.renderTarget;
                rtv.GetMaterial() = gbuffer.material.renderTarget;
                rtv.GetDepth()    = gbuffer.depth.depthStencil;
                gbuffer.compiled  = rtv.compile(*command_list);
            }

            {
                RT::Frame::DepthOnly rtv;
                rtv.GetDepth()              = gbuffer.HalfBuffer.hiZ_depth.depthStencil;
                gbuffer.HalfBuffer.compiled = rtv.compile(*command_list);
            }

            // Fresh/resized HiZ holds garbage — clear to far (0, reversed-Z).
            if (data.GBuffer_HiZ.is_new())
                command_list->get_graphics().set_rtv(
                    gbuffer.HalfBuffer.compiled, RTOptions::Default | RTOptions::ClearDepth);

            context->g_buffer = &gbuffer;

            context->list->get_graphics().set_rtv(gbuffer.compiled, RTOptions::Default | RTOptions::ClearAll);

            m_graph->set_slot(SlotID::FrameInfo, command_list->get_graphics());
            m_graph->set_slot(SlotID::FrameInfo, command_list->get_compute());

            scene_renderer->render(context, scene.scene->get_ptr<Scene>());

            //FIX THIS!!
            {
                command_list->get_copy().copy_texture(
                    gbuffer.depth_mips.resource->get_ptr(), 0,
                    gbuffer.depth.resource->get_ptr(), 0);
            }
        };
    }

    void render(Graph& graph)
    {
        m_graph = &graph;
        m_size  = graph.get_context<ViewportInfo>().frame_size;

        auto& sceneinfo = graph.get_context<SceneInfo>();
        auto scene = sceneinfo.scene->get_ptr<Scene>().get();

        scene->update(*graph.builder.current_frame);

        // TranslucentRTX's generated [SetupCondition] reads this; left unset,
        // translucent materials would be invisible in previews.
        graph.get_context<Table::Raytrace::RenderDeviceCapabilities>().rtx_supported = RenderSystem::get().device().is_rtx_supported();

		pipeline.add_passes(graph);
        // This pipeline's own [PreSetup] hooks (CubeSky's sun-direction diff)
        // -- per-pipeline, so the main pipeline's PreScene/NRD hooks do not
        // fire while this graph is being built.
        pipeline.run_pre_setups(graph);


        graph.add_slot_generator([this](Graph& graph) {
            auto& time    = graph.get_context<TimeInfo>();
            auto& skyinfo = graph.get_context<SkyInfo>();
            auto& cam     = graph.get_context<CameraInfo>();
            PROFILE(L"FrameInfo");
            Slots::Frame::FrameInfo frameInfo;
            //// hack zone
            auto sky = graph.builder.get(FrameGraph::ResourceID::sky_cubemap_filtered);
            if (sky && sky->resource)
                frameInfo.GetSky() = sky->get_handler<Handlers::TextureCube>()->textureCube;

            auto sky_diffuse = graph.builder.get(FrameGraph::ResourceID::sky_cubemap_filtered_diffuse);
            if (sky_diffuse && sky_diffuse->resource)
                frameInfo.GetSky_diffuse() = sky_diffuse->get_handler<Handlers::TextureCube>()->textureCube;
            /////////
            frameInfo.GetSunDir().xyz = skyinfo.sunDir;
            frameInfo.GetDebugFlags() = debug_flags;
            frameInfo.GetTime() = { time.time, time.totalTime, 0, 0 };
            frameInfo.GetCamera()     = cam.cam->camera_cb.current;
            frameInfo.GetPrevCamera() = cam.cam->camera_cb.prev;
            frameInfo.GetBrdf()           = EngineAssets::brdf.get_asset()->get_texture()->texture_3d().texture3D;
            frameInfo.GetBestFitNormals() = EngineAssets::best_fit_normals.get_asset()->get_texture()->texture_2d().texture2D;
            auto compiled = frameInfo.compile(*graph.builder.current_frame);
            graph.register_slot_setter(compiled);
        });

        graph.add_slot_generator([this](Graph& graph) {
            auto& scene = graph.get_context<SceneInfo>();
            graph.register_slot_setter(scene.scene->compiledScene);
        });
    }

};



void AssetRenderer::draw(Scene::ptr scene, HAL::Texture::ptr result)
{

    graph.start_new_frame();


    scene->update_transforms();

    auto mn = scene->get_min();
    auto mx = scene->get_max();
    scene->add_child(mesh_plane);
    scene->update(*graph.builder.current_frame);

    float x = mx.x - mn.x;
    float y = mx.y - mn.y;


    mesh_plane->local_transform.scaling(std::max(x, y));

    mesh_plane->local_transform.a42 = mn.y;
    scene->update_transforms();


    auto& time    = graph.get_context<TimeInfo>();
    auto& skyinfo = graph.get_context<SkyInfo>();
    auto& caminfo = graph.get_context<CameraInfo>();
    auto& sceneinfo = graph.get_context<SceneInfo>();
    auto& vp = graph.get_context<ViewportInfo>();


    cam.target   = (mn + mx) / 2;
    cam.position = cam.target + (vec3(30, 10, 30).normalize()) * ((mx - mn).length());
    cam.set_projection_params(Math::pi / 4, 1, 1, 100 + (mn - mx).length());
    cam.update();


    skyinfo.sunDir = float3(1, 1, 1).normalize();

   graph.builder.debug = true;
    // Render 1:1 -- this is a small, one-shot thumbnail render (asset
    // browser preview), not a real-time viewport, so there's no perf reason
    // to render at half res and upscale (which just blurs the result).
    vp.frame_size   = result->get_size().xy;
    vp.upscale_size = result->get_size().xy;

    sceneinfo.scene    = scene;
    sceneinfo.renderer = scene_renderer;
    caminfo.cam        = &cam;

    rendering->render(graph);
	graph.builder.pass_texture(FrameGraph::ResourceID::swapchain, result->resource, {}, ResourceFlags::Required);

    graph.setup();
    graph.compile(frame++);
    graph.render();
    graph.commit_command_lists();
    graph.reset();
    mesh_plane->remove_from_parent();
}

namespace
{
    std::atomic<bool> preview_deferred = false;
}

void AssetRenderer::defer_preview()
{
    preview_deferred = true;
}

bool AssetRenderer::take_deferred()
{
    return preview_deferred.exchange(false);
}

void AssetRenderer::draw(MaterialAsset::ptr mat, HAL::Texture::ptr result)
{
    std::lock_guard<std::mutex> g(lock);

    scene->add_child(material_tester);
    material_tester->override_material(1, mat);

    draw(scene, result);
    material_tester->remove_from_parent();
}

void AssetRenderer::draw(scene_object::ptr obj, HAL::Texture::ptr result)
{
    std::lock_guard<std::mutex> g(lock);

    scene->add_child(obj);
    draw(scene, result);
    obj->remove_from_parent();
}

AssetRenderer::AssetRenderer()
{
    std::lock_guard<std::mutex> g(lock);

    // graph is a plain member, already fully constructed (default Graph()) by
    // the time this body runs -- same "too late for Scope" situation as
    // main.cpp's vsm, so it needs the same explicit re-parent rather than a
    // Scope wrapping its construction.
    preview_context->add_child(&graph);

    scene_renderer = std::make_shared<main_renderer>();
    {
        // Distinguishes this mesh_renderer's Properties-tree entry from the
        // main view's and SceneTextureRenderer's otherwise-identically-named ones.
        VariableContext::Scope scope(*preview_context);
        scene_renderer->register_renderer(meshes_renderer = std::make_shared<mesh_renderer>());
    }

    // Meshlet Hi-Z occlusion needs FrameInfo::mainHiZ, which only the main view
    // binds (see main.cpp). This renderer has no pyramid to point it at, so the
    // test would run against whatever descriptor 0 happens to hold -- that is
    // what emptied previews of their mesh (GBV #940: a BUFFER sampled as a
    // Texture2D, which over-culls). Select the PSO permutation without the test
    // rather than feeding it a fallback: in a single-asset preview there is
    // nothing to occlude against, so the whole test is dead weight.
    meshes_renderer->use_meshlet_hiz_occlusion = false;

    cam.position = vec3(0, 5, -30);

    mesh_plane.reset(new MeshAssetInstance(EngineAssets::plane.get_asset()));
    material_tester.reset(new MeshAssetInstance(EngineAssets::material_tester.get_asset()));

    scene = std::make_shared<Scene>();

    rendering = std::make_shared<SceneRenderWorkflow>();
    rendering->scene_renderer = scene_renderer;
    //	ssgi = std::make_shared<SSGI>(*gbuffer);

}


// ===========================================================================
//  SceneTextureRenderer — copy of AssetRenderer's scene-to-texture core.
//  (AssetRenderer left intact; it will later be rewritten to use this.)
// ===========================================================================

SceneTextureRenderer::SceneTextureRenderer()
{
    std::lock_guard<std::mutex> g(lock);

    preview_context->add_child(&graph);

    scene_renderer = std::make_shared<main_renderer>();
    {
        VariableContext::Scope scope(*preview_context);
        scene_renderer->register_renderer(meshes_renderer = std::make_shared<mesh_renderer>());
        settings = std::make_unique<PreviewSettings>();
    }

    // Meshlet Hi-Z occlusion needs FrameInfo::mainHiZ, which only the main view
    // binds (see main.cpp). This renderer has no pyramid to point it at, so the
    // test would run against whatever descriptor 0 happens to hold -- that is
    // what emptied previews of their mesh (GBV #940: a BUFFER sampled as a
    // Texture2D, which over-culls). Select the PSO permutation without the test
    // rather than feeding it a fallback: in a single-asset preview there is
    // nothing to occlude against, so the whole test is dead weight.
    meshes_renderer->use_meshlet_hiz_occlusion = false;

    cam.position = vec3(0, 5, -30);

    mesh_plane.reset(new MeshAssetInstance(EngineAssets::plane.get_asset()));
    material_tester.reset(new MeshAssetInstance(EngineAssets::material_tester.get_asset()));

    scene = std::make_shared<Scene>();

    rendering = std::make_shared<SceneRenderWorkflow>();
    rendering->scene_renderer = scene_renderer;
}

void SceneTextureRenderer::draw(Scene::ptr scene, HAL::Texture::ptr result)
{
    switch ((PreviewLobes)settings->lobes)
    {
    case PreviewLobes::WithoutLobe:
        render_scene(scene, result, true);
        break;

    case PreviewLobes::LobeDifference:
    {
        // Two renders at the target's size, subtracted on the GPU.
        if (!lobe_with || ivec2(lobe_with->get_size().xy) != ivec2(result->get_size().xy))
        {
            auto make = [&] {
                return std::make_shared<HAL::Texture>(RenderSystem::get().device(),
                    HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, { result->get_size().xy }, 1, 6,
                        HAL::ResFlags::ShaderResource | HAL::ResFlags::RenderTarget | HAL::ResFlags::UnorderedAccess));
            };

            lobe_with    = make();
            lobe_without = make();
        }

        render_scene(scene, lobe_with, false);
        render_scene(scene, lobe_without, true);
        subtract_lobes(lobe_with, lobe_without, result);
        break;
    }

    default:
        render_scene(scene, result, false);
        break;
    }
}

void SceneTextureRenderer::render_scene(Scene::ptr scene, HAL::Texture::ptr result, bool hide_lobe)
{
    graph.start_new_frame();

    scene->update_transforms();

    auto mn = scene->get_min();
    auto mx = scene->get_max();
    scene->add_child(mesh_plane);
    scene->update(*graph.builder.current_frame);

    float x = mx.x - mn.x;
    float y = mx.y - mn.y;

    mesh_plane->local_transform.scaling(std::max(x, y));

    mesh_plane->local_transform.a42 = mn.y;
    scene->update_transforms();

    auto& time      = graph.get_context<TimeInfo>();
    auto& skyinfo   = graph.get_context<SkyInfo>();
    auto& caminfo   = graph.get_context<CameraInfo>();
    auto& sceneinfo = graph.get_context<SceneInfo>();
    auto& vp        = graph.get_context<ViewportInfo>();

    auto  rsize  = result->get_size().xy;
    float aspect = float(rsize.x) / float(rsize.y);

    float base_dist = (mx - mn).length();
    if (base_dist < 0.001f) base_dist = 1.0f;

    // Orbit direction from yaw/pitch; distance scaled by zoom.
    vec3 dir;
    dir.x = Math::cos(m_orbit.y) * Math::sin(m_orbit.x);
    dir.y = Math::sin(m_orbit.y);
    dir.z = Math::cos(m_orbit.y) * Math::cos(m_orbit.x);

    cam.target   = (mn + mx) / 2;
    cam.position = cam.target + dir * (base_dist * m_zoom);
    cam.set_projection_params(Math::pi / 4, aspect, 1, 100 + (mn - mx).length());
    cam.update();

    {
        // Azimuth about +y from +x, elevation above the horizon: 45 / 35.3 degrees is the old fixed (1, 1, 1).
        const float azimuth   = (float)settings->sun_azimuth * Math::pi / 180.0f;
        const float elevation = (float)settings->sun_elevation * Math::pi / 180.0f;
        skyinfo.sunDir = float3(Math::cos(elevation) * Math::cos(azimuth), Math::sin(elevation), Math::cos(elevation) * Math::sin(azimuth)).normalize();
        skyinfo.studio      = (PreviewEnvironment)settings->environment == PreviewEnvironment::Studio ? 1.0f : 0.0f;
        skyinfo.studio_fill = (float)settings->studio_fill;
    }

    rendering->debug_flags = hide_lobe ? (uint32_t)Dev::RTXDebugFlags::HideMaterialLobe : 0u;

    graph.builder.debug = true;
    // See AssetRenderer::draw's identical comment -- these are thumbnail/
    // live-editing-widget renders, not a real-time viewport, so render at
    // the target's actual resolution instead of half-res + upscale.
    vp.frame_size   = rsize;
    vp.upscale_size = rsize;

    sceneinfo.scene    = scene;
    sceneinfo.renderer = scene_renderer;
    caminfo.cam        = &cam;

    rendering->render(graph);
    graph.builder.pass_texture(FrameGraph::ResourceID::swapchain, result->resource, {}, ResourceFlags::Required);

    graph.setup();
    graph.compile(frame++);
    graph.render();
    graph.commit_command_lists();
    graph.reset();
    mesh_plane->remove_from_parent();
}

void SceneTextureRenderer::draw(scene_object::ptr obj, HAL::Texture::ptr result)
{
    std::lock_guard<std::mutex> g(lock);

    scene->add_child(obj);
    draw(scene, result);
    obj->remove_from_parent();
}

MeshAssetInstance::ptr SceneTextureRenderer::get_shape_instance()
{
    const auto shape = (PreviewShape)settings->shape;

    if (shape == PreviewShape::MaterialTester)
        return material_tester;

    auto& instance = shape_instances[shape];

    if (!instance)
    {
        auto& asset = shape == PreviewShape::Sphere      ? EngineAssets::preview_sphere
                    : shape == PreviewShape::RoundedCube ? EngineAssets::preview_rounded_cube
                    : shape == PreviewShape::Slab        ? EngineAssets::preview_slab
                                                         : EngineAssets::preview_cloth;
        instance.reset(new MeshAssetInstance(asset.get_asset()));
    }

    return instance;
}

void SceneTextureRenderer::draw(MaterialAsset::ptr mat, HAL::Texture::ptr result)
{
    std::lock_guard<std::mutex> g(lock);

    auto instance = get_shape_instance();
    // The material tester has the material on its second part (the first is the case around it).
    const size_t slot = (PreviewShape)settings->shape == PreviewShape::MaterialTester ? 1 : 0;

    instance->local_transform.scaling((float)settings->scale);
    scene->add_child(instance);
    instance->override_material(slot, mat);

    draw(scene, result);
    instance->remove_from_parent();
}

// result = |with - without| x 4: what the material's extra lobe adds to the picture.
void SceneTextureRenderer::subtract_lobes(HAL::Texture::ptr with, HAL::Texture::ptr without, HAL::Texture::ptr result)
{
    auto& device = RenderSystem::get().device();
    auto& queue  = device.get_queue(HAL::CommandListType::DIRECT);
    auto  list   = queue->get_free_list();
    list->begin(L"PreviewLobeDifference");

    HAL::Texture2DView with_view(with->resource, *list);
    HAL::Texture2DView without_view(without->resource, *list);
    HAL::Texture2DView result_view(result->resource, *list);

    // A fresh render target must be cleared before anything else touches it; the compute write below
    // would otherwise be its first operation (D3D12 #1422).
    {
        HAL::CompiledRT compiled_rt;
        compiled_rt.table_rtv = result_view.renderTarget;
        list->get_graphics().set_rtv(compiled_rt, HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0, vec4(0, 0, 0, 0));
    }

    auto& compute = list->get_compute();
    compute.set_signature(Layouts::DefaultLayout);

    Slots::Editor::PreviewDiffParams params;
    params.GetA()      = with_view.texture2D;
    params.GetB()      = without_view.texture2D;
    params.GetResult() = result_view.rwTexture2D;
    params.GetGain()   = 4.0f;
    compute.set(params);

    compute.set_pipeline<PSOS::Editor::PreviewDiff>();
    compute.dispatch(ivec2(result->get_size().xy), ivec2{ 8, 8 });
    list->execute_and_wait();
}

void SceneTextureRenderer::orbit(vec2 delta)
{
    m_orbit += delta;
    m_orbit.y = Math::clamp(m_orbit.y, -Math::m_pi_2 + Math::eps2, Math::m_pi_2 - Math::eps2);
}

void SceneTextureRenderer::zoom(float amount)
{
    m_zoom = Math::clamp(m_zoom * (1.0f - amount * 0.1f), 0.05f, 10.0f);
}
