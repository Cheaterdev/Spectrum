#include "pch.h"

#include "FlowSystem.h"
#include "Assets/EngineAssets.h"
#include "Assets/AssetRenderer.h"


#include "Effects/Sky.h"
#include "Effects/PostProcess/SMAA.h"
#include "Effects/VoxelGI/VoxelGI.h"
#include "Effects/RTX/RTX.h"
#include "Effects/FSR/FSR.h"

#include "Lighting/PSSM.h"

#include "Renderer/StencilRenderer.h"

#include "Materials/universal_material.h"

#include "GUI/Elements/AssetExplorer.h"
#include "GUI/Debugging/TimerGraph.h"
#include "GUI/Debugging/OutputWindow.h"
#include "GUI/Debugging/TaskViewer.h"
#include "GUI/Debugging/TimerWatcher.h"

#include "GUI/Elements/FlowGraph/FlowManager.h"
#include "GUI/Elements/FlowGraph/ParameterWindow.h"
#include "GUI/Elements/CircleSelector.h"
#include "GUI/Elements/Tree.h"
#include "GUI/Elements/DockBase.h"
#include "GUI/Elements/ComboBox.h"
#include "GUI/Elements/ListBox.h"

import Graphics;


#include "Platform/Window.h"

import ppl;
import Debug;
import Utils;
using namespace FrameGraph;

HRESULT device_fail()
{
	auto hr = Graphics::Device::get().get_native_device()->GetDeviceRemovedReason();

	Graphics::Device::get().DumpDRED();
	Log::get().crash_error(hr);
	return hr;
}


class tick_timer
{
	std::chrono::time_point<std::chrono::system_clock> last_tick;

public:
	tick_timer()
	{
		last_tick = std::chrono::system_clock::now();
	}

	double tick()
	{
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = now - last_tick;
		last_tick = now;
		return elapsed_seconds.count();
	}
};

class count_meter
{
	double time = 0;
	tick_timer t;
	unsigned int ticks = 0;

	double average = 0;
public:

	bool tick()
	{
		time += t.tick();
		ticks++;

		if (time > 1)
		{
			average = ticks / time;
			ticks = 0;
			time = 0;
			return true;
		}

		return false;
	}

	float get()
	{
		return (float)average;
	}
};


class GraphDebugRender : public GUI::Elements::FlowGraph::canvas
{
public:

	void generate(Graph& graph)
	{

	}

};

class triangle_drawer : public GUI::Elements::image, public GraphGenerator, VariableContext, public GraphUsage
{
	main_renderer::ptr scene_renderer;
	main_renderer::ptr gpu_scene_renderer;
	stencil_renderer::ptr stenciler;

	GUI::Elements::label::ptr info;

	size_t time = 0;
	struct EyeData : public Events::prop_handler
	{

		using ptr = std::shared_ptr<EyeData>;
		first_person_camera cam;



		EyeData(Graphics::RootSignature::ptr sig)
		{

		}
	};

	std::vector<EyeData::ptr> eyes;
	first_person_camera cam;
public:
	using ptr = std::shared_ptr<triangle_drawer>;
	//	PostProcessGraph::ptr render_graph;


	Variable<bool> enable_gi = { true, "GI", this };
	Variable<bool> enable_fsr = { true, "FSR", this };
	Variable<bool> downsampled = { true, "downsampled", this };

	//Variable<bool> debug_draw = Variable<bool>(false, "debug_draw",this);
	//	VoxelGI::ptr voxel_renderer;

	int visible_count;

	mesh_renderer::ptr meshes_renderer;

	//	gpu_cached_renderer::ptr gpu_meshes_renderer_static;
	//	gpu_cached_renderer::ptr gpu_meshes_renderer_dynamic;

	Scene::ptr scene;
	Graphics::QueryHeap::ptr query_heap;
	float draw_time;
	MeshAssetInstance::ptr instance;




	std::shared_ptr<OVRContext> vr_context = std::make_shared<OVRContext>();
	PSSM pssm;
	SMAA smaa;
	SkyRender sky;
	FSR fsr;
	VoxelGI::ptr voxel_gi;
	std::string debug_view;
	triangle_drawer() :VariableContext(L"triangle_drawer")
	{
		texture.mul_color = { 1,1,1,0 };
		texture.add_color = { 0,0,0,1 };

		//	texture.srv = Graphics::StaticDescriptors::get().place(1);



		auto t = CounterManager::get().start_count<triangle_drawer>();
		thinkable = true;
		clickable = true;

		scene.reset(new Scene());
		scene->name = L"Scene";

		query_heap = std::make_shared<  Graphics::QueryHeap>(2, D3D12_QUERY_HEAP_TYPE::D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS);
		scene_renderer = std::make_shared<main_renderer>();
		scene_renderer->register_renderer(meshes_renderer = std::make_shared<mesh_renderer>());


		gpu_scene_renderer = std::make_shared<main_renderer>();

		gpu_scene_renderer->register_renderer(std::make_shared<mesh_renderer>());


		//gpu_scene_renderer->register_renderer(gpu_meshes_renderer_static = std::make_shared<gpu_cached_renderer>(scene, MESH_TYPE::STATIC));
		//gpu_scene_renderer->register_renderer(gpu_meshes_renderer_dynamic = std::make_shared<gpu_cached_renderer>(scene, MESH_TYPE::DYNAMIC));
		//cam.position = vec3(0, 5, -30);

		cam.position = vec3(0, 0, 0);
		Graphics::PipelineStateCache::create();
		stenciler.reset(new stencil_renderer());
		stenciler->player_cam = &cam;
		stenciler->scene = scene;
		base::add_child(stenciler);

		info.reset(new GUI::Elements::label);
		info->docking = GUI::dock::TOP;
		info->x_type = GUI::pos_x_type::LEFT;
		info->magnet_text = FW1_LEFT;
		base::add_child(info);
		base::ptr props(new base);
		props->docking = GUI::dock::FILL;
		props->x_type = GUI::pos_x_type::LEFT;
		props->y_type = GUI::pos_y_type::TOP;
		props->width_size = GUI::size_type::MATCH_CHILDREN;
		props->height_size = GUI::size_type::MATCH_CHILDREN;
		base::add_child(props);

		{
			auto combo = std::make_shared<GUI::Elements::combo_box>();

			combo->size = { 200, 25 };
			combo->on_click = [this](GUI::Elements::button::ptr butt) {

				GUI::Elements::combo_box::ptr combo = butt->get_ptr<GUI::Elements::combo_box>();
				combo->remove_items();


				combo->add_item("Normal")->on_click = [this](GUI::Elements::menu_list_element::ptr) {debug_view = ""; };


				for (auto& e : last_graph->builder.alloc_resources)
				{

					if (e.second.type != FrameGraph::ResourceType::Texture) continue;

					std::string str = e.first;
					combo->add_item(str)->on_click = [this, str](GUI::Elements::menu_list_element::ptr) {debug_view = str; };

				}

			};




			props->add_child(combo);
		}


		GUI::Elements::circle_selector::ptr circle(new  GUI::Elements::circle_selector);
		circle->docking = GUI::dock::FILL;
		circle->x_type = GUI::pos_x_type::RIGHT;
		circle->y_type = GUI::pos_y_type::TOP;


		base::add_child(circle);
		//	lighting = std::make_shared<LightingNode>();

		circle->on_change.register_handler(this, [this](const float2& value)
			{
				float2 v = value;
				float3 dir = { 0.001 + v.x,sqrt(1.001 - v.length_squared()),-v.y };
				pssm.set_position(dir);
			});

		circle->set_value({ 1,0 });



		MeshAsset::ptr asset_ptr = EngineAssets::material_tester.get_asset();



		auto make_material = [](float3 color, float roughness, float metallic) {
			MaterialGraph::ptr graph(new MaterialGraph);


			{

				auto value_node = std::make_shared<VectorNode>(vec4(color, 1));
				graph->register_node(value_node);
				value_node->get_output(0)->link(graph->get_base_color());
			}


			{
				auto value_node = std::make_shared<ScalarNode>(roughness);
				graph->register_node(value_node);
				value_node->get_output(0)->link(graph->get_roughness());
			}

			{
				auto value_node = std::make_shared<ScalarNode>(metallic);
				graph->register_node(value_node);
				value_node->get_output(0)->link(graph->get_mettalic());

			}


			materials::universal_material* m = (new materials::universal_material(graph));
			//m->generate_material();
			m->register_new();
			return m->get_ptr<MaterialAsset>();
		};



		//	auto base_mat = make_material({ 1,1,1 }, 1, 0);

		int count = -1;
		float distance = 5;
		for (int i = 0; i <= count; i++)
			for (int j = 0; j <= count; j++)
			{
				MeshAssetInstance::ptr instance(new MeshAssetInstance(asset_ptr));
				//	instance->override_material(0, base_mat);
				//	instance->override_material(1, make_material({ 1,0,0 }, float(i) / count, float(j) / count));

				instance->local_transform[3] = { i * distance - count * distance / 2,0,j * distance - count * distance / 2,1 };

				scene->add_child(instance);
			}

		{
			MeshAsset::ptr ruins_ptr = EngineAssets::plane.get_asset();


			if (ruins_ptr)
			{
				MeshAssetInstance::ptr instance(new MeshAssetInstance(ruins_ptr));
				//	instance->local_transform = mat4x4::translation({ 0,3.8,0 });
				scene->add_child(instance);
			}

		}


		{
			MeshAsset::ptr ruins_ptr = nullptr;// AssetManager::get().find_by_name<MeshAsset>(L"cubes.obj");


			if (ruins_ptr)
			{
				MeshAssetInstance::ptr instance(new MeshAssetInstance(ruins_ptr));
				//	instance->local_transform=mat4x4::translation({ 0,3.8,0 });
				scene->add_child(instance);
			}

		}

		eyes.emplace_back(new EyeData(nullptr));

		voxel_gi = std::make_shared<VoxelGI>(scene);
	}
	float scale_speed = 0;
	vec2 wheel_pos;
	bool on_wheel(mouse_wheel type, float value, vec2 pos)
	{
		scale_speed += value * 0.1f;
		wheel_pos = pos;
		return true;

	}

	void think(float dt)
	{
		//	if (!g_buffer.size.has_value())
			//	return;

		scene->update_transforms();

		auto min = scene->get_min();
		auto max = scene->get_max();


		auto points = cam.get_points(min, max);
		auto bounds = points.get_bounds_in(cam.get_view());

		//cam.set_projection_params(std::max(1.0f, bounds.znear - 1), bounds.zfar + 1);
		cam.set_projection_params(1, 1500);


		cam.frame_move(dt);

	}



	void draw_eye(Graphics::CommandList::ptr _list, float dt, EyeData& data, Graphics::Texture::ptr target)
	{

	}

	void update_texture(Graphics::CommandList::ptr list, float dt, const std::shared_ptr<OVRContext>& vr)
	{


	}

	Graph* last_graph = nullptr;
	tick_timer my_timer;
	ResourceAllocInfo* debug_tex_handle = nullptr;

	void generate(Graph& graph)
	{

		last_graph = &graph;
		vr_context->eyes.resize(1);
		vr_context->eyes[0].dir = quat();

		vr_context->eyes[0].offset = vec3(0, 0, 0);

		ivec2 size = ivec2::max(ivec2(get_render_bounds().size), ivec2(64, 64));
		struct pass_data
		{
			ResourceHandler* o_texture;
			ResourceHandler* sky_cubemap;
			GBufferViewDesc gbuffer;

		};

		scene->update(*graph.builder.current_frame);

		if (downsampled)
			graph.frame_size = size / 1.5;
		else
			graph.frame_size = size;

		graph.upscale_size = size;

		graph.scene = scene.get();
		graph.renderer = gpu_scene_renderer.get();
		graph.cam = &cam;
		graph.time = (float)my_timer.tick();
		graph.totalTime += graph.time;
		graph.sunDir = pssm.get_position();
		cam.update({ 0,0 });


		for (int i = 0; i < eyes.size(); i++)
		{
			eyes[i]->cam = cam;
			eyes[i]->cam.update({ 0,0 });
		}

		{

			CommandList::ptr command_list = Graphics::Device::get().get_queue(CommandListType::DIRECT)->get_free_list();

			command_list->begin("pre");
			{
				SceneFrameManager::get().prepare(command_list, *scene);
				if (Graphics::Device::get().is_rtx_supported())
				{
					scene->raytrace_scene->update(command_list, (UINT)scene->raytrace->max_size(), scene->raytrace->buffer->get_resource_address(), false);
					RTX::get().prepare(command_list);
				}
			}

			command_list->end();

			command_list->execute();
		}


		{
			struct GBufferData
			{
				GBufferViewDesc gbuffer;

				Handlers::Texture H(GBuffer_HiZ);
				Handlers::Texture H(GBuffer_HiZ_UAV);


			};

			/*graph.add_pass<GBufferData>("GBUFFER", [this, &graph](GBufferData& data, TaskBuilder& builder) {

				auto size = graph.frame_size;
				data.gbuffer.create(size, builder);
				data.gbuffer.create_mips(size, builder);
				data.gbuffer.create_quality(size, builder);

			//	builder.create(data.GBuffer_HiZ, { ivec3(size / 8, 1), Graphics::Format::R32_TYPELESS, 1 }, ResourceFlags::DepthStencil);
			//	builder.create(data.GBuffer_HiZ_UAV, { ivec3(size / 8, 1), Graphics::Format::R32_FLOAT,1 }, ResourceFlags::UnorderedAccess);

				}, [this, &graph](GBufferData& data, FrameContext& _context) {

				});*/


			graph.add_pass<GBufferData>("SCENE", [this, &graph](GBufferData& data, TaskBuilder& builder) {

				auto size = graph.frame_size;
				data.gbuffer.create(size, builder);
				data.gbuffer.create_mips(size, builder);
				data.gbuffer.create_quality(size, builder);

				builder.create(data.GBuffer_HiZ, { ivec3(size / 8, 1), Graphics::Format::R32_TYPELESS, 1 }, ResourceFlags::DepthStencil);
				builder.create(data.GBuffer_HiZ_UAV, { ivec3(size / 8, 1), Graphics::Format::R32_FLOAT,1 }, ResourceFlags::UnorderedAccess);

				}, [this, &graph](GBufferData& data, FrameContext& _context) {

					auto& command_list = _context.get_list();

					//std::this_thread::sleep_for(1ms);
				//	gpu_scene_renderer->render(context_gbuffer, scene);
					MeshRenderContext::ptr context(new MeshRenderContext());


					context->current_time = time;
					//		context->sky_dir = lighting->lighting.pssm.get_position();
					context->priority = TaskPriority::HIGH;
					context->list = command_list;
					context->eye_context = vr_context;

					context->cam = &eyes[0]->cam;

					command_list->get_graphics().set_layout(Layouts::DefaultLayout);
					command_list->get_compute().set_layout(Layouts::DefaultLayout);


					//				gpu_meshes_renderer_static->update(context);
						//			gpu_meshes_renderer_dynamic->update(context);

					GBuffer gbuffer = data.gbuffer.actualize(_context);

					gbuffer.rtv_table = RenderTargetTable(context->list->get_graphics(), { gbuffer.albedo, gbuffer.normals, gbuffer.specular, gbuffer.speed }, gbuffer.depth);

					gbuffer.HalfBuffer.hiZ_depth = *(data.GBuffer_HiZ);
					gbuffer.HalfBuffer.hiZ_table = RenderTargetTable(context->list->get_graphics(), {  }, gbuffer.HalfBuffer.hiZ_depth);
					gbuffer.HalfBuffer.hiZ_depth_uav = *(data.GBuffer_HiZ_UAV);

					context->g_buffer = &gbuffer;

					gbuffer.rtv_table.set(context, true, true);
					gbuffer.rtv_table.set_window(context->list->get_graphics());

					RT::Slot::GBuffer rt_gbuffer;


					rt_gbuffer.GetAlbedo() = gbuffer.albedo.renderTarget;
					rt_gbuffer.GetNormals() = gbuffer.normals.renderTarget;
					rt_gbuffer.GetSpecular() = gbuffer.specular.renderTarget;
					rt_gbuffer.GetMotion() = gbuffer.speed.renderTarget;

					rt_gbuffer.GetDepth() = gbuffer.depth.depthStencil;

					context->gbuffer_compiled = rt_gbuffer.compile(*context->list);

					graph.set_slot(SlotID::FrameInfo, command_list->get_graphics());
					graph.set_slot(SlotID::FrameInfo, command_list->get_compute());

					gpu_scene_renderer->render(context, scene);

					//	stenciler->render(context, scene);

					command_list->get_copy().copy_texture(gbuffer.depth_prev_mips.resource, 0, gbuffer.depth_mips.resource, 0);
					command_list->get_copy().copy_texture(gbuffer.depth_mips.resource, 0, gbuffer.depth.resource, 0);

					//	
				});
		}

		{
			struct RTXDebugData
			{
				GBufferViewDesc gbuffer;

				Handlers::Texture H(RTXDebug);

				Handlers::Texture H(RTXDebugPrev);
			};

			if (Graphics::Device::get().is_rtx_supported())
				graph.add_pass<RTXDebugData>("RTXDebug", [this, &graph](RTXDebugData& data, TaskBuilder& builder) {
				auto size = graph.frame_size;
				data.gbuffer.need(builder, false);
				builder.create(data.RTXDebug, { ivec3(size, 1), Graphics::Format::R16G16B16A16_FLOAT, 1 }, ResourceFlags::UnorderedAccess | ResourceFlags::Static);
				builder.create(data.RTXDebugPrev, { ivec3(size, 1), Graphics::Format::R16G16B16A16_FLOAT, 1 }, ResourceFlags::UnorderedAccess | ResourceFlags::Static);

					}, [this, &graph](RTXDebugData& data, FrameContext& context) {
						auto& compute = context.get_list()->get_compute();
						auto& copy = context.get_list()->get_copy();

						if (data.RTXDebug.is_new())
						{
							context.get_list()->clear_uav(data.RTXDebug->rwTexture2D, vec4(0, 0, 0, 0));
						}

						compute.set_signature(RTX::get().rtx.m_root_sig);

						graph.set_slot(SlotID::VoxelInfo, compute);
						graph.set_slot(SlotID::FrameInfo, compute);
						graph.set_slot(SlotID::SceneData, compute);

						{
							Slots::VoxelOutput output;
							output.GetNoise() = data.RTXDebug->rwTexture2D;
							output.set(compute);
						}

						{
							auto gbuffer = data.gbuffer.actualize(context);

							Slots::VoxelScreen voxelScreen;
							gbuffer.SetTable(voxelScreen.GetGbuffer());
							voxelScreen.GetPrev_depth() = gbuffer.depth_prev_mips.texture2D;
							voxelScreen.GetPrev_gi() = data.RTXDebugPrev->texture2D;
							voxelScreen.set(compute);
						}
						RTX::get().render<Shadow>(compute, scene->raytrace_scene, data.RTXDebug->get_size());


						copy.copy_resource(data.RTXDebugPrev->resource, data.RTXDebug->resource);
					});
		}


		struct no
		{
			Handlers::Texture H(ResultTexture);

		};
		graph.add_pass<no>("no", [this, &graph](no& data, TaskBuilder& builder) {
			builder.create(data.ResultTexture, { uint3(graph.frame_size,1), Graphics::Format::R16G16B16A16_FLOAT, 1 });
			}, [](no& data, FrameContext& _context) {});



		pssm.generate(graph);
		sky.generate(graph);

		// remove on intel
		if (enable_gi) voxel_gi->generate(graph);


		sky.generate_sky(graph);

		stenciler->generate_after(graph);

		smaa.generate(graph);
		if (downsampled && enable_fsr)
			fsr.generate(graph);


		struct debug_data
		{
			Handlers::Texture debug_tex;
		};

		graph.add_slot_generator([this](Graph& graph) {

			PROFILE(L"FrameInfo");
			Slots::FrameInfo frameInfo;
			//// hack zone
			auto& sky = graph.builder.alloc_resources["sky_cubemap_filtered"];
			if (sky.resource)
				frameInfo.GetSky() = sky.get_handler<Handlers::Texture>()->textureCube;
			/////////
			frameInfo.GetSunDir().xyz = graph.sunDir;
			frameInfo.GetTime() = { graph.time ,graph.totalTime,0,0 };


			frameInfo.GetCamera() = graph.cam->camera_cb.current;
			frameInfo.GetPrevCamera() = graph.cam->camera_cb.prev;

			frameInfo.GetBrdf() = EngineAssets::brdf.get_asset()->get_texture()->texture_3d()->texture3D;
			frameInfo.GetBestFitNormals() = EngineAssets::best_fit_normals.get_asset()->get_texture()->texture_2d()->texture2D;

			auto compiled = frameInfo.compile(*graph.builder.current_frame);
			graph.register_slot_setter(compiled);
			});

		graph.add_slot_generator([this](Graph& graph) {
			graph.register_slot_setter(scene->compiledScene);
			});

	}

	Handlers::Texture debug_tex;

	void use(TaskBuilder& builder) override
	{
		std::string res_tex = "ResultTexture";
		if (!debug_view.empty())
			res_tex = debug_view;

		debug_tex = Handlers::Texture(res_tex);
		if (builder.exists(debug_tex))
			builder.need(debug_tex, ResourceFlags::PixelRead);
	}
	virtual void draw(Graphics::context& t) override
	{
		if (debug_tex) texture.srv = debug_tex->texture2D;
		image::draw(t);
	}





	virtual void on_bounds_changed(const rect& r) override
	{
		base::on_bounds_changed(r);
		if (r.w <= 64 || r.h <= 64) return;
		ivec2 size = r.size;
		cam.set_projection_params(Math::pi / 4, float(r.w) / r.h, 1, 1500);
	}



};

UINT64 frame_counter = 0;

class FrameFlowGraph : public  ::FlowGraph::graph
{

};

class PassNode : public::FlowGraph::Node, public  GUI::Elements::FlowGraph::VisualGraph
{

	virtual  void operator()(::FlowGraph::GraphContext*)
	{}
	GUI::base::ptr create_editor_window() override
	{
		GUI::Elements::image::ptr img(new GUI::Elements::image);
		img->texture.texture = Graphics::Texture::get_resource({ to_path(L"textures/gui/shadow.png"), false, false });
		img->texture.padding = { 9, 9, 9, 9 };
		img->padding = { 9, 9, 9, 9 };
		img->width_size = GUI::size_type::MATCH_CHILDREN;
		img->height_size = GUI::size_type::MATCH_CHILDREN;
		return img;
	}
};

class GraphRender : public Window, public GUI::user_interface
{
	Graphics::SwapChain::ptr swap_chain;

	tick_timer main_timer;
	ivec2 new_size;

	std::shared_ptr<OVRContext> vr_context = std::make_shared<OVRContext>();
	std::future<void> task_future;

	Graph graph;

	count_meter fps;

	GUI::Elements::label::ptr label_fps;
	GUI::Elements::label::ptr label_tiles;
	GUI::Elements::label::ptr instance_info;

	std::shared_ptr<triangle_drawer> drawer;
	std::shared_ptr<FrameFlowGraph> frameFlowGraph;
	GUI::base::ptr area;


	size_t graph_usage = 0;

public:
	void on_destroy() override
	{
		Application::get().shutdown();
	}

	virtual	void render()
	{
		if (swap_chain)	swap_chain->resize(new_size);


		{
			std::lock_guard<std::mutex> g(m);

			if (GetAsyncKeyState('R'))
			{
				Graphics::Device::get().get_queue(Graphics::CommandListType::DIRECT)->signal_and_wait();
				Graphics::Device::get().get_queue(Graphics::CommandListType::COMPUTE)->signal_and_wait();
				Graphics::Device::get().get_queue(Graphics::CommandListType::COPY)->signal_and_wait();

				//   AssetManager::get().reload_resources();
				Graphics::pixel_shader::reload_all();
				Graphics::vertex_shader::reload_all();
				Graphics::geometry_shader::reload_all();
				Graphics::hull_shader::reload_all();
				Graphics::domain_shader::reload_all();
				Graphics::compute_shader::reload_all();
				Graphics::library_shader::reload_all();
				Graphics::mesh_shader::reload_all();
				Graphics::amplification_shader::reload_all();

				Graphics::Texture::reload_all();
			}

			Profiler::get().on_frame(frame_counter++);

			GUI::user_interface::size = new_size;
			if (fps.tick())
			{
				size_t total = 0;
				for (auto& h : DescriptorHeapManager::get().heaps)
				{
					if (h)
					{
						total += h->used_size();
					}
				}


				size_t total_gpu = 0;


				label_fps->text = std::to_string(fps.get()) + " " + std::to_string(Graphics::Device::get().get_vram()) + " " + std::to_string(total) + " " + std::to_string(total_gpu) + " " + std::to_string(graph_usage);
			}


			process_ui((float)main_timer.tick());
			{
				PROFILE(L"Wait next");

				swap_chain->start_next();
			}

			setup_graph();

			graph.render();

			swap_chain->wait_for_free();
			graph.commit_command_lists();
			{

				PROFILE(L"reset");

				graph.reset();
			}
			swap_chain->present(Graphics::Device::get().get_queue(Graphics::CommandListType::DIRECT)->signal());
		}



		if (Application::get().is_alive())
		{
			auto ptr = get_ptr();
			task_future = scheduler::get().enqueue([ptr, this]() {

				render();
				std::this_thread::yield();
				}, std::chrono::steady_clock::now());
		}
	}



	void setup_graph()
	{


		struct pass_data
		{
			Handlers::Texture H(swapchain);

		};


		struct pass_no
		{

		};

		graph.start_new_frame();
		graph.builder.pass_texture("swapchain", swap_chain->get_current_frame());
		graph.builder.pass_texture("swapchain_prev", swap_chain->get_prev_frame());


		{

			PROFILE(L"create_graph");
			create_graph(graph);


			auto ptr = get_ptr();
			//	if(false)
			graph.add_pass<pass_data>("PROFILER", [](pass_data& data, TaskBuilder& builder) {
				builder.need(data.swapchain, ResourceFlags::Required);
				}, [this, ptr](pass_data& data, FrameContext& context) {

					context.get_list()->transition_present(data.swapchain->resource.get());

					Graphics::GPUTimeManager::get().read_buffer(context.get_list(), [ptr, this]() {
						run_on_ui([this, ptr]() {	Profiler::get().update(); });

						});

				}, PassFlags::Required);

		}

		graph.setup();
		graph.compile(swap_chain->m_frameIndex);


		static bool gen = false;

		if (!gen && GetAsyncKeyState('N'))
		{
			gen = true;
			frameFlowGraph->clear();

			struct res_stage
			{

			};
			std::map<ResourceAllocInfo*, ::FlowGraph::parameter::ptr> resource_stages;


			for (auto& res : graph.builder.alloc_resources)
			{

				if (res.second.passed)
				{
					auto input = frameFlowGraph->register_input(res.second.name);
					resource_stages[&res.second] = input;
				}
			}
			for (auto pass : graph.passes)
			{
				auto node = std::make_shared<PassNode>();
				node->name = pass->name + " " + std::to_string(pass->id);


				if (!pass->enabled)
				{
					node->color = float4(1, 0, 0, 1);
				}

				if (check(pass->flags & PassFlags::Required))
				{
					node->color = float4(1, 1, 0, 1);
				}

				frameFlowGraph->register_node(node);

				for (auto& info : pass->used.resources)
				{
					if (pass->used.resource_creations.count(info) == 0) {
						auto input = node->register_input(info->name);
						auto prev = resource_stages[info];

						if (prev)
						{


							prev->link(input);
						}
						auto output = node->register_output(info->name);

						resource_stages[info] = output;
					}
				}


				for (auto& info : pass->used.resources)
				{

					if (pass->used.resource_creations.count(info))
					{
						auto output = node->register_output(info->name);
						resource_stages[info] = output;
					}
				}
			}


			for (auto& res : graph.builder.alloc_resources)
			{

				if (res.second.passed && check(res.second.flags & ResourceFlags::Required))
				{
					auto input = frameFlowGraph->register_output(res.second.name);
					auto prev = resource_stages[&res.second];

					if (prev)
					{


						prev->link(input);
					}
				}
			}



		}

	}

	GraphRender()
	{
		Window::input_handler = this;
		Graphics::swap_chain_desc desc;
		desc.format = Graphics::Format::R8G8B8A8_UNORM;
		desc.fullscreen = nullptr;
		desc.stereo = false;
		desc.window = this;
		swap_chain = Graphics::Device::get().create_swap_chain(desc);

		set_capture = [this](bool v)
		{
			if (v)
				SetCapture(get_hwnd());
			else
				ReleaseCapture();
		};

		on_resize(get_size());




		{
			GUI::Elements::image::ptr back(new GUI::Elements::image);
			back->texture = Graphics::Texture::get_resource(Graphics::texure_header(to_path(L"textures/gui/back_fill.png"), false, false));
			back->texture.tiled = true;
			back->width_size = GUI::size_type::MATCH_PARENT;
			back->height_size = GUI::size_type::MATCH_PARENT;
			add_child(back);
		}

		EVENT("Start UI");
		// 		if (false)
		{
			area.reset(new GUI::base());
			area->docking = GUI::dock::FILL;
			add_child(area);
			GUI::Elements::dock_base::ptr  d = std::make_shared<GUI::Elements::dock_base>();
			d->docking = GUI::dock::FILL;
			{
				EVENT("Start Drawer");
				drawer.reset(new triangle_drawer());
				drawer->docking = GUI::dock::FILL;

				d->get_tabs()->add_page("Game", drawer);
				EVENT("End Drawer");
			}
			{
				GUI::Elements::list_box::ptr l(new GUI::Elements::list_box());
				auto dock = d->get_dock(GUI::dock::BOTTOM);
				dock->size = { 100, 100 };
				area->add_child(d);
				dock->get_tabs()->add_page("TaskViewer", std::make_shared<GUI::Elements::Debug::TaskViewer>());

				{

					auto b = std::make_shared<GUI::base>();


					auto folders = std::make_shared<GUI::Elements::tree<VariableContext>>();
					folders->size = { 200, 150 };
					folders->docking = GUI::dock::LEFT;

					auto table = std::make_shared<GUI::base>();
					table->docking = GUI::dock::FILL;

					b->add_child(folders);
					b->add_child(table);

					folders->on_select = [this, table](VariableContext* elem)
					{

						table->remove_all();

						for (auto v : elem->variables)
						{
							auto property = GUI::Elements::create_property(*v);
							table->add_child(property);
						}
					};

					folders->init(&VariableContext::get());

					dock->get_tabs()->add_page("Properties", b);

				}
				//			dock->get_tabs()->add_page("output", std::make_shared<GUI::Elements::Debug::OutputWindow>());
							//       GUI::Elements::tree::ptr t(new GUI::Elements::tree());
			//                    t->init(drawer->scene.get());
			//                    dock->get_tabs()->add_page("Scene", t);
				{
					//	GUI::Elements::Debug::TimerWatcher::ptr t(new GUI::Elements::Debug::TimerWatcher());
					//	t->init(&Profiler::get());
					//	dock->get_tabs()->add_page("Profiler", t);

					GUI::Elements::Debug::TimeGraph::ptr t2(new GUI::Elements::Debug::TimeGraph());

					dock->get_tabs()->add_page("Graph", t2);



					frameFlowGraph = std::make_shared< FrameFlowGraph>();

					dock->get_tabs()->add_button(GUI::Elements::FlowGraph::manager::get().add_graph(frameFlowGraph));

				}
			}

			{
				{
					GUI::Elements::menu_strip::ptr menu(new GUI::Elements::menu_strip());
					auto file = menu->add_item("File")->get_menu();
					auto edit = menu->add_item("Edit")->get_menu();
					auto help = menu->add_item("Help");// ->get_menu();
					file->add_item("New")->on_click = [this](GUI::Elements::menu_list_element::ptr elem)
					{
						add_task([this]()
							{
								drawer->scene->remove_all();
							});
					};
					file->add_item("Open")->on_click = [this](GUI::Elements::menu_list_element::ptr elem)
					{
						add_task([this]()
							{
								try
								{
									auto f = FileSystem::get().get_file(to_path(L"scene.dat"))->load_all();
									Serializer::deserialize(f, *drawer->scene);
								}
								catch (std::exception e)
								{
									message_box("error", "cant open", [](bool) {});
								}


							});
					};
					file->add_item("Save")->on_click = [this](GUI::Elements::menu_list_element::ptr elem)
					{
						auto data = Serializer::serialize(*drawer->scene);
						FileSystem::get().save_data(to_path(L"scene.dat"), data);
					};
					file->add_item("Quit")->on_click = [this](GUI::Elements::menu_list_element::ptr elem)
					{
						on_destroy();
					};
					auto add = edit->add_item("Add smth")->get_menu();
					add->add_item("Mesh");
					add->add_item("Material");
					add->add_item("Sound");
					//->get_menu()->add_item("12454");
					add_child(menu);
				}
				{
					GUI::Elements::status_bar::ptr bar(new GUI::Elements::status_bar());
					label_fps = GUI::Elements::label::ptr(new GUI::Elements::label());
					instance_info = GUI::Elements::label::ptr(new GUI::Elements::label());
					label_tiles = GUI::Elements::label::ptr(new GUI::Elements::label());

					bar->add_child(label_fps);
					bar->add_child(label_tiles);

					bar->add_child(instance_info);
					instance_info->docking = GUI::dock::RIGHT;
					label_tiles->margin = { 20,0,0,0 };
					add_child(bar);
				}


				{
					EVENT("Start Asset Explorer");
					auto dock = d->get_dock(GUI::dock::RIGHT);
					GUI::Elements::asset_explorer::ptr cont(new GUI::Elements::asset_explorer());
					dock->get_tabs()->add_page("Asset Explorer", cont);
					dock->size = { 400, 400 };
					EVENT("End Asset Explorer");
				}




			}
		}
	}

	virtual ~GraphRender()
	{

		if (task_future.valid())
			task_future.wait();
	}
	void on_resize(vec2 size) override
	{
		new_size = vec2::max(size, vec2{ 64,64 });
	}


	virtual void on_size_changed(const vec2& r) override
	{
		user_interface::on_size_changed(r);

	}
};

class RenderApplication : public Application
{

	friend class Singleton<Application>;


	std::shared_ptr<GraphRender> main_window;
	//std::shared_ptr<WindowRender> main_window;
#ifdef OCULUS_SUPPORT
	std::shared_ptr<OVRRender> ovr;
#endif
	// TODO:: shared_objects
	std::atomic<bool> need_reload_resource;
	std::atomic<bool> need_print_screen;

protected:
	RenderApplication()
	{
		THREAD_SCOPE(GUI);

		//	assert(ppp.inited);
		FileSystem::get().register_provider(std::make_shared<native_file_provider>());

		EVENT("Device");
		Graphics::Device::create();

		EVENT("PSO");
		init_signatures();
		PSOHolder::create();
		if (Device::get().is_rtx_supported())
			RTX::create();

		EVENT("AssetManager");
		AssetRenderer::create();
		AssetManager::create();
		EVENT("WindowRender");

		//	auto ps = Graphics::pixel_shader::get_resource({ "test.hlsl", "PS", 0,{}, false });
		//	auto cs = Graphics::compute_shader::get_resource({ "test.hlsl", "CS", 0,{}, false });

#ifdef OCULUS_SUPPORT
		//ovr = std::make_shared<OVRRender>();
#endif


		//	main_window = std::make_shared<WindowRender>();
		main_window = std::make_shared<GraphRender>();


		concurrency::create_task([this]() {

			if (main_window)main_window->render();

#ifdef OCULUS_SUPPORT
			if (ovr)ovr->render();
#endif

			});
		EVENT("good");
	}

	virtual ~RenderApplication()
	{
		shutdown();// really need this?

		main_window = nullptr;

		std::this_thread::sleep_for(100_ms);
		scheduler::reset();
		//


	//	
		//   Graphics::Device::get().get_queue(Graphics::CommandListType::DIRECT)->stop_all();
		Graphics::Device::get().stop_all();
		Skin::reset();
		Graphics::Texture::reset_manager();
		Graphics::pixel_shader::reset_manager();
		Graphics::vertex_shader::reset_manager();
		Graphics::domain_shader::reset_manager();
		Graphics::hull_shader::reset_manager();
		Graphics::geometry_shader::reset_manager();
		Graphics::compute_shader::reset_manager();
		GUI::Elements::FlowGraph::manager::reset();
		Profiler::reset();
		Graphics::GPUTimeManager::reset();
		///    main_window2 = nullptr;
		Fonts::FontSystem::reset();
		RTX::reset();
		AssetRenderer::reset();
		//Graphics::BufferCache::reset();
		TextureAssetRenderer::reset();
		AssetManager::reset();


		Graphics::PipelineLibrary::reset();

		Graphics::Device::reset();
		//   Graphics::Device::reset();



	}


	virtual void on_tick() override
	{
		if (GetAsyncKeyState(VK_ESCAPE))
			shutdown();

		Window::process_messages();
	}

	std::vector<std::string> file_open(const std::string& Name, const std::string& StartPath, const std::string& Extension) override
	{
		return Window::file_open(Name, StartPath, Extension);
	}

};

void SetupDebug()
{
	Log::create<WinErrorLogger>();
	FileTXTLogger::create();
	VSOutputLogger::create();
	//  ConsoleLogger::create();
	Log::get().set_logging_level(Log::LEVEL_WARNING);
	// Here we can disable some of notification types
	ClassLogger<resource_system>::get().set_logging_level(Log::LEVEL_ERROR);
	ClassLogger<Resource>::get().set_logging_level(Log::LEVEL_ERROR);
	ClassLogger<Graphics::Resource>::get().set_logging_level(Log::LEVEL_ERROR);
	Log::get() << Log::LEVEL_INFO << "info text" << Log::endl;
	Log::get() << Log::LEVEL_WARNING << "warning text" << Log::endl;
	Log::get() << Log::LEVEL_DEBUG << "debug text" << Log::endl;
	Log::get() << Log::LEVEL_ERROR << "error text" << Log::endl;
}

struct test
{
	D3D12_AUTO_BREADCRUMB_OP op = D3D12_AUTO_BREADCRUMB_OP_BUILDRAYTRACINGACCELERATIONSTRUCTURE;
	std::string str = "wtf";
	vec4 data = { 1,2,3,4 };

	std::vector<vec2> vec;
	test()
	{
		vec.emplace_back(1, 2);
		vec.emplace_back(3, 4);
		vec.emplace_back(5, 6);

	}
	template<class T = void>
	void foo() requires(false)
	{

	}
	SERIALIZE()
	{
		ar& NVP(op)& NVP(str)& NVP(data)& NVP(vec);
	}
} v;

//#include <DbgHelp.h>
int APIENTRY WinMain(_In_ HINSTANCE hinst,
	_In_opt_ HINSTANCE,
	_In_ LPTSTR,
	_In_ int)
{
	setlocale(LC_ALL, "");
	CoInitialize(NULL);



	//entry->SetCompressionStream();
	//	archive->WriteToStream();
//    FlowGraph::FlowSystem::get().register_node<FlowGraph::graph>("empty graph");
//   FlowGraph::FlowSystem::get().register_node<test_node>("test_node");
	// FlowGraph::FlowSystem::get().register_node<start_test_node>("start_test_node");
	// FlowGraph::FlowSystem::get().register_node<test_graph>("ololol");
//   FlowGraph::FlowSystem::get().register_node<TextureNode>("TextureNode");
	FlowGraph::FlowSystem::get().register_node<PowerNode>("PowerNode");
	FlowGraph::FlowSystem::get().register_node<SumNode>("SumNode");
	FlowGraph::FlowSystem::get().register_node<MulNode>("MulNode");
	FlowGraph::FlowSystem::get().register_node<SpecToMetNode>("SpecToMetNode");


	FlowGraph::FlowSystem::get().register_node("Scalar", []()->ScalarNode::ptr
		{
			auto res = std::make_shared<ScalarNode>(1.0f);
			res->name = "Scalar";
			return res;
		});



	FlowGraph::FlowSystem::get().register_node("ZeroColor", []()->VectorNode::ptr
		{
			auto res = std::make_shared<VectorNode>(float4{ 0,0,0,0 });
			res->name = "ZeroColor";
			return res;
		});



	//	FlowGraph::FlowSystem::get().register_node<ResultNode>("Material");
	//	FlowGraph::FlowSystem::get().register_node<MaterialGraph>("MaterialGraph");
	FlowGraph::FlowSystem::get().register_node<MaterialFunction>("MaterialFunction");
	FlowGraph::FlowSystem::get().register_node("file", []()-> FlowGraph::graph::ptr
		{
			auto f = FileSystem::get().get_file(to_path("graph.flg"));

			if (f)
				return Serializer::deserialize<FlowGraph::graph>(f->load_all());

			return nullptr;
		});

	auto result_code = 0;
	SetupDebug();
	auto a = []() {
		if constexpr (false)
		{
			v.foo();
		}
	};

	a();

	Log::get() << v << Log::endl;
	Log::get() << D3D12_AUTO_BREADCRUMB_OP_ATOMICCOPYBUFFERUINT << Log::endl;



	EVENT("start");
	Application::create<RenderApplication>();
	EVENT("create");




	// There can be error while creating, so test
	if (Application::is_good())
	{
		EVENT("run");
		Application::get().run(); // TODO: Create task and run engine (scheduling tasks)
		Application::reset();
	}

	else
		result_code = S_FALSE;

	CounterManager::get().print();
	EVENT("end");
	CoUninitialize();

	return result_code;
}
