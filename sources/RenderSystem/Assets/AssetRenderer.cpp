
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
    Pipelines::AssetPipeline pipeline;

    PSSM      pssm;
    SkyRender sky;

    Graph* m_graph = nullptr;
    ivec2  m_size  = {};

public:

    float time = 0;

    main_renderer::ptr scene_renderer;

    SceneRenderWorkflow() : pipeline(), sky(pipeline), pssm(pipeline)
    {
        pipeline.assetGBuffer.setup_func = [this](auto& data, TaskBuilder& builder) -> bool
        {
            GBufferViewDesc::create(m_size, data.gbuffer, builder);

            builder.need(data.scene, FrameGraph::ResourceFlags::PixelRead);

            builder.create(data.gbuffer.GBuffer_HiZ,     { ivec3(m_size / 8, 0), HAL::Format::R32_TYPELESS, 1 }, ResourceFlags::DepthStencil);
            builder.create(data.gbuffer.GBuffer_HiZ_UAV, { ivec3(m_size / 8, 0), HAL::Format::R32_FLOAT,    1 }, ResourceFlags::UnorderedAccess);
            return true;
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

            GBuffer gbuffer = GBufferViewDesc::actualize(data.gbuffer);
            gbuffer.HalfBuffer.hiZ_depth     = *data.gbuffer.GBuffer_HiZ;
            gbuffer.HalfBuffer.hiZ_depth_uav = *data.gbuffer.GBuffer_HiZ_UAV;

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

		pipeline.add_passes(graph);
        //	pssm.generate(graph);
        //	sky.generate(graph);
        //	sky.generate_sky(graph);

        graph.add_library_pass<Passes::AssetMip>([this, &graph](auto& data, TaskBuilder& builder) -> bool {
            builder.need(data.ResultTexture, ResourceFlags::ComputeRead);
            builder.need(data.swapchain, ResourceFlags::UnorderedAccess);
            GBufferViewDesc::need(builder, data.gbuffer);
            return true;
        }, [](auto& data, FrameContext& _context) {
            MipMapGenerator::get().render_texture_2d_slow(_context.get_list()->get_graphics(), *data.swapchain, *data.ResultTexture);
            MipMapGenerator::get().generate(_context.get_list()->get_compute(), *data.swapchain);
        });

        graph.add_slot_generator([this](Graph& graph) {
            auto& time    = graph.get_context<TimeInfo>();
            auto& skyinfo = graph.get_context<SkyInfo>();
            auto& cam     = graph.get_context<CameraInfo>();
            PROFILE(L"FrameInfo");
            Slots::FrameInfo frameInfo;
            //// hack zone
            auto sky = graph.builder.get(FrameGraph::ResourceID::sky_cubemap_filtered);
            if (sky && sky->resource)
                frameInfo.GetSky() = sky->get_handler<Handlers::TextureCube>()->textureCube;
            /////////
            frameInfo.GetSunDir().xyz = skyinfo.sunDir;
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
    vp.frame_size   = result->get_size().xy / 2;
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

    scene_renderer = std::make_shared<main_renderer>();
    scene_renderer->register_renderer(meshes_renderer = std::make_shared<mesh_renderer>());
    cam.position = vec3(0, 5, -30);

    mesh_plane.reset(new MeshAssetInstance(EngineAssets::plane.get_asset()));
    material_tester.reset(new MeshAssetInstance(EngineAssets::material_tester.get_asset()));

    scene = std::make_shared<Scene>();

    rendering = std::make_shared<SceneRenderWorkflow>();
    rendering->scene_renderer = scene_renderer;
    //	ssgi = std::make_shared<SSGI>(*gbuffer);

}
