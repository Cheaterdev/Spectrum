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
					auto timer = Profiler::get().start(L"FrameInfo");
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
					downsample.GetDepth() = gbuffer.depth_mips.resource->create_view<Render::TextureView>(*graphics.get_base().frame_resources, subres).get_srv();
					downsample.GetNormals() = gbuffer.normals.resource->create_view<Render::TextureView>(*graphics.get_base().frame_resources, subres).get_srv();
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
		CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex3D(DXGI_FORMAT_R8G8B8A8_UNORM, 512, 512, 512, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE);

		tiled_volume_albedo.reset(new Texture3DTiledDynamic(desc));
		tiled_volume_normal.reset(new Texture3DTiledDynamic(desc));

		tiled_volume_albedo_static.reset(new Texture3DTiledDynamic(desc));
		tiled_volume_normal_static.reset(new Texture3DTiledDynamic(desc));

		desc.MipLevels = 9;
		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		tiled_volume_lighted.reset(new Texture3DTiledDynamic(desc));

		lighed_to_albedo_coeff = tiled_volume_lighted->get_tiles_count() / tiled_volume_albedo->get_tiles_count();
		dynamic_generator = TileDynamicGenerator(tiled_volume_lighted->get_tiles_count());
		dynamic_generator_voxelizing = TileDynamicGenerator(tiled_volume_albedo->get_tiles_count());
		visibility = tiled_volume_lighted->create_visibility();

		tiled_volume_lighted->on_tile_load = [this](Tile::ptr& tile)
		{

			if (gpu_tiles_buffer[tile->mip_level])
			{
				gpu_tiles_buffer[tile->mip_level]->insert(tile->position);
			}

			tiled_volume_lighted->create_tile(tile->position / 2, tile->mip_level + 1);
		};
		tiled_volume_lighted->on_tile_remove = [this](Tile::ptr& tile)
		{

			if (gpu_tiles_buffer[tile->mip_level])
			{
				gpu_tiles_buffer[tile->mip_level]->erase(tile->position);
			}

			//	tiled_volume_lighted->create_tile(tile->position / 2, tile->mip_level + 1);
		};

		visibility->on_process = [this](ivec3 pos, int mip) {

			if (mip == 0)
			{

				auto& tile1 = tiled_volume_albedo->create_static_tile(pos / lighed_to_albedo_coeff, mip);
				auto& tile2 = tiled_volume_normal->create_static_tile(pos / lighed_to_albedo_coeff, mip);
				auto& tile3 = tiled_volume_lighted->create_static_tile(pos, mip);
				//	tile->last_visible = std::chrono::system_clock::now().time_since_epoch().count();
			}
			else
			{
				/*	auto &tile = tiled_volume_lighted->get_tile(0,pos);
				if (tile&&	tile->last_visible <std::chrono::system_clock::now().time_since_epoch().count()-5000)
				{
				remove_static_tiles(tile->position);
				}
				*/
			}
		};


		//	tiled_volume_lighted->load_all();


		volume_albedo = tiled_volume_albedo->texture;
		volume_normal = tiled_volume_normal->texture;
		volume_lighted = tiled_volume_lighted->texture;

		gpu_tiles_buffer.resize(desc.MipLevels);

		gpu_tiles_buffer[0].reset(new GPUTilesBuffer);
		gpu_tiles_buffer[1].reset(new GPUTilesBuffer);
		gpu_tiles_buffer[4].reset(new GPUTilesBuffer);
	
		gpu_tiles_buffer[0]->set_size(tiled_volume_lighted->get_max_tiles_count());
		gpu_tiles_buffer[1]->set_size(tiled_volume_lighted->get_max_tiles_count(1));
		gpu_tiles_buffer[4]->set_size(tiled_volume_lighted->get_max_tiles_count(4));
	
	}

	init_states();

}




void VoxelGI::init_states()
{
	

	gi_rtv = Render::DescriptorHeapManager::get().get_rt()->create_table(2);

}

void VoxelGI::start_new()
{
	//Log::get() << 1 << Log::endl;

	all_scene_regen_counter = 2;
	tiled_volume_lighted->remove_all();

	//	Log::get() << 2 << Log::endl;

	tiled_volume_normal_static->remove_all();
	tiled_volume_albedo_static->remove_all();
	//Log::get() <<3 << Log::endl;

	tiled_volume_normal->remove_all();
	tiled_volume_albedo->remove_all();
	//	Log::get() << 4 << Log::endl;

	dynamic_generator.remove_all();
	dynamic_generator_voxelizing.remove_all();
	//	Log::get() << 5 << Log::endl;

	for (auto&& b : gpu_tiles_buffer)
		if (b)b->clear();
	tiled_volume_lighted->load_all();
	//	Log::get() << 6 << Log::endl;

	//tiled_volume_albedo->load_all();

	//	tiled_volume_albedo->test();
}


void VoxelGI::voxelize(MeshRenderContext::ptr& context, main_renderer* r)
{
	auto& graphics = context->list->get_graphics();
	auto& list = *context->list;


	static bool prev = 0;
	bool cur = !!GetAsyncKeyState('P');

	if (!cur && prev)
	{
		start_new();
		//all_scene_regen_counter++;
	}

	if (all_scene_regen_counter > 0)
	{
		visibility->wait_for_results();
	}
	prev = cur;
	if (!all_scene_regen_counter) {
		dynamic_generator.begin(scene->voxel_info.GetMin(), scene->voxel_info.GetMin() + scene->voxel_info.GetSize());
		dynamic_generator_voxelizing.begin(scene->voxel_info.GetMin(), scene->voxel_info.GetMin() + scene->voxel_info.GetSize());

		scene->iterate_meshes(MESH_TYPE::DYNAMIC, [this](scene_object::ptr& obj) {

			MeshAssetInstance* mesh = dynamic_cast<MeshAssetInstance*>(obj.get());

			if (mesh)
			{
				for (auto elem : mesh->rendering)
				{
					auto min = elem.primitive_global->get_min();
					auto max = elem.primitive_global->get_max();

					dynamic_generator.add(min, max);
					dynamic_generator_voxelizing.add(min, max);
				}
			}

			}
		);
		dynamic_generator.end();
		dynamic_generator_voxelizing.end();
	}

	for (auto& e : dynamic_generator.tiles_to_load)
	{

		auto pos = dynamic_generator.get_pos(e);

		auto& tile = tiled_volume_lighted->get_tile(0, pos);

		if (!tile || !tile->has_static)
		{
			auto& tile2 = tiled_volume_lighted->create_tile(pos, 0);
			tile2->has_dynamic = true;
		}

	}

	for (auto& e : dynamic_generator_voxelizing.tiles_to_load)
	{
		auto pos = dynamic_generator_voxelizing.get_pos(e);
		auto& tile1 = tiled_volume_albedo->create_dynamic_tile(pos, 0);
		auto& tile2 = tiled_volume_normal->create_dynamic_tile(pos, 0);
	}


	for (auto& e : dynamic_generator.tiles_to_remove)
	{
		auto pos = dynamic_generator.get_pos(e);

		auto& tile = tiled_volume_lighted->get_tile(0, pos);

		if (!tile)
			continue;
		if (!tile->has_static)
		{
			tile->has_static = false;
			tile->has_dynamic = false;
			tiled_volume_lighted->remove_tile(tile);



		}
		else
		{
			tile->has_dynamic = false;
		}

		//	remove_dynamic_tiles(pos);

	}

	for (auto& e : dynamic_generator_voxelizing.tiles_to_remove)
	{
		auto pos = dynamic_generator_voxelizing.get_pos(e);
		tiled_volume_albedo->remove_dynamic_tile(pos, 0);
		tiled_volume_normal->remove_dynamic_tile(pos, 0);
	}


	auto timer = context->list->start(L"voxelizing");




	tiled_volume_albedo->clear_dynamic(graphics.get_base());
	if (clear_scene && all_scene_regen_counter)
	{
		auto timer = context->list->start(L"clear");
		//if(all_scene_regen_counter)
		//tiled_volume_albedo->clear_static(graphics);
	//	dynamic_generator_voxelizing.remove_all();

	}
	else
	{
		for (auto& e : dynamic_generator_voxelizing.dynamic_tiles)
		{
			auto pos = dynamic_generator_voxelizing.get_pos(e);

			auto albedo_tile = tiled_volume_albedo->get_tile(0, pos);
			auto normal_tile = tiled_volume_normal->get_tile(0, pos);
			if (albedo_tile->page_backup)
			{
				auto from = albedo_tile->page_backup->get_tile_texture(albedo_tile->tile_offset_backup);
				auto to = albedo_tile->page->get_tile_texture(albedo_tile->tile_offset);

				context->list->get_copy().copy_texture(to, 0, from, 0);
			}


			if (normal_tile->page_backup)
			{
				auto from = normal_tile->page_backup->get_tile_texture(normal_tile->tile_offset_backup);
				auto to = normal_tile->page->get_tile_texture(normal_tile->tile_offset);

				context->list->get_copy().copy_texture(to, 0, from, 0);
			}
		}


	}


	context->render_type = RENDER_TYPE::VOXEL;
	context->render_mesh = MESH_TYPE::DYNAMIC;
	if (all_scene_regen_counter)
	{
		context->render_mesh = MESH_TYPE::STATIC;
	}


	context->pipeline.rtv.rtv_formats.clear();
	context->pipeline.rtv.enable_depth = false;
	context->pipeline.rtv.enable_depth_write = false;
	context->pipeline.rtv.ds_format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;

	graphics.set_viewport({ 0, 0, volume_albedo->get_size().xy });
	graphics.set_scissor({ 0, 0, volume_albedo->get_size().xy });
	graphics.set_rtv(0, Render::Handle(), Render::Handle());

	{
		auto timer = context->list->start(L"tiled_update");
		tiled_volume_albedo->update();
		tiled_volume_normal->update();
		tiled_volume_lighted->update();
	}



	{
		auto timer = context->list->start(L"render");
		r->render(context, scene);
	}


	if (all_scene_regen_counter)
	{
		auto timer = context->list->start(L"visibility update");
		visibility->update(context->list);

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

		data.gbuffer.need(builder,true);

		}, [this, &graph](VoxelDebug& data, FrameContext& _context) {

			auto& command_list = _context.get_list();

			auto voxel_lighted = _context.get_texture(data.voxel_lighted);

			MeshRenderContext::ptr context(new MeshRenderContext());
			auto target_tex = _context.get_texture(data.target_tex);
			auto gbuffer = data.gbuffer.actualize(_context);

			context->current_time = 0;
			//		context->sky_dir = lighting->lighting.pssm.get_position();
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

			graphics.set_rtv(1, target_tex.get_rtv(),Render::Handle());
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
				debug.GetVolume() = voxel_lighted.get_srv();
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
		data.sky_cubemap_filtered = builder.need_texture("sky_cubemap_filtered_diffuse", ResourceFlags::PixelRead);

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
				command_list->clear_rtv( gi_views[1].get_rtv(), vec4(0, 0, 0, 0));
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

				frameInfo.GetBrdf() = EngineAssets::brdf.get_asset()->get_texture()->texture_3d()->get_srv();
				frameInfo.GetTime() = graph.time;
				frameInfo.set(graphics);
			}

			{
				Slots::VoxelScreen voxelScreen;
				gbuffer.SetTable(voxelScreen.MapGbuffer());
				voxelScreen.GetVoxels() = volume_lighted->texture_3d()->get_static_srv();
				voxelScreen.GetTex_cube() = sky_cubemap_filtered.get_srv();
				voxelScreen.set(graphics);
			}
			scene->voxels_compiled.set(graphics);



			{	
				graphics.set_pipeline(GetPSO<PSOS::VoxelIndirectLow>());
				auto timer = context->list->start(L"downsampled");
				graphics.set_viewport(views[0].get_viewport());
				graphics.set_scissor(views[0].get_scissor());
				graphics.set_rtv(1, views[0].get_rtv(), Render::Handle());

				graphics.draw(4);
			}


			{
				auto timer = context->list->start(L"filter");

				graphics.set_pipeline(GetPSO<PSOS::VoxelIndirectFilter>());

				for (int i = 0; i < 1; i++)
				{
					Render::TextureView& cur = views[(i+1) % 2];
					Render::TextureView& prev = views[i % 2];

					{
						Slots::VoxelBlur voxelBlur;
						voxelBlur.GetTex_color() = prev.get_srv();
						voxelBlur.set(graphics);
					}

					graphics.set_rtv(1, cur.get_rtv(), Render::Handle());

					graphics.draw(4);
				}

			}




			{
				auto timer = context->list->start(L"main");

				{
					Slots::VoxelUpscale  voxelUpscale;
					voxelUpscale.GetTex_downsampled() = views[1].get_srv();
					voxelUpscale.GetTex_gi_prev() = gi_views[1- gi_index].get_srv();
					voxelUpscale.GetTex_depth_prev() = gbuffer.depth_prev_mips.get_srv();

					voxelUpscale.set(graphics);
				}


				graphics.set_viewport(target_tex.get_viewport());
				graphics.set_scissor(target_tex.get_scissor());

				gi_rtv[0].place(target_tex.get_rtv(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
				gi_rtv[1].place(gi_views[gi_index].get_rtv(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

				graphics.set_rtv(gi_rtv, gbuffer.quality.get_dsv());

				gi_index = 1 - gi_index;

				{
					auto timer = context->list->start(L"full");
					context->list->get_native_list()->OMSetStencilRef(1);
					graphics.set_pipeline(GetPSO<PSOS::VoxelIndirectHi>());
					graphics.draw(4);
				}


				{
					auto timer = context->list->start(L"resize");

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

				frameInfo.GetBrdf() = EngineAssets::brdf.get_asset()->get_texture()->texture_3d()->get_srv();
				frameInfo.GetTime() = graph.time;
				frameInfo.set(graphics);
			}

			{
				Slots::VoxelScreen voxelScreen;
				gbuffer.SetTable(voxelScreen.MapGbuffer());
				voxelScreen.GetVoxels() = volume_lighted->texture_3d()->get_static_srv();
				voxelScreen.GetTex_cube() = sky_cubemap_filtered.get_srv();
				voxelScreen.set(graphics);
			}
			scene->voxels_compiled.set(graphics);



			{
				graphics.set_pipeline(GetPSO<PSOS::VoxelReflectionLow>());
				auto timer = context->list->start(L"downsampled");
				graphics.set_viewport(downsampled_reflection.get_viewport());
				graphics.set_scissor(downsampled_reflection.get_scissor());
				graphics.set_rtv(1, downsampled_reflection.get_rtv(), Render::Handle());

				graphics.draw(4);
			}


			


			{
				auto timer = context->list->start(L"main");

				{
					Slots::VoxelUpscale  voxelUpscale;
					voxelUpscale.GetTex_downsampled() = downsampled_reflection.get_srv();
					voxelUpscale.GetTex_depth_prev() = gbuffer.depth_prev_mips.get_srv();

					voxelUpscale.set(graphics);
				}


				graphics.set_viewport(target_tex.get_viewport());
				graphics.set_scissor(target_tex.get_scissor());

 	   		graphics.set_rtv(1, target_tex.get_rtv(), gbuffer.quality.get_dsv());

	

				{
					auto timer = context->list->start(L"full");
					context->list->get_native_list()->OMSetStencilRef(2);
					graphics.set_pipeline(GetPSO<PSOS::VoxelReflectionHi>());
					graphics.draw(4);
				}


				{
					auto timer = context->list->start(L"resize");

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

			//SceneFrameManager::get().prepare(command_list, *scene);

			MeshRenderContext::ptr context(new MeshRenderContext());


			context->current_time = 0;
			//		context->sky_dir = lighting->lighting.pssm.get_position();
			context->priority = TaskPriority::HIGH;
			context->list = command_list;
			//	context->eye_context = vr_context;

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

		data.global_depth = builder.need_buffer("global_depth", ResourceFlags::ComputeRead);
		data.global_camera = builder.need_texture("global_camera", ResourceFlags::ComputeRead);
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
			//		context->sky_dir = lighting->lighting.pssm.get_position();
			context->priority = TaskPriority::HIGH;
			context->list = command_list;
			//	context->eye_context = vr_context;

			context->cam = graph.cam;



			auto& list = *context->list;
			auto& compute = context->list->get_compute();


		//	if (GetAsyncKeyState('G'))
				compute.set_pipeline(GetPSO<PSOS::Lighting>(PSOS::Lighting::SecondBounce.Use(!GetAsyncKeyState('G'))));
		//	else
			//	compute.set_pipeline(get_PSO(PSO::LightingSecondBounce));

			Slots::VoxelLighting ligthing;
			{

				ligthing.GetAlbedo() = volume_albedo->texture_3d()->get_static_srv();
				ligthing.GetNormals() = volume_normal->texture_3d()->get_static_srv();
				ligthing.GetOutput() = volume_lighted->texture_3d()->get_static_uav();
				ligthing.GetTex_cube() = sky_cubemap_filtered.get_srv();
				Render::ResourceViewDesc subres;
				subres.type = Render::ResourceType::TEXTURE3D;

				subres.Texture2D.ArraySize = 1;
				subres.Texture2D.FirstArraySlice = 0;
				subres.Texture2D.MipLevels = volume_lighted->get_desc().MipLevels - 1;
				subres.Texture2D.MipSlice = 1;
				subres.Texture2D.PlaneSlice = 0;

				ligthing.GetLower() = volume_lighted->create_view<TextureView>(*graph.builder.current_frame, subres).get_srv();

				ligthing.GetVisibility() = gpu_tiles_buffer[0]->get_srv()[0];
				ligthing.GetGroupCount() = tiled_volume_lighted->get_voxels_per_tile().x * tiled_volume_lighted->get_voxels_per_tile().y * tiled_volume_lighted->get_voxels_per_tile().z / (4 * 4 * 4);


				auto pssm = ligthing.MapPssmGlobal();

				pssm.GetLight_buffer() = global_depth.get_srv();

				auto buffer_view = global_camera.resource->create_view<StructuredBufferView<camera::shader_params>>(*graph.builder.current_frame);

				pssm.GetLight_camera() = buffer_view.get_srv();


				ligthing.set(compute);

			}
			graph.scene->voxels_compiled.set(compute);


			compute.dispach(ivec3(gpu_tiles_buffer[0]->size() * ligthing.GetGroupCount(), 1, 1), ivec3(1, 1, 1));

			//	compute.get_base().clear_uav(volume_lighted, volume_lighted->texture_3d()->get_static_uav(), vec4(1, 0, 0, 0.1));

		});

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
			//		context->sky_dir = lighting->lighting.pssm.get_position();
			context->priority = TaskPriority::HIGH;
			context->list = command_list;
			//	context->eye_context = vr_context;

			context->cam = graph.cam;



			auto& list = *context->list;
			auto& compute = context->list->get_compute();




			unsigned int mip_count = 0;

			//	list.transition(volume_lighted.get(), Render::ResourceState::PIXEL_SHADER_RESOURCE, mip_count);
			mip_count++;


			compute.set_signature(get_Signature(Layouts::DefaultLayout));


			graph.scene->voxels_compiled.set(compute);

			Slots::VoxelMipMap mipmapping;
			mipmapping.GetGroupCount() = tiled_volume_lighted->get_voxels_per_tile().x * tiled_volume_lighted->get_voxels_per_tile().y * tiled_volume_lighted->get_voxels_per_tile().z / (4 * 4 * 4);




			while (mip_count < volume_lighted->get_desc().MipLevels)
			{

				if(!gpu_tiles_buffer[mip_count]) break;
				unsigned int current_mips = std::min(3u, volume_lighted->get_desc().MipLevels - mip_count);
				compute.set_pipeline(GetPSO<PSOS::VoxelDownsample>(PSOS::VoxelDownsample::Count(current_mips)));

				mipmapping.GetSrcMip() = volume_lighted->texture_3d()->get_srv(mip_count - 1);

				for (unsigned int i = 0; i < current_mips; i++)
					mipmapping.GetOutMips()[i] = volume_lighted->texture_3d()->get_uav(mip_count + i);
			
				mipmapping.GetVisibility() = gpu_tiles_buffer[mip_count]->get_srv()[0];
				mipmapping.set(compute, false);

				compute.dispach(ivec3(gpu_tiles_buffer[mip_count]->size() * mipmapping.GetGroupCount(), 1, 1), ivec3(1, 1, 1));

				mip_count += current_mips;
			}
		});

}
void VoxelGI::generate(FrameGraph& graph)
{

	graph.builder.pass_texture("voxel_albedo", volume_albedo);
	graph.builder.pass_texture("voxel_normal", volume_normal);
	graph.builder.pass_texture("voxel_lighted", volume_lighted);

	if (need_start_new)
	{
		start_new();
		need_start_new = false;
	}

	Slots::VoxelInfo& voxel_info = scene->voxel_info;

	voxel_info.GetMin() = scene->get_min() - float3(1, 1, 1);
	voxel_info.GetSize() = scene->get_max() + float3(1, 1, 1) - scene->get_min();
	voxel_info.GetSize().x = voxel_info.GetSize().y = voxel_info.GetSize().z = max(200.0f, voxel_info.GetSize().max_element());

	voxel_info.GetVoxel_tiles_count() = tiled_volume_lighted->get_tiles_count(0);
	voxel_info.GetVoxels_per_tile() = tiled_volume_lighted->get_voxels_per_tile();

	scene->voxels_compiled = scene->voxel_info.compile(*graph.builder.current_frame);



	{
		Slots::Voxelization voxelization;
		voxelization.MapInfo().GetMin() = scene->voxel_info.GetMin();
		voxelization.MapInfo().GetSize() = scene->voxel_info.GetSize();
		voxelization.MapInfo().GetVoxel_tiles_count() = scene->voxel_info.GetVoxel_tiles_count();
		voxelization.MapInfo().GetVoxels_per_tile() = scene->voxel_info.GetVoxels_per_tile();

		voxelization.GetAlbedo() = volume_albedo->texture_3d()->get_static_uav();
		voxelization.GetNormals() = volume_normal->texture_3d()->get_static_uav();
		voxelization.GetVisibility() = visibility->buffer->create_view<Render::BufferView>(*graph.builder.current_frame).get_uav();

		scene->voxelization_compiled = voxelization.compile(*graph.builder.current_frame);
	}


	if (voxelize_scene) voxelize(graph);



	light_counter = (light_counter + 1) % 5;

	if (light_scene && light_counter == 0 || all_scene_regen_counter > 0)
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