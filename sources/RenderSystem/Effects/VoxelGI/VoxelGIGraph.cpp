#include "pch.h"


class GBufferDownsampler :public Events::prop_handler
{

public:
	using ptr = std::shared_ptr<GBufferDownsampler>;


	void generate(FrameGraph& graph)
	{

		struct DownsampleData
		{
			GBufferViewDesc gbuffer;

			ResourceHandler* temp;
		};

		auto size = graph.frame_size;

		graph.add_pass<DownsampleData>("GBufferDownsampler", [this, size](DownsampleData& data, TaskBuilder& builder) {

			data.gbuffer.need(builder, false, true);
			data.gbuffer.quality = builder.need_texture("GBuffer_Quality", ResourceFlags::DepthStencil);
			//	data.gbuffer.create_quality(size, builder);
			data.temp = data.gbuffer.create_temp_color(size, builder);


			}, [this, &graph](DownsampleData& data, FrameContext& _context) {

				auto& command_list = _context.get_list();
				auto tempColor = _context.get_texture(data.temp);
				auto gbuffer = data.gbuffer.actualize(_context);
				auto& graphics = command_list->get_graphics();

				graphics.set_signature(get_Signature(Layouts::DefaultLayout));

				Slots::FrameInfo::Compiled compiledFrame;
				{
					PROFILE(L"FrameInfo");
					Slots::FrameInfo frameInfo;
					auto camera = frameInfo.MapCamera();
					camera.cb = graph.cam->camera_cb.current;
					frameInfo.set(graphics);
				}

				graphics.set_topology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


				graphics.set_pipeline(GetPSO<PSOS::GBufferDownsample>());

				for (int i = 1; i < gbuffer.depth_mips.resource->get_desc().MipLevels; i++)
				{


					auto table = graphics.get_base().rtv_cpu.place(2);

					{
						Render::ResourceViewDesc subres;
						subres.type = Render::ResourceType::TEXTURE2D;

						subres.Texture2D.ArraySize = 1;
						subres.Texture2D.FirstArraySlice = 0;
						subres.Texture2D.MipLevels = 1;
						subres.Texture2D.MipSlice = i;
						subres.Texture2D.PlaneSlice = 0;

						auto depth_view = gbuffer.depth_mips.resource->create_view<Render::TextureView>(*graphics.get_base().frame_resources, subres);
						auto normal_view = gbuffer.normals.resource->create_view<Render::TextureView>(*graphics.get_base().frame_resources, subres);

						table[0].place(depth_view.get_rtv(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
						table[1].place(normal_view.get_rtv(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV);


						graphics.set_viewport(depth_view.get_viewport());
						graphics.set_scissor(depth_view.get_scissor());



					}

					graphics.set_rtv(table, Render::Handle());

					Slots::GBufferDownsample downsample;



					{
						ResourceViewDesc subres;
						subres.type = Render::ResourceType::TEXTURE2D;

						subres.Texture2D.ArraySize = 1;
						subres.Texture2D.FirstArraySlice = 0;
						subres.Texture2D.MipLevels = 1;
						subres.Texture2D.MipSlice = i - 1;
						subres.Texture2D.PlaneSlice = 0;
						downsample.GetDepth() = gbuffer.depth_mips.resource->create_view<Render::TextureView>(*graphics.get_base().frame_resources, subres).texture2D;
						downsample.GetNormals() = gbuffer.normals.resource->create_view<Render::TextureView>(*graphics.get_base().frame_resources, subres).texture2D;
					}
					downsample.set(graphics);
					graphics.draw(4);
				}

				MipMapGenerator::get().generate_quality(graphics, nullptr, gbuffer, tempColor);



			});

	}


};



VoxelGI::VoxelGI(Scene::ptr& scene) :scene(scene)
{
	scene->on_element_add.register_handler(this, [this](scene_object* object) {
		auto render_object = dynamic_cast<MeshAssetInstance*>(object);

		if (!render_object) return;

		if (render_object->type == MESH_TYPE::STATIC)
			need_start_new = true;
		});


	{
		dispatch_command = Render::IndirectCommand::create_command<DispatchArguments>(sizeof(Underlying<command>));
	}
	{
		CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex3D(DXGI_FORMAT_R8G8B8A8_UNORM, 512, 512, 512, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE);

		albedo.set(desc);
		normal.set(desc);

		desc.MipLevels = 6;
		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		tex_lighting.set(desc);

		lighed_to_albedo_coeff = tex_lighting.tex_result->get_tiles_count() / albedo.tex_result->get_tiles_count();
		dynamic_generator_lighted = TileDynamicGenerator(tex_lighting.tex_result->get_tiles_count());
		dynamic_generator_voxelizing = TileDynamicGenerator(albedo.tex_result->get_tiles_count());
		visibility = std::make_shared<VisibilityBufferUniversal>(tex_lighting.tex_result->get_tiles_count());

		tex_lighting.tex_result->on_load = [this](ivec4 pos)
		{
			if (gpu_tiles_buffer[pos.w])
			{
				gpu_tiles_buffer[pos.w]->insert(pos);
			}

		};

		tex_lighting.tex_result->on_zero = [this](ivec4 pos)
		{
			if (gpu_tiles_buffer[pos.w])
			{
				gpu_tiles_buffer[pos.w]->erase(pos);
			}
		};
	

		albedo.tex_dynamic->on_load = [this](ivec4 pos)
		{
			albedo_tiles->insert(pos);
		};

		albedo.tex_dynamic->on_zero = [this](ivec4 pos)
		{
			albedo_tiles->erase(pos);
		};
		albedo_tiles.reset(new GPUTilesBuffer);
		albedo_tiles->set_size(albedo.tex_result->get_tiles_count(), albedo.tex_result->get_tile_shape());


		gpu_tiles_buffer.resize(tex_lighting.tex_result->unpacked_mip_count);

		for (int i = 0; i < gpu_tiles_buffer.size(); i++)
		{
			gpu_tiles_buffer[i].reset(new GPUTilesBuffer);
			gpu_tiles_buffer[i]->set_size(tex_lighting.tex_result->get_tiles_count(i), tex_lighting.tex_result->get_tile_shape());
		}
	

	}

	init_states();

}




void VoxelGI::init_states()
{


	gi_rtv = Render::DescriptorHeapManager::get().get_rt()->create_table(2);

}

void VoxelGI::start_new(Render::CommandList& list)
{

	all_scene_regen_counter = 2;
	tex_lighting.zero_tiles(list);

	albedo.zero_tiles(list);
	normal.zero_tiles(list);


	dynamic_generator_lighted.remove_all();
	dynamic_generator_voxelizing.remove_all();

}


void VoxelGI::voxelize(MeshRenderContext::ptr& context, main_renderer* r)
{
	auto& graphics = context->list->get_graphics();
	auto& compute = context->list->get_compute();
	auto& list = *context->list;


	static bool prev = 0;
	bool cur = !!GetAsyncKeyState('P');

	if (!cur && prev)
	{
		start_new(list);
		//all_scene_regen_counter++;
	}

	if (all_scene_regen_counter > 0)
	{
		if (vis_update.valid())
		{
			auto updates = vis_update.get();


			for (auto& pos : updates.tiles_to_load)
			{
				ivec3 mesh_pos = pos / lighed_to_albedo_coeff;
				albedo.tex_static->load_tiles(&list, mesh_pos, mesh_pos);
				normal.tex_static->load_tiles(&list, mesh_pos, mesh_pos);
		
			}

			tex_lighting.load_static(updates.tiles_to_load);
		}
	}

	prev = cur;
	if (!all_scene_regen_counter) {
		dynamic_generator_lighted.begin(scene->voxel_info.GetMin(), scene->voxel_info.GetMin() + scene->voxel_info.GetSize());
		dynamic_generator_voxelizing.begin(scene->voxel_info.GetMin(), scene->voxel_info.GetMin() + scene->voxel_info.GetSize());

		scene->iterate_meshes(MESH_TYPE::DYNAMIC, [this](scene_object::ptr obj) {

			MeshAssetInstance* mesh = dynamic_cast<MeshAssetInstance*>(obj.get());

			if (mesh)
			{
				for (auto elem : mesh->rendering)
				{
					auto min = elem.primitive_global->get_min();
					auto max = elem.primitive_global->get_max();

					dynamic_generator_lighted.add(min, max);
					dynamic_generator_voxelizing.add(min, max);
				}
			}

			}
		);
		dynamic_generator_lighted.end();
		dynamic_generator_voxelizing.end();
	}

	albedo.tex_dynamic->zero_tiles(&list, dynamic_generator_voxelizing.tiles_to_remove);
	normal.tex_dynamic->zero_tiles(&list, dynamic_generator_voxelizing.tiles_to_remove);
	tex_lighting.zero_dynamic(dynamic_generator_lighted.tiles_to_remove);


	albedo.tex_dynamic->load_tiles(&list, dynamic_generator_voxelizing.tiles_to_load);
	normal.tex_dynamic->load_tiles(&list, dynamic_generator_voxelizing.tiles_to_load);


	tex_lighting.load_dynamic(dynamic_generator_lighted.tiles_to_load);


	PROFILE_GPU(L"voxelizing");

	//list.clear_uav(tiled_volume_albedo_dynamic, tiled_volume_albedo_dynamic->texture_3d()->get_static_uav());

	if (clear_scene && all_scene_regen_counter)
	{
		PROFILE_GPU(L"clear");
		list.clear_uav(albedo.tex_static, albedo.tex_static->texture_3d()->get_static_uav());
	}
	else
	{
		PROFILE_GPU(L"copy");

		if (GetAsyncKeyState('4'))
		{
			context->list->get_copy().copy_texture(albedo.tex_dynamic, 0, albedo.tex_static, 0); // aaaa my fps!
			context->list->get_copy().copy_texture(normal.tex_dynamic, 0, normal.tex_static, 0);	// optimize this! 

		}else if (GetAsyncKeyState('5'))
	
		for (auto p : albedo_tiles->used_tiles)
		{
			ivec3 pos = p * normal.tex_result->get_tile_shape();

			context->list->get_copy().copy_texture(albedo.tex_dynamic, pos, albedo.tex_static, pos, albedo.tex_result->get_tile_shape()); // aaaa my fps!
			context->list->get_copy().copy_texture(normal.tex_dynamic, pos, normal.tex_static, pos, normal.tex_result->get_tile_shape());	// optimize this! 
		}
		else  // that's what i'm talking about
		{
			albedo_tiles->update(context->list);
			compute.set_pipeline(GetPSO<PSOS::VoxelCopy>());
			scene->voxels_compiled.set(compute);

			{

				{
					Slots::VoxelCopy utils;
					utils.GetTarget()[0] = albedo.tex_dynamic->texture_3d()->rwTexture3D[0];
					utils.GetSource()[0] = albedo.tex_static->texture_3d()->texture3D;

					utils.GetTarget()[1] = normal.tex_dynamic->texture_3d()->rwTexture3D[0];
					utils.GetSource()[1] = normal.tex_static->texture_3d()->texture3D;

					auto params = utils.MapParams();
					params.GetTiles() = albedo_tiles->buffer->structuredBuffer;
					params.GetVoxels_per_tile() = normal.tex_result->get_tile_shape();
					utils.set(compute);
				}

				compute.execute_indirect(
					dispatch_command,
					1,
					albedo_tiles->dispatch_buffer.get());

			}

		}
	}


	context->render_type = RENDER_TYPE::VOXEL;
	
	Slots::Voxelization voxelization;
	voxelization.MapInfo().GetMin() = scene->voxel_info.GetMin();
	voxelization.MapInfo().GetSize() = scene->voxel_info.GetSize();
	voxelization.MapInfo().GetVoxel_tiles_count() = scene->voxel_info.GetVoxel_tiles_count();
	voxelization.MapInfo().GetVoxels_per_tile() = scene->voxel_info.GetVoxels_per_tile();

	voxelization.GetVisibility() = visibility->buffer->create_view<Render::TextureView>(*list.frame_resources).rwTexture3D;


	if (all_scene_regen_counter)
	{
		context->render_mesh = MESH_TYPE::STATIC;
		voxelization.GetAlbedo() = albedo.tex_static->texture_3d()->rwTexture3D[0];
		voxelization.GetNormals() = normal.tex_static->texture_3d()->rwTexture3D[0];
	}
	else
	{
		context->render_mesh = MESH_TYPE::DYNAMIC;

		voxelization.GetAlbedo() = albedo.tex_dynamic->texture_3d()->rwTexture3D[0];
		voxelization.GetNormals() = normal.tex_dynamic->texture_3d()->rwTexture3D[0];
	}

	albedo.flush(list);
	normal.flush(list);
	tex_lighting.flush(list);

	context->voxelization_compiled = voxelization.compile(*list.frame_resources);
	context->pipeline.rtv.rtv_formats.clear();
	context->pipeline.rtv.enable_depth = false;
	context->pipeline.rtv.enable_depth_write = false;
	context->pipeline.rtv.ds_format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;

	graphics.set_viewport({ 0, 0,  albedo.tex_dynamic->get_size().xy });
	graphics.set_scissor({ 0, 0,  albedo.tex_dynamic->get_size().xy });
	graphics.set_rtv(0, Render::Handle(), Render::Handle());


	{
		PROFILE_GPU(L"render");
		r->render(context, scene);
	}


	if (all_scene_regen_counter)
	{
		PROFILE_GPU(L"visibility update");
		vis_update = visibility->update(context->list);
	}

	for (auto&& b : gpu_tiles_buffer)
		if (b)
			b->update(context->list);

	if (all_scene_regen_counter > 0)
		all_scene_regen_counter--;
}



void VoxelGI::debug(FrameGraph& graph)
{
	struct VoxelDebug
	{
		GBufferViewDesc gbuffer;
		ResourceHandler* target_tex;
		ResourceHandler* voxel_lighted;

	};

	auto size = graph.frame_size;

	graph.add_pass<VoxelDebug>("VoxelDebug", [this, size](VoxelDebug& data, TaskBuilder& builder) {

		data.target_tex = builder.create_texture("VoxelDebug", size, 1, DXGI_FORMAT_R16G16B16A16_FLOAT, ResourceFlags::RenderTarget);
		data.voxel_lighted = builder.need_texture("voxel_lighted", ResourceFlags::ComputeRead);

		data.gbuffer.need(builder, true);

		}, [this, &graph](VoxelDebug& data, FrameContext& _context) {

			auto& command_list = _context.get_list();

			auto voxel_lighted = _context.get_texture(data.voxel_lighted);

			MeshRenderContext::ptr context(new MeshRenderContext());
			auto target_tex = _context.get_texture(data.target_tex);
			auto gbuffer = data.gbuffer.actualize(_context);

			context->current_time = 0;
			//		context->sky_dir = lighting->tex_lighting.pssm.get_position();
			context->priority = TaskPriority::HIGH;
			context->list = command_list;
			//	context->eye_context = vr_context;

			context->cam = graph.cam;

			auto scene = graph.scene;
			auto renderer = graph.renderer;
			context->begin();

			auto& graphics = context->list->get_graphics();

			graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			graphics.set_viewport(target_tex.get_viewport());
			graphics.set_scissor(target_tex.get_scissor());

			graphics.set_rtv(1, target_tex.get_rtv(), Render::Handle());
			graphics.set_pipeline(GetPSO<PSOS::VoxelDebug>());


			{
				Slots::FrameInfo frameInfo;

				auto camera = frameInfo.MapCamera();
				camera.cb = graph.cam->camera_cb.current;
				frameInfo.set(graphics);
			}


			scene->voxels_compiled.set(graphics);

			{
				Slots::VoxelDebug debug;
				debug.GetVolume() = voxel_lighted.texture3D;
				gbuffer.SetTable(debug.MapGbuffer());
				debug.set(graphics);

			}
			graphics.draw(4);

			//screen_reflection(context, scene);
		});
}



void VoxelGI::screen(FrameGraph& graph)
{
	struct Screen
	{
		GBufferViewDesc gbuffer;
		ResourceHandler* target_tex;

		ResourceHandler* downsampled_light0;
		ResourceHandler* downsampled_light1;


		ResourceHandler* gi_static0;
		ResourceHandler* gi_static1;

		ResourceHandler* sky_cubemap_filtered;

		ResourceHandler* voxel_lighted;

	};
	auto size = graph.frame_size;

	graph.add_pass<Screen>("VoxelScreen", [this, size](Screen& data, TaskBuilder& builder) {

		data.target_tex = builder.need_texture("ResultTexture", ResourceFlags::RenderTarget);

		data.gbuffer.need(builder, true);
		data.downsampled_light0 = builder.create_texture("downsampled_light0", ivec2(size.x / 2, size.y / 2), 1, DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, ResourceFlags::RenderTarget);
		data.downsampled_light1 = builder.create_texture("downsampled_light1", ivec2(size.x / 2, size.y / 2), 1, DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, ResourceFlags::RenderTarget);

		data.gi_static0 = builder.create_texture("gi_static0", ivec2(size.x, size.y), 1, DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, ResourceFlags::RenderTarget | ResourceFlags::Static);
		data.gi_static1 = builder.create_texture("gi_static1", ivec2(size.x, size.y), 1, DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, ResourceFlags::RenderTarget | ResourceFlags::Static);
		data.sky_cubemap_filtered = builder.need_texture("sky_cubemap_filtered", ResourceFlags::PixelRead);

		data.voxel_lighted = builder.need_texture("voxel_lighted", ResourceFlags::ComputeRead);

		}, [this, &graph](Screen& data, FrameContext& _context) {

			auto& command_list = _context.get_list();


			MeshRenderContext::ptr context(new MeshRenderContext());
			auto target_tex = _context.get_texture(data.target_tex);
			auto gbuffer = data.gbuffer.actualize(_context);
			auto sky_cubemap_filtered = _context.get_texture(data.sky_cubemap_filtered);
			Render::TextureView views[] = { _context.get_texture(data.downsampled_light0), _context.get_texture(data.downsampled_light1) };
			Render::TextureView gi_views[] = { _context.get_texture(data.gi_static0), _context.get_texture(data.gi_static1) };

			if (data.gi_static0->is_new())
			{
				command_list->clear_rtv(gi_views[0].get_rtv(), vec4(0, 0, 0, 0));
				command_list->clear_rtv(gi_views[1].get_rtv(), vec4(0, 0, 0, 0));
			}

			context->current_time = 0;
			context->priority = TaskPriority::HIGH;
			context->list = command_list;

			context->cam = graph.cam;

			auto scene = graph.scene;
			auto renderer = graph.renderer;
			context->begin();

			auto& graphics = context->list->get_graphics();


			graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			graphics.set_signature(get_Signature(Layouts::DefaultLayout));



			{
				Slots::FrameInfo frameInfo;


				frameInfo.MapCamera().cb = graph.cam->camera_cb.current;
				frameInfo.MapPrevCamera().cb = graph.cam->camera_cb.prev;

				frameInfo.GetBrdf() = EngineAssets::brdf.get_asset()->get_texture()->texture_3d()->texture3D;
				frameInfo.GetTime() = graph.time;
				frameInfo.set(graphics);
			}

			{
				Slots::VoxelScreen voxelScreen;
				gbuffer.SetTable(voxelScreen.MapGbuffer());
				voxelScreen.GetVoxels() = tex_lighting.tex_result->texture_3d()->texture3D;
				voxelScreen.GetTex_cube() = sky_cubemap_filtered.textureÑube;
				voxelScreen.set(graphics);
			}
			scene->voxels_compiled.set(graphics);



			{
				graphics.set_pipeline(GetPSO<PSOS::VoxelIndirectLow>());
				PROFILE_GPU(L"downsampled");
				graphics.set_viewport(views[0].get_viewport());
				graphics.set_scissor(views[0].get_scissor());
				graphics.set_rtv(1, views[0].get_rtv(), Render::Handle());

				graphics.draw(4);
			}


			{
				PROFILE_GPU(L"filter");

				graphics.set_pipeline(GetPSO<PSOS::VoxelIndirectFilter>());

				for (int i = 0; i < 1; i++)
				{
					Render::TextureView& cur = views[(i + 1) % 2];
					Render::TextureView& prev = views[i % 2];

					{
						Slots::VoxelBlur voxelBlur;
						voxelBlur.GetTex_color() = prev.texture2D;
						voxelBlur.set(graphics);
					}

					graphics.set_rtv(1, cur.get_rtv(), Render::Handle());

					graphics.draw(4);
				}

			}




			{
				PROFILE_GPU(L"main");

				{
					Slots::VoxelUpscale  voxelUpscale;
					voxelUpscale.GetTex_downsampled() = views[1].texture2D;
					voxelUpscale.GetTex_gi_prev() = gi_views[1 - gi_index].texture2D;
					voxelUpscale.GetTex_depth_prev() = gbuffer.depth_prev_mips.texture2D;

					voxelUpscale.set(graphics);
				}


				graphics.set_viewport(target_tex.get_viewport());
				graphics.set_scissor(target_tex.get_scissor());

				gi_rtv[0].place(target_tex.get_rtv(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
				gi_rtv[1].place(gi_views[gi_index].get_rtv(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

				graphics.set_rtv(gi_rtv, gbuffer.quality.get_dsv());

				gi_index = 1 - gi_index;

				{
					PROFILE_GPU(L"full");
					context->list->get_native_list()->OMSetStencilRef(1);
					graphics.set_pipeline(GetPSO<PSOS::VoxelIndirectHi>());
					graphics.draw(4);
				}


				{
					PROFILE_GPU(L"resize");

					context->list->get_native_list()->OMSetStencilRef(0);
					graphics.set_pipeline(GetPSO<PSOS::VoxelIndirectUpsample>());
					graphics.draw(4);
				}


			}



		});
}



void VoxelGI::screen_reflection(FrameGraph& graph)
{
	struct ScreenReflection
	{
		GBufferViewDesc gbuffer;
		ResourceHandler* target_tex;

		ResourceHandler* downsampled_reflection;
		ResourceHandler* sky_cubemap_filtered;
	};
	auto size = graph.frame_size;

	graph.add_pass<ScreenReflection>("ScreenReflection", [this, size](ScreenReflection& data, TaskBuilder& builder) {

		data.target_tex = builder.need_texture("ResultTexture", ResourceFlags::RenderTarget);

		data.gbuffer.need(builder, true);
		data.downsampled_reflection = builder.create_texture("downsampled_reflection", ivec2(size.x / 2, size.y / 2), 1, DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, ResourceFlags::RenderTarget);
		data.sky_cubemap_filtered = builder.need_texture("sky_cubemap_filtered", ResourceFlags::PixelRead);
		}, [this, &graph](ScreenReflection& data, FrameContext& _context) {

			auto& command_list = _context.get_list();


			MeshRenderContext::ptr context(new MeshRenderContext());
			auto target_tex = _context.get_texture(data.target_tex);
			auto gbuffer = data.gbuffer.actualize(_context);
			auto sky_cubemap_filtered = _context.get_texture(data.sky_cubemap_filtered);

			Render::TextureView downsampled_reflection = _context.get_texture(data.downsampled_reflection);



			context->current_time = 0;
			context->priority = TaskPriority::HIGH;
			context->list = command_list;

			context->cam = graph.cam;

			auto scene = graph.scene;
			auto renderer = graph.renderer;
			context->begin();

			auto& graphics = context->list->get_graphics();


			graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			graphics.set_signature(get_Signature(Layouts::DefaultLayout));



			{
				Slots::FrameInfo frameInfo;


				frameInfo.MapCamera().cb = graph.cam->camera_cb.current;
				frameInfo.MapPrevCamera().cb = graph.cam->camera_cb.prev;

				frameInfo.GetBrdf() = EngineAssets::brdf.get_asset()->get_texture()->texture_3d()->texture3D;
				frameInfo.GetTime() = graph.time;
				frameInfo.set(graphics);
			}

			{
				Slots::VoxelScreen voxelScreen;
				gbuffer.SetTable(voxelScreen.MapGbuffer());
				voxelScreen.GetVoxels() = tex_lighting.tex_result->texture_3d()->texture3D;
				voxelScreen.GetTex_cube() = sky_cubemap_filtered.textureÑube;
				voxelScreen.set(graphics);
			}
			scene->voxels_compiled.set(graphics);



			{
				graphics.set_pipeline(GetPSO<PSOS::VoxelReflectionLow>());
				PROFILE_GPU(L"downsampled");
				graphics.set_viewport(downsampled_reflection.get_viewport());
				graphics.set_scissor(downsampled_reflection.get_scissor());
				graphics.set_rtv(1, downsampled_reflection.get_rtv(), Render::Handle());

				graphics.draw(4);
			}





			{
				PROFILE_GPU(L"main");

				{
					Slots::VoxelUpscale  voxelUpscale;
					voxelUpscale.GetTex_downsampled() = downsampled_reflection.texture2D;
					voxelUpscale.GetTex_depth_prev() = gbuffer.depth_prev_mips.texture2D;

					voxelUpscale.set(graphics);
				}


				graphics.set_viewport(target_tex.get_viewport());
				graphics.set_scissor(target_tex.get_scissor());

				graphics.set_rtv(1, target_tex.get_rtv(), gbuffer.quality.get_dsv());



				{
					PROFILE_GPU(L"full");
					context->list->get_native_list()->OMSetStencilRef(2);
					graphics.set_pipeline(GetPSO<PSOS::VoxelReflectionHi>());
					graphics.draw(4);
				}


				{
					PROFILE_GPU(L"resize");

					context->list->get_native_list()->OMSetStencilRef(0);
					graphics.set_pipeline(GetPSO<PSOS::VoxelReflectionUpsample>());
					graphics.draw(4);
				}


			}



		});
}


void VoxelGI::voxelize(FrameGraph& graph)
{
	struct Voxelize
	{
		ResourceHandler* voxel_albedo;
		ResourceHandler* voxel_normal;
	};

	graph.add_pass<Voxelize>("voxelize", [this](Voxelize& data, TaskBuilder& builder) {

		data.voxel_albedo = builder.need_texture("voxel_albedo", ResourceFlags::UnorderedAccess);
		data.voxel_normal = builder.need_texture("voxel_normal", ResourceFlags::UnorderedAccess);

		}, [this, &graph](Voxelize& data, FrameContext& _context) {
			auto& command_list = _context.get_list();

			if (need_start_new)
			{
				start_new(*command_list);
				need_start_new = false;
			}


			MeshRenderContext::ptr context(new MeshRenderContext());
			context->current_time = 0;
			context->priority = TaskPriority::HIGH;
			context->list = command_list;
			context->cam = graph.cam;

			auto scene = graph.scene;
			auto renderer = graph.renderer;
			context->begin();




			voxelize(context, renderer);
		});
}

void VoxelGI::lighting(FrameGraph& graph)
{


	struct Lighting
	{
		ResourceHandler* global_depth;
		ResourceHandler* global_camera;

		ResourceHandler* sky_cubemap_filtered;
		ResourceHandler* voxel_lighted;
		ResourceHandler* voxel_albedo;
		ResourceHandler* voxel_normal;
	};


	graph.add_pass<Lighting>("Lighting", [this](Lighting& data, TaskBuilder& builder) {

		data.global_depth = builder.need_texture("global_depth", ResourceFlags::ComputeRead);
		data.global_camera = builder.need_buffer("global_camera", ResourceFlags::ComputeRead);
		data.sky_cubemap_filtered = builder.need_texture("sky_cubemap_filtered", ResourceFlags::PixelRead);

		data.voxel_lighted = builder.need_texture("voxel_lighted", ResourceFlags::UnorderedAccess);
		data.voxel_albedo = builder.need_texture("voxel_albedo", ResourceFlags::ComputeRead);
		data.voxel_normal = builder.need_texture("voxel_normal", ResourceFlags::ComputeRead);


		}, [this, &graph](Lighting& data, FrameContext& _context) {

			auto& command_list = _context.get_list();

			auto global_depth = _context.get_texture(data.global_depth);
			auto global_camera = _context.get_buffer(data.global_camera);
			auto sky_cubemap_filtered = _context.get_texture(data.sky_cubemap_filtered);

			MeshRenderContext::ptr context(new MeshRenderContext());
			context->current_time = 0;
			context->priority = TaskPriority::HIGH;
			context->list = command_list;
			context->cam = graph.cam;



			auto& list = *context->list;
			auto& compute = context->list->get_compute();

			compute.set_pipeline(GetPSO<PSOS::Lighting>(PSOS::Lighting::SecondBounce.Use(!GetAsyncKeyState('G'))));

			//todo: remove after size is indirect
	//		list.clear_uav(tex_lighting.tex_result, tex_lighting.tex_result->texture_3d()->get_static_uav());

			Slots::VoxelLighting ligthing;
			{

				ligthing.GetAlbedo() = albedo.tex_result->texture_3d()->texture3D;
				ligthing.GetNormals() = normal.tex_result->texture_3d()->texture3D;
				ligthing.GetOutput() = tex_lighting.tex_result->texture_3d()->rwTexture3D[0];
				ligthing.GetTex_cube() = sky_cubemap_filtered.textureÑube;
				Render::ResourceViewDesc subres;
				subres.type = Render::ResourceType::TEXTURE3D;

				subres.Texture2D.ArraySize = 1;
				subres.Texture2D.FirstArraySlice = 0;
				subres.Texture2D.MipLevels = tex_lighting.tex_result->get_desc().MipLevels - 1;
				subres.Texture2D.MipSlice = 1;
				subres.Texture2D.PlaneSlice = 0;

				ligthing.GetLower() = tex_lighting.tex_result->create_view<TextureView>(*graph.builder.current_frame, subres).texture3D;


				auto params = ligthing.MapParams();
				params.GetTiles() = gpu_tiles_buffer[0]->buffer->structuredBuffer;
				params.GetVoxels_per_tile() = tex_lighting.tex_result->get_tile_shape();

				auto pssm = ligthing.MapPssmGlobal();

				pssm.GetLight_buffer() = global_depth.texture2D;

				auto buffer_view = global_camera.resource->create_view<StructuredBufferView<Table::Camera>>(*graph.builder.current_frame);

				pssm.GetLight_camera() = buffer_view.structuredBuffer;


				ligthing.set(compute);

			}
			graph.scene->voxels_compiled.set(compute);

			// make indirect as gpu_tiles_buffer->size is CPU multithread
		//	compute.dispach(ivec3(gpu_tiles_buffer[0]->size() * ligthing.GetGroupCount(), 1, 1), ivec3(1, 1, 1));
			compute.execute_indirect(
				dispatch_command,
				1,
				gpu_tiles_buffer[0]->dispatch_buffer.get());

		}, PassFlags::Compute);

}


void VoxelGI::mipmapping(FrameGraph& graph)
{
	struct Mipmapping
	{
		ResourceHandler* voxel_lighted;

	};

	graph.add_pass<Mipmapping>("Mipmapping", [this](Mipmapping& data, TaskBuilder& builder) {

		data.voxel_lighted = builder.need_texture("voxel_lighted", ResourceFlags::UnorderedAccess);

		}, [this, &graph](Mipmapping& data, FrameContext& _context) {

			auto& command_list = _context.get_list();

			auto voxel_lighted = _context.get_texture(data.voxel_lighted);

			MeshRenderContext::ptr context(new MeshRenderContext());

			context->current_time = 0;
			//		context->sky_dir = lighting->tex_lighting.pssm.get_position();
			context->priority = TaskPriority::HIGH;
			context->list = command_list;
			//	context->eye_context = vr_context;

			context->cam = graph.cam;



			auto& list = *context->list;
			auto& compute = context->list->get_compute();




		


			compute.set_signature(get_Signature(Layouts::DefaultLayout));


			graph.scene->voxels_compiled.set(compute);


			compute.set_pipeline(GetPSO<PSOS::VoxelZero>());
			{
				PROFILE_GPU(L"ZERO");
				for (int i = 1; i < tex_lighting.tex_result->get_desc().MipLevels; i++)
				{
					if (i >= gpu_tiles_buffer.size() || !gpu_tiles_buffer[i])
					{
						list.clear_uav(tex_lighting.tex_result, tex_lighting.tex_result->texture_3d()->rwTexture3D[i], vec4(0, 0, 0, 0));
						continue;
					}

					{
						Slots::VoxelZero utils;
						utils.GetTarget() = tex_lighting.tex_result->texture_3d()->rwTexture3D[i];

						auto params = utils.MapParams();
						params.GetTiles() = gpu_tiles_buffer[i]->buffer->structuredBuffer;
						params.GetVoxels_per_tile() = tex_lighting.tex_result->get_tile_shape();

						utils.set(compute);
					}

					compute.execute_indirect(
						dispatch_command,
						1,
						gpu_tiles_buffer[i]->dispatch_buffer.get());

				}
			}

			{
				PROFILE_GPU(L"EXEC");
				unsigned int mip_count = 1;

				while (mip_count < tex_lighting.tex_result->get_desc().MipLevels)
				{

					if (!gpu_tiles_buffer[mip_count]) break;
					unsigned int current_mips = std::min(3u, tex_lighting.tex_result->get_desc().MipLevels - mip_count);
					compute.set_pipeline(GetPSO<PSOS::VoxelDownsample>(PSOS::VoxelDownsample::Count(current_mips)));

					{
					

						Slots::VoxelMipMap mipmapping;
						mipmapping.GetSrcMip() = tex_lighting.tex_result->texture_3d()->texture3DMips[mip_count - 1];

						for (unsigned int i = 0; i < current_mips; i++)
						{
							mipmapping.GetOutMips()[i] = tex_lighting.tex_result->texture_3d()->rwTexture3D[mip_count + i];
							//	list.clear_uav(tex_lighting.tex_result, tex_lighting.tex_result->texture_3d()->rwTexture3D[mip_count + i], vec4(0, 0, 0, 0));
						}

						auto params = mipmapping.MapParams();
						params.GetTiles() = gpu_tiles_buffer[mip_count]->buffer->structuredBuffer;
						params.GetVoxels_per_tile() = tex_lighting.tex_result->get_tile_shape();

						mipmapping.set(compute);
					}
					PROFILE_GPU(L"exec");
					compute.execute_indirect(
						dispatch_command,
						1,
						gpu_tiles_buffer[mip_count]->dispatch_buffer.get());

					mip_count += current_mips;
				}
			}
		}, PassFlags::Compute);

}
void VoxelGI::generate(FrameGraph& graph)
{

	graph.builder.pass_texture("voxel_albedo", albedo.tex_result);
	graph.builder.pass_texture("voxel_normal", normal.tex_result);
	graph.builder.pass_texture("voxel_lighted", tex_lighting.tex_result);

	Slots::VoxelInfo& voxel_info = scene->voxel_info;

	//if (all_scene_regen_counter)
	{
		min = scene->get_min() - float3(1, 1, 1);
		size = scene->get_max() + float3(1, 1, 1) - scene->get_min();
	}

	voxel_info.GetMin() = min;
	voxel_info.GetSize() = size;
	voxel_info.GetSize().x = voxel_info.GetSize().y = voxel_info.GetSize().z = max(200.0f, voxel_info.GetSize().max_element());

	voxel_info.GetVoxel_tiles_count() = tex_lighting.tex_result->get_tiles_count(0);
	voxel_info.GetVoxels_per_tile() = tex_lighting.tex_result->get_tile_shape();

	scene->voxels_compiled = scene->voxel_info.compile(*graph.builder.current_frame);

	if (voxelize_scene) voxelize(graph);

	light_counter = (light_counter + 1) % 5;

	if (light_scene/* && light_counter == 0 || all_scene_regen_counter > 0*/)
	{
		//if (gpu_tiles_buffer[0]->size())
		{
			lighting(graph);
			mipmapping(graph);
		}
	}



	if (!downsampler) downsampler = std::make_shared<GBufferDownsampler>();
	downsampler->generate(graph);

	screen(graph);
	//screen_reflection(graph);



	debug(graph);

}