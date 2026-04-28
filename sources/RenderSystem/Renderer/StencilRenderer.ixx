export module Graphics:StencilRenderer;
import :Camera;
import :MeshAsset;
import :Scene;
import :FrameGraphContext;

import FrameGraph;
import HAL;
import GUI;

export class stencil_renderer : public GUI::base, public Events::Runner, public VariableContext
{
        camera cam;
        camera axis_intersect_cam;

        MeshAssetInstance::ptr axis;
        camera axis_cam;

        int selected_axis = -1;
        float3 center_pos;
        float3 pivot_pos;
        std::pair<MeshAssetInstance::ptr, int> current_selected;

        void select_current();
        virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;
        virtual bool on_mouse_move(vec2 pos) override;

        float3 mouse_pos;
        float3 prev_mouse_pos;
        int mouse_on_axis = -1;

        float3 get_axis(int axis);
        float3 get_current_pos();
        float3 get_normal(int axis);
        std::pair<MeshAssetInstance::ptr, int> mouse_on_object;

        HAL::StructuredBufferView<vec4> vertex_buffer;
        HAL::IndexBuffer index_buffer;

        virtual bool can_accept(GUI::drag_n_drop_package::ptr p) override;
        virtual void on_drop_move(GUI::drag_n_drop_package::ptr p, vec2) override;
        virtual bool on_drop(GUI::drag_n_drop_package::ptr p, vec2 m) override;

        Plane current_plane;

    public:
        Scene::ptr debug_scene;
        Scene::ptr scene;
        camera* player_cam;

        Variable<bool> draw_aabb = { false, "Draw AABB", this };

        using ptr = s_ptr<stencil_renderer>;
        std::vector<std::pair<MeshAssetInstance::ptr, int>> selected;
        vec3 direction;

        stencil_renderer();

        template<typename TPipeline>
        explicit stencil_renderer(TPipeline& pipeline) : stencil_renderer()
        {
            pipeline.stencil_renderer_before.flags = FrameGraph::PassFlags::Required;

            pipeline.stencil_renderer_before.setup_func = [this](auto& data, FrameGraph::TaskBuilder& builder) -> bool
            {
                process_tasks();
                debug_scene->update_transforms();

                auto& caminfo = builder.graph->get_context<CameraInfo>();
                cam = *caminfo.cam;
                cam.set_projection_params(0.01f, 1.f, 0.1f, 10000.f);
                cam.target = cam.position + direction;
                cam.update();

                axis_cam = *caminfo.cam;
                vec3 dir = caminfo.cam->target - caminfo.cam->position;
                dir.normalize();
                axis_cam.set_projection_params(1, 1000);
                axis_cam.position -= center_pos;
                axis_cam.position.normalize();
                axis_cam.position *= 200;
                axis_cam.target = axis_cam.position + dir;
                axis_cam.update();

                axis_intersect_cam = axis_cam;
                axis_intersect_cam.set_projection_params(1, 1000);
                axis_intersect_cam.target = axis_intersect_cam.position + direction;
                axis_intersect_cam.update();

                builder.create(data.depth_tex,      { { 1,1,0 }, HAL::Format::R32_TYPELESS, 1 }, FrameGraph::ResourceFlags::DepthStencil);
                builder.create(data.id_buffer,      { 1 }, FrameGraph::ResourceFlags::UnorderedAccess);
                builder.create(data.axis_id_buffer, { 1 }, FrameGraph::ResourceFlags::UnorderedAccess);
                return true;
            };

            pipeline.stencil_renderer_before.render_func = [this](auto& data, FrameGraph::FrameContext& context)
            {
                auto& list     = *context.get_list();
                auto& graphics = list.get_graphics();
                auto& compute  = list.get_compute();
                auto& copy     = list.get_copy();

                auto obj = context.graph->get_context<SceneInfo>().scene;

                RT::DepthOnly::Compiled rtv;
                {
                    RT::DepthOnly rt;
                    rt.GetDepth() = data.depth_tex->depthStencil;
                    rtv = rt.compile(list);
                }

                std::vector<std::pair<MeshAssetInstance::ptr, int>> current;
                auto mesh_func = [&](MeshAssetInstance* l)
                {
                    for (unsigned int i = 0; i < l->rendering.size(); i++)
                    {
                        auto& m = l->rendering[i];
                        if (intersect(cam, m.primitive_global.get()) == INTERSECT_TYPE::FULL_OUT)
                            continue;
                        current.emplace_back(l->get_ptr<MeshAssetInstance>(), i);
                        graphics.set(m.compiled_mesh_info);
                        graphics.set(m.mesh_instance_info);
                        {
                            Slots::Instance instance;
                            instance.GetInstanceId() = (UINT)current.size();
                            graphics.set(instance);
                        }
                        graphics.dispatch_mesh(m.dispatch_mesh_arguments);
                    }
                };

                graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);
                graphics.set_pipeline<PSOS::DrawStencil>();
                graphics.set(scene->compiledScene);

                compute.clear(*data.id_buffer);
                compute.clear(*data.axis_id_buffer);

                {
                    Slots::FrameInfo frameInfo;
                    frameInfo.GetCamera() = cam.camera_cb.current;
                    graphics.set(frameInfo);
                }
                {
                    Slots::PickerBuffer buffer;
                    buffer.GetViewBuffer() = *data.id_buffer;
                    graphics.set(buffer);
                }

                graphics.set_rtv(rtv, RTOptions::Default | RTOptions::ClearDepth);

                obj->iterate([&](scene_object* node)
                {
                    auto render_object = dynamic_cast<Graphics::renderable*>(node);
                    if (render_object)
                        mesh_func(dynamic_cast<MeshAssetInstance*>(render_object));
                    return true;
                });

                graphics.set_rtv(rtv, RTOptions::ClearDepth);

                {
                    Slots::FrameInfo frameInfo;
                    frameInfo.GetCamera() = axis_intersect_cam.camera_cb.current;
                    graphics.set(frameInfo);
                }
                {
                    Slots::PickerBuffer buffer;
                    buffer.GetViewBuffer() = *data.axis_id_buffer;
                    graphics.set(buffer);
                }

                axis->iterate([&](scene_object* node)
                {
                    auto render_object = dynamic_cast<Graphics::renderable*>(node);
                    if (render_object)
                    {
                        auto l = dynamic_cast<MeshAssetInstance*>(render_object);
                        for (unsigned int i = 0; i < (UINT)l->rendering.size(); i++)
                        {
                            auto& m = l->rendering[i];
                            graphics.set(m.compiled_mesh_info);
                            graphics.set(m.mesh_instance_info);
                            {
                                Slots::Instance instance;
                                instance.GetInstanceId() = i + 1;
                                graphics.set(instance);
                            }
                            graphics.dispatch_mesh(m.dispatch_mesh_arguments);
                        }
                    }
                    return true;
                });

                copy.read<uint>(*data.id_buffer, 0, 1, [current, this](std::span<uint> memory)
                {
                    auto result = *memory.data() - 1;
                    run([result, this, current]()
                    {
                        mouse_on_object.first = nullptr;
                        if (result < current.size())
                            mouse_on_object = current[result];
                    });
                });

                copy.read<uint>(*data.axis_id_buffer, 0, 1, [this](std::span<uint> memory)
                {
                    auto result = *memory.data() - 1;
                    run([this, result]() { mouse_on_axis = result; });
                });
            };

            pipeline.stencil_renderer_after.setup_func = [this](auto& data, FrameGraph::TaskBuilder& builder) -> bool
            {
                if (selected.empty())
                    return false;
                auto& frame = builder.graph->get_context<ViewportInfo>();
                builder.need(data.ResultTexture, FrameGraph::ResourceFlags::RenderTarget);
                builder.create(data.Stencil_color_tex,
                    { ivec3(frame.frame_size, 0), HAL::Format::R8_SNORM, 1, 1 },
                    FrameGraph::ResourceFlags::RenderTarget);
                return true;
            };

            pipeline.stencil_renderer_after.render_func = [this](auto& data, FrameGraph::FrameContext& context)
            {
                auto& list     = *context.get_list();
                auto& graphics = list.get_graphics();

                graphics.set_signature(Layouts::DefaultLayout);
                graphics.set(scene->compiledScene);

                {
                    RT::SingleColor rt;
                    rt.GetColor() = data.Stencil_color_tex->renderTarget;
                    graphics.set_rtv(rt, RTOptions::Default | RTOptions::ClearAll);
                }

                graphics.set_pipeline<PSOS::DrawSelected>();
                graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);

                context.graph->set_slot(SlotID::FrameInfo, graphics);

                for (auto& sel : selected)
                {
                    auto& m = sel.first->rendering[sel.second];
                    graphics.set(m.compiled_mesh_info);
                    graphics.set(m.mesh_instance_info);
                    graphics.dispatch_mesh(m.dispatch_mesh_arguments);
                }

                // apply color mask
                {
                    graphics.set_pipeline<PSOS::StencilerLast>();
                    graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);
                    {
                        Slots::Countour contour;
                        contour.GetColor() = { 1, 0.5f, 0, 1 };
                        contour.GetTex()   = *data.Stencil_color_tex;
                        graphics.set(contour);
                    }
                    graphics.set_viewport(data.ResultTexture->get_viewport());
                    graphics.set_scissor(data.ResultTexture->get_scissor());
                    {
                        RT::SingleColor rt;
                        rt.GetColor() = data.ResultTexture->renderTarget;
                        graphics.set_rtv(rt);
                    }
                    {
             //           PROFILE_GPU(L"blend");
                        graphics.draw(4);
                    }
                }

                {
                    RT::SingleColor rt;
                    rt.GetColor() = data.ResultTexture->renderTarget;
                    graphics.set_rtv(rt);
                }

                if (draw_aabb)
                {
                    graphics.set_pipeline<PSOS::DrawBox>();
                    graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);
                    graphics.set_index_buffer(index_buffer.get_index_buffer_view());
                    {
                        Slots::DrawStencil draw;
                        draw.GetVertices() = vertex_buffer;
                        graphics.set(draw);
                    }
                    for (auto& sel : selected)
                    {
                        auto& m = sel.first->rendering[sel.second];
                        graphics.set(m.compiled_mesh_info);
                        graphics.set(m.mesh_instance_info);
                        graphics.draw_indexed(36, 0, 0);
                    }
                }

                // draw axis
                {
                    graphics.set_index_buffer(HAL::Views::IndexBuffer());
                    {
                        Slots::FrameInfo frameInfo;
                        frameInfo.GetCamera() = axis_cam.camera_cb.current;
                        graphics.set(frameInfo);
                    }
                    graphics.set_pipeline<PSOS::DrawAxis>();
                    graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);

                    int i = 0;
                    for (auto& m : axis->rendering)
                    {
                        float lighted = (mouse_on_axis == i) * 0.7f;
                        {
                            Slots::Color color;
                            color.GetColor() = { i == 0 ? 1.0f : lighted, i == 1 ? 1.0f : lighted, i == 2 ? 1.0f : lighted, 1 };
                            graphics.set(color);
                        }
                        graphics.set(m.compiled_mesh_info);
                        graphics.set(m.mesh_instance_info);
                        graphics.dispatch_mesh(m.dispatch_mesh_arguments);
                        i++;
                    }
                }
            };
        }
};
