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
		};

		auto size = graph.frame_size;

		graph.add_pass<DownsampleData>("GBufferDownsampler", [this, size](DownsampleData& data, TaskBuilder& builder) {

			data.gbuffer.need(builder, true, true);
			data.gbuffer.create_temp_color(size, builder);


			}, [this, &graph](DownsampleData& data, FrameContext& _context) {

				auto& command_list = _context.get_list();
				auto tempColor = *data.gbuffer.GBuffer_TempColor;
				auto gbuffer = data.gbuffer.actualize(_context);
				auto& graphics = command_list->get_graphics();

				graphics.set_signature(get_Signature(Layouts::DefaultLayout));

				graph.set_slot(SlotID::FrameInfo, graphics);

				graphics.set_topology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


				graphics.set_pipeline(GetPSO<PSOS::GBufferDownsample>());

				for (int i = 1; i < gbuffer.depth_mips.resource->get_desc().MipLevels; i++)
				{


					auto table = graphics.get_base().get_cpu_heap(Render::DescriptorHeapType::RTV).place(2);

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

						table[0].place(depth_view.renderTarget);
						table[1].place(normal_view.renderTarget);


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



VoxelGI::VoxelGI(Scene::ptr& scene) :scene(scene), VariableContext(L"VoxelGI")
{
	scene->on_element_add.register_handler(this, [this](scene_object* object) {
		auto render_object = dynamic_cast<MeshAssetInstance*>(object);

		if (!render_object) return;

		if (render_object->type == MESH_TYPE::STATIC)
			need_start_new = true;
		});


	{
		dispatch_command = Render::IndirectCommand::create_command<DispatchArguments>(sizeof(Underlying<DispatchArguments>));
	}

	{
		CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex3D(DXGI_FORMAT_R8G8B8A8_UNORM, 512, 512, 512, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE);

		albedo.set(desc);
		normal.set(desc);

		desc.MipLevels = 7;
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



}

void VoxelGI::start_new(Render::CommandList& list)
{


	tex_lighting.zero_tiles(list);

	albedo.zero_tiles(list);
	normal.zero_tiles(list);


	dynamic_generator_lighted.remove_all();
	dynamic_generator_voxelizing.remove_all();

}


void VoxelGI::voxelize(MeshRenderContext::ptr& context, main_renderer* r, FrameGraph& graph)
{
	auto& graphics = context->list->get_graphics();
	auto& compute = context->list->get_compute();
	auto& list = *context->list;



	if (all_scene_regen_counter > 0)
	{
		if (vis_update.valid())
		{
			auto updates = vis_update.get();


			auto albedo_tiles = updates.tiles_to_load | std::views::transform([this](ivec3 pos) {return pos / lighed_to_albedo_coeff; });

			albedo.tex_static->load_tiles2(&list, albedo_tiles);
			normal.tex_static->load_tiles2(&list, albedo_tiles);

			tex_lighting.load_static(updates.tiles_to_load);
		}
	}

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
		list.clear_uav(albedo.tex_static->texture_3d()->get_static_uav());
	}
	else
	{
		PROFILE_GPU(L"copy");

		albedo_tiles->update(context->list);
		compute.set_pipeline(GetPSO<PSOS::VoxelCopy>());
		//scene->voxels_compiled.set(compute);
		graph.set_slot(SlotID::VoxelInfo, compute);
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


}



void VoxelGI::debug(FrameGraph& graph)
{
	struct VoxelDebugData
	{
		GBufferViewDesc gbuffer;
		Handlers::Texture H(VoxelDebug);
		Handlers::Texture H(VoxelLighted);

	};

	auto size = graph.frame_size;

	graph.add_pass<VoxelDebugData>("VoxelDebug", [this, size](VoxelDebugData& data, TaskBuilder& builder) {

		 builder.create(data.VoxelDebug, { ivec3(size,1),  DXGI_FORMAT_R16G16B16A16_FLOAT,1 ,1}, ResourceFlags::RenderTarget);
		 builder.need(data.VoxelLighted, ResourceFlags::ComputeRead);

		data.gbuffer.need(builder);

		}, [this, &graph](VoxelDebugData& data, FrameContext& _context) {

			auto& command_list = _context.get_list();

			auto voxel_lighted = *data.VoxelLighted;

			MeshRenderContext::ptr context(new MeshRenderContext());
			auto target_tex = *data.VoxelDebug;
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

			graphics.set_rtv(1, target_tex.renderTarget, Render::Handle());
			graphics.set_pipeline(GetPSO<PSOS::VoxelDebug>());

			graph.set_slot(SlotID::VoxelInfo, graphics);
			graph.set_slot(SlotID::FrameInfo, graphics);

			//scene->voxels_compiled.set(graphics);

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
		Handlers::Texture H(ResultTexture);

		Handlers::Texture H(VoxelLighted);
		Handlers::Texture H(VoxelFramesCount);
		Handlers::Texture H(VoxelIndirectNoise);
		Handlers::Texture H(VoxelIndirectFiltered);

		Handlers::Texture H(sky_cubemap_filtered);


		Handlers::StructuredBuffer<DispatchArguments> H(VoxelScreen_hi);
		Handlers::StructuredBuffer<DispatchArguments> H(VoxelScreen_low);


		Handlers::StructuredBuffer<uint2> H(VoxelScreen_low_data);
		Handlers::StructuredBuffer<uint2> H(VoxelScreen_hi_data);

	};

	auto size = graph.frame_size;
	int count = 2 * Math::DivideByMultiple(size.x, 32) * Math::DivideByMultiple(size.y, 32);
	/*
	if (!hi || hi->get_count() < count)
	{
		hi = std::make_shared<Render::StructuredBuffer<uint2>>(count, Render::counterType::HELP_BUFFER, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		low = std::make_shared<Render::StructuredBuffer<uint2>>(count, Render::counterType::HELP_BUFFER, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	}*/

	graph.add_pass<Screen>("VoxelScreen", [this, size](Screen& data, TaskBuilder& builder) {


		data.gbuffer.need(builder, false);
		builder.create(data.VoxelFramesCount, { ivec3(size.x, size.y,1),  DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT, 1 ,1}, ResourceFlags::UnorderedAccess);
		 builder.create(data.VoxelIndirectNoise, { ivec3(size.x, size.y,1), DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT,1 ,0}, ResourceFlags::UnorderedAccess);
		builder.create(data.VoxelIndirectFiltered, { ivec3(size.x, size.y,1), DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT , 1,1 }, ResourceFlags::UnorderedAccess | ResourceFlags::Static);
		builder.need(data.sky_cubemap_filtered, ResourceFlags::PixelRead);
		builder.need(data.VoxelLighted, ResourceFlags::ComputeRead);

		builder.create(data.VoxelScreen_hi, { 1 }, ResourceFlags::UnorderedAccess);
		builder.create(data.VoxelScreen_low, { 1 }, ResourceFlags::UnorderedAccess);


		UINT count = 2 * Math::DivideByMultiple(size.x, 32) * Math::DivideByMultiple(size.y, 32);


		builder.create(data.VoxelScreen_low_data, { count, true }, ResourceFlags::UnorderedAccess);
		builder.create(data.VoxelScreen_hi_data, { count, true }, ResourceFlags::UnorderedAccess);


		}, [this, &graph](Screen& data, FrameContext& _context) {

			auto& command_list = _context.get_list();

			bool use_rtx = Device::get().is_rtx_supported() && this->use_rtx;
			auto gbuffer = data.gbuffer.actualize(_context);
			auto sky_cubemap_filtered =*data.sky_cubemap_filtered;
			auto noisy_output = *data.VoxelIndirectNoise;
			auto gi_filtered = *data.VoxelIndirectFiltered;
			auto frames_count = *data.VoxelFramesCount;

			auto size = noisy_output.get_size();


			if (data.VoxelIndirectFiltered.is_new())
			{
				command_list->clear_uav(gi_filtered.rwTexture2D, vec4(0, 0, 0, 0));
				command_list->clear_uav(noisy_output.rwTexture2D, vec4(0, 0, 0, 0));
				command_list->clear_uav(frames_count.rwTexture2D, vec4(0, 0, 0, 0));

			}


			auto scene = graph.scene;
			auto renderer = graph.renderer;

			auto& compute = command_list->get_compute();

			//	graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			if (use_rtx)

				command_list->get_compute().set_signature(RTX::get().rtx.m_root_sig);
			else
				compute.set_signature(get_Signature(Layouts::DefaultLayout));



			Slots::SceneData::Compiled& compiledScene = scene->compiledScene;


			compiledScene.set(command_list->get_compute());
			graph.set_slot(SlotID::FrameInfo, command_list->get_compute());



			{
				Slots::VoxelScreen voxelScreen;
				gbuffer.SetTable(voxelScreen.MapGbuffer());
				voxelScreen.GetVoxels() = tex_lighting.tex_result->texture_3d()->texture3D;
				voxelScreen.GetTex_cube() = sky_cubemap_filtered.texture�ube;
				//voxelScreen.GetPrev_frames() = views[1 - gi_index].texture2D;
				voxelScreen.GetPrev_depth() = gbuffer.depth_prev_mips.texture2D;
				voxelScreen.GetPrev_gi() = gi_filtered.texture2D;
				//	voxelScreen.set(graphics);
				voxelScreen.set(compute);
			}

			//scene->voxels_compiled.set(compute);
			//	scene->voxels_compiled.set(compute);
			graph.set_slot(SlotID::VoxelInfo, compute);

			{
				Slots::VoxelOutput output;

				output.GetFrames() = frames_count.rwTexture2D;
				output.GetNoise() = noisy_output.rwTexture2D;

				output.set(compute);
			}


			if (use_rtx)
			{
				RTX::get().render<Indirect>(compute, scene->raytrace_scene, noisy_output.get_size());
			}
			else
			{
				PROFILE_GPU(L"noise");
				compute.set_pipeline(GetPSO<PSOS::VoxelIndirectLow>());
				compute.dispach(size);
			}


			compute.set_signature(get_Signature(Layouts::DefaultLayout));
			{
				PROFILE_GPU(L"classification");

				command_list->clear_uav(data.VoxelScreen_hi_data->counter_view.rwRAW);
				command_list->clear_uav(data.VoxelScreen_low_data->counter_view.rwRAW);

				{
					Slots::FrameClassification frame_classification;

					frame_classification.GetFrames() = frames_count.texture2D;
					frame_classification.GetHi() = data.VoxelScreen_hi_data->appendStructuredBuffer;
					frame_classification.GetLow() = data.VoxelScreen_low_data->appendStructuredBuffer;

					frame_classification.set(compute);
				}

				compute.set_pipeline(GetPSO<PSOS::FrameClassification>());
				//compute.dispach(uint2( Math::DivideByMultiple(size.x, 32) , Math::DivideByMultiple(size.y, 32)));

				uint2 tiles_count = uint2(Math::DivideByMultiple(size.x, 32), Math::DivideByMultiple(size.y, 32));
				compute.dispach(tiles_count);
			}
			{
				PROFILE_GPU(L"init_dispatch");

				{
					Slots::FrameClassificationInitDispatch frame_classification_init;
					frame_classification_init.GetHi_counter() = data.VoxelScreen_hi_data->counter_view.structuredBuffer;
					frame_classification_init.GetLow_counter() = data.VoxelScreen_low_data->counter_view.structuredBuffer;

					frame_classification_init.GetHi_dispatch_data() = data.VoxelScreen_hi->rwStructuredBuffer;
					frame_classification_init.GetLow_dispatch_data() = data.VoxelScreen_low->rwStructuredBuffer;

					frame_classification_init.set(compute);
				}
				compute.set_pipeline(GetPSO<PSOS::FrameClassificationInitDispatch>());
				compute.dispach(1, 1, 1);
			}

			{
				PROFILE_GPU(L"mipmaps");
				MipMapGenerator::get().generate(compute, noisy_output);
			}

			if (denoiser)
			{
				PROFILE_GPU(L"history");
				compute.set_pipeline(GetPSO<PSOS::DenoiserHistoryFix>());
				{
					Slots::DenoiserHistoryFix denoiser_history;
					Render::ResourceViewDesc subres;
					subres.type = Render::ResourceType::TEXTURE2D;

					subres.Texture2D.ArraySize = 1;
					subres.Texture2D.FirstArraySlice = 0;
					subres.Texture2D.MipLevels = noisy_output.resource->get_desc().MipLevels - 1;
					subres.Texture2D.MipSlice = 1;
					subres.Texture2D.PlaneSlice = 0;

					denoiser_history.GetColor() = noisy_output.resource->create_view<TextureView>(*graph.builder.current_frame, subres).texture2D;

					subres.Texture2D.MipLevels = 1;
					denoiser_history.GetFrames() = frames_count.texture2D;
					denoiser_history.GetTarget() = noisy_output.rwTexture2D;
					denoiser_history.set(compute);
				}

				{
					Slots::TilingPostprocess tilingPostprocess;
					auto tiling = tilingPostprocess.MapTiling();
					tiling.GetTiles() = data.VoxelScreen_hi_data->structuredBuffer;
					tilingPostprocess.set(compute);
				}

				//	compute.dispach(frames_count.get_size());

				compute.execute_indirect(dispatch_command, 1, data.VoxelScreen_hi->resource.get());
			}




		}, PassFlags::Compute);




	graph.add_pass<Screen>("VoxelCombine", [this, size](Screen& data, TaskBuilder& builder) {

		builder.need(data.ResultTexture, ResourceFlags::UnorderedAccess);

		data.gbuffer.need(builder, false);
	 builder.need(data.VoxelIndirectFiltered, ResourceFlags::UnorderedAccess);
	 builder.need(data.sky_cubemap_filtered, ResourceFlags::PixelRead);
		builder.need(data.VoxelFramesCount, ResourceFlags::UnorderedAccess);
		builder.need(data.VoxelIndirectNoise, ResourceFlags::ComputeRead);

		builder.need(data.VoxelScreen_hi,ResourceFlags::ComputeRead);
		builder.need(data.VoxelScreen_low, ResourceFlags::ComputeRead);


		builder.need(data.VoxelScreen_low_data);
		builder.need(data.VoxelScreen_hi_data);

		}, [this, &graph](Screen& data, FrameContext& _context) {

			auto& command_list = _context.get_list();


			auto target_tex = *(data.ResultTexture);
			auto gbuffer = data.gbuffer.actualize(_context);
			auto sky_cubemap_filtered = *(data.sky_cubemap_filtered);
			auto noisy_output = *(data.VoxelIndirectNoise);
			auto frames_count = *(data.VoxelFramesCount);

			auto gi_filtered = *(data.VoxelIndirectFiltered);

			auto size = target_tex.get_size();

			auto scene = graph.scene;
			auto renderer = graph.renderer;

			auto& compute = command_list->get_compute();

			//	graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			compute.set_signature(get_Signature(Layouts::DefaultLayout));

			graph.set_slot(SlotID::FrameInfo, command_list->get_compute());


			{
				Slots::VoxelScreen voxelScreen;
				gbuffer.SetTable(voxelScreen.MapGbuffer());
				voxelScreen.GetVoxels() = tex_lighting.tex_result->texture_3d()->texture3D;
				voxelScreen.GetTex_cube() = sky_cubemap_filtered.texture�ube;
				//voxelScreen.GetPrev_frames() = views[1 - gi_index].texture2D;
				voxelScreen.GetPrev_depth() = gbuffer.depth_prev_mips.texture2D;
				voxelScreen.GetPrev_gi() = gi_filtered.texture2D;
				//	voxelScreen.set(graphics);
				voxelScreen.set(compute);
			}

			//	scene->voxels_compiled.set(compute);
				//	scene->voxels_compiled.set(compute);
			graph.set_slot(SlotID::VoxelInfo, compute);


			{
				Slots::VoxelBlur voxelBlur;

				voxelBlur.GetNoisy_output() = noisy_output.texture2D;
				voxelBlur.GetPrev_result() = frames_count.texture2D;

				voxelBlur.GetScreen_result() = target_tex.rwTexture2D;
				voxelBlur.GetGi_result() = gi_filtered.rwTexture2D;



				voxelBlur.set(compute);
			}

			{
				PROFILE_GPU(L"blur");
				compute.set_pipeline(GetPSO<PSOS::VoxelIndirectFilter>(PSOS::VoxelIndirectFilter::Blur()));

				{
					Slots::TilingPostprocess tilingPostprocess;
					auto tiling = tilingPostprocess.MapTiling();
					tiling.GetTiles() = data.VoxelScreen_hi_data->structuredBuffer;
					tilingPostprocess.set(compute);
				}

				compute.execute_indirect(dispatch_command, 1, data.VoxelScreen_hi->resource.get());
			}

			{
				PROFILE_GPU(L"blur2");
				compute.set_pipeline(GetPSO<PSOS::VoxelIndirectFilter>());

				{
					Slots::TilingPostprocess tilingPostprocess;
					auto tiling = tilingPostprocess.MapTiling();
					tiling.GetTiles() = data.VoxelScreen_low_data->structuredBuffer;
					tilingPostprocess.set(compute);
				}


				compute.execute_indirect(dispatch_command, 1, data.VoxelScreen_low->resource.get());
			}


		}, PassFlags::Compute);



	gi_index = 1 - gi_index;

}



void VoxelGI::screen_reflection(FrameGraph& graph)
{
	struct ScreenReflection
	{
		GBufferViewDesc gbuffer;

		Handlers::Texture H(VoxelReflectionNoise);
		Handlers::Texture H(VoxelReflectionFiltered);

		Handlers::Texture H(sky_cubemap_filtered);

		Handlers::Texture H(noise_dir_pdf);
Handlers::Texture H(prev_gi_temp);

Handlers::Texture H(ResultTexture);


Handlers::StructuredBuffer<DispatchArguments> H(VoxelScreen_hi);
Handlers::StructuredBuffer<DispatchArguments> H(VoxelScreen_low);

Handlers::StructuredBuffer<uint2> H(VoxelScreen_low_data);
Handlers::StructuredBuffer<uint2> H(VoxelScreen_hi_data);
	};
	auto size = graph.frame_size;

	graph.add_pass<ScreenReflection>("ScreenReflection", [this, size](ScreenReflection& data, TaskBuilder& builder) {

		 builder.need(data.ResultTexture, ResourceFlags::RenderTarget);
		 builder.create(data.VoxelReflectionNoise, { ivec3(size.x, size.y,1),  DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT,1 ,1}, ResourceFlags::UnorderedAccess);
		 builder.create(data.noise_dir_pdf, { ivec3(size.x, size.y,1),  DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT,1,1 }, ResourceFlags::UnorderedAccess);

		data.gbuffer.need(builder, false);
		//	data.downsampled_reflection = builder.create("downsampled_reflection", ivec2(size.x / 2, size.y / 2), 1, DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, ResourceFlags::RenderTarget);
		builder.need(data.sky_cubemap_filtered, ResourceFlags::PixelRead);
		}, [this, &graph](ScreenReflection& data, FrameContext& _context) {

			auto& command_list = _context.get_list();


			MeshRenderContext::ptr context(new MeshRenderContext());
			auto target_tex = *(data.ResultTexture);
			auto gbuffer = data.gbuffer.actualize(_context);
			auto sky_cubemap_filtered = *(data.sky_cubemap_filtered);
			auto noisy_output = *(data.VoxelReflectionNoise);
			auto dir_and_pdf = *(data.noise_dir_pdf);
			//	auto gi_filtered = *(data.gi_filtered);

			//	Render::TextureView downsampled_reflection = *(data.downsampled_reflection);



			context->current_time = 0;
			context->priority = TaskPriority::HIGH;
			context->list = command_list;

			context->cam = graph.cam;

			auto scene = graph.scene;
			auto renderer = graph.renderer;
			context->begin();

			auto& graphics = context->list->get_graphics();
			auto& compute = context->list->get_compute();


			graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			graphics.set_signature(get_Signature(Layouts::DefaultLayout));

			{

				Slots::SceneData::Compiled& compiledScene = scene->compiledScene;

				graph.set_slot(SlotID::FrameInfo, compute);
				graph.set_slot(SlotID::FrameInfo, graphics);

				compiledScene.set(compute);
				compiledScene.set(graphics);
			}


			{
				Slots::VoxelScreen voxelScreen;
				gbuffer.SetTable(voxelScreen.MapGbuffer());
				voxelScreen.GetVoxels() = tex_lighting.tex_result->texture_3d()->texture3D;
				voxelScreen.GetTex_cube() = sky_cubemap_filtered.texture�ube;
				//		voxelScreen.GetPrev_gi() = gi_filtered.texture2D;
				voxelScreen.set(graphics);
				voxelScreen.set(compute);
			}

			//	scene->voxels_compiled.set(graphics);
			//	scene->voxels_compiled.set(compute);

			graph.set_slot(SlotID::VoxelInfo, compute);
			graph.set_slot(SlotID::VoxelInfo, graphics);



			if (use_rtx)
			{
				{
					Slots::VoxelOutput output;

					//	output.GetFrames() = gi_filtered.rwTexture2D;
					output.GetNoise() = noisy_output.rwTexture2D;
					output.GetDirAndPdf() = dir_and_pdf.rwTexture2D;
					output.set(compute);
				}


				RTX::get().render<Reflection>(compute, scene->raytrace_scene, noisy_output.get_size());

				//		command_list->get_copy().copy_resource(gi_filtered.resource, noisy_output.resource);
			}
			else
			{
				graphics.set_viewport(target_tex.get_viewport());
				graphics.set_scissor(target_tex.get_scissor());
				graphics.set_rtv(1, target_tex.renderTarget, gbuffer.quality.depthStencil);


				PROFILE_GPU(L"full");
				graphics.set_stencil_ref(2);
				graphics.set_pipeline(GetPSO<PSOS::VoxelReflectionHi>());
				graphics.draw(4);
			}

		});

		if(Device::get().is_rtx_supported() && this->use_rtx)
	graph.add_pass<ScreenReflection>("ReflCombine", [this, size](ScreenReflection& data, TaskBuilder& builder) {

		builder.need(data.ResultTexture, ResourceFlags::UnorderedAccess);

		data.gbuffer.need(builder, false);
		builder.create(data.VoxelReflectionFiltered, { ivec3(size.x, size.y,1),  DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT,1,1 }, ResourceFlags::UnorderedAccess | ResourceFlags::Static);
		 builder.create(data.prev_gi_temp, { ivec3(size.x, size.y,1), DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT,1,1 },ResourceFlags::RenderTarget);

	
		builder.need(data.sky_cubemap_filtered, ResourceFlags::PixelRead);

	 builder.need(data.VoxelReflectionNoise, ResourceFlags::ComputeRead);

		 builder.need(data.noise_dir_pdf, ResourceFlags::ComputeRead); 


		 builder.need(data.VoxelScreen_hi, ResourceFlags::ComputeRead);
		 builder.need(data.VoxelScreen_low, ResourceFlags::ComputeRead);
		 builder.need(data.VoxelScreen_low_data, ResourceFlags::ComputeRead);
		 builder.need(data.VoxelScreen_hi_data, ResourceFlags::ComputeRead);

		}, [this, &graph](ScreenReflection& data, FrameContext& _context) {

			auto& command_list = _context.get_list();


			auto target_tex = *(data.ResultTexture);
			auto gbuffer = data.gbuffer.actualize(_context);
			auto sky_cubemap_filtered = *(data.sky_cubemap_filtered);
			auto noisy_output = *(data.VoxelReflectionNoise);
			//	auto frames_count = *(data.frames_count);
			auto dir_and_pdf = *(data.noise_dir_pdf);

			auto cur_gi = *(data.VoxelReflectionFiltered);

			auto prev_gi = *(data.prev_gi_temp);

			if (data.VoxelReflectionFiltered.is_new())
			{
				command_list->clear_uav(cur_gi.rwTexture2D, vec4(0, 0, 0, 0));
			}

			command_list->get_copy().copy_resource(prev_gi.resource, cur_gi.resource);

			auto size = target_tex.get_size();

			auto scene = graph.scene;
			auto renderer = graph.renderer;

			auto& compute = command_list->get_compute();

			//	graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			compute.set_signature(get_Signature(Layouts::DefaultLayout));


			{
				Slots::VoxelScreen voxelScreen;
				gbuffer.SetTable(voxelScreen.MapGbuffer());
				voxelScreen.GetVoxels() = tex_lighting.tex_result->texture_3d()->texture3D;
				voxelScreen.GetTex_cube() = sky_cubemap_filtered.texture�ube;
				//voxelScreen.GetPrev_frames() = views[1 - gi_index].texture2D;
				voxelScreen.GetPrev_depth() = gbuffer.depth_prev_mips.texture2D;
				voxelScreen.GetPrev_gi() = prev_gi.texture2D;
				//	voxelScreen.set(graphics);
				voxelScreen.set(compute);
			}

			//	scene->voxels_compiled.set(compute);
				//	scene->voxels_compiled.set(compute);
			graph.set_slot(SlotID::VoxelInfo, compute);
			graph.set_slot(SlotID::FrameInfo, compute);

{
					Slots::VoxelBlur voxelBlur;

					voxelBlur.GetNoisy_output() = noisy_output.texture2D;
					voxelBlur.GetPrev_result() = prev_gi.texture2D;

					voxelBlur.GetScreen_result() = target_tex.rwTexture2D;
					voxelBlur.GetGi_result() = cur_gi.rwTexture2D;
					voxelBlur.GetHit_and_pdf() = dir_and_pdf.texture2D;

					voxelBlur.set(compute);
				}
			{
				PROFILE_GPU(L"blur");
				compute.set_pipeline(GetPSO<PSOS::VoxelIndirectFilter>(/*PSOS::VoxelIndirectFilter::Blur() |*/ PSOS::VoxelIndirectFilter::Reflection()));
				

				{
					Slots::TilingPostprocess tilingPostprocess;
					auto tiling = tilingPostprocess.MapTiling();
					tiling.GetTiles() = data.VoxelScreen_hi_data->structuredBuffer;
					tilingPostprocess.set(compute);
				}


				compute.execute_indirect(dispatch_command, 1, data.VoxelScreen_hi->resource.get());
			}

			{
				PROFILE_GPU(L"blur2");
				compute.set_pipeline(GetPSO<PSOS::VoxelIndirectFilter>(PSOS::VoxelIndirectFilter::Reflection()));

				{
					Slots::TilingPostprocess tilingPostprocess;
					auto tiling = tilingPostprocess.MapTiling();
					tiling.GetTiles() = data.VoxelScreen_low_data->structuredBuffer;
					tilingPostprocess.set(compute);
				}

				compute.execute_indirect(dispatch_command, 1, data.VoxelScreen_low->resource.get());
			}


		}, PassFlags::Compute);

	refl_index = 1 - refl_index;
}


void VoxelGI::voxelize(FrameGraph& graph)
{
	struct Voxelize
	{
		Handlers::Texture H(VoxelAlbedo);
		Handlers::Texture H(VoxelNormal);

	};

	graph.add_pass<Voxelize>("voxelize", [this](Voxelize& data, TaskBuilder& builder) {


	builder.need(data.VoxelAlbedo, ResourceFlags::UnorderedAccess);
		 builder.need(data.VoxelNormal, ResourceFlags::UnorderedAccess);

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

			voxelize(context, renderer, graph);
		});
}

void VoxelGI::lighting(FrameGraph& graph)
{


	struct Lighting
	{
		Handlers::Texture H(global_depth);
		Handlers::StructuredBuffer<Table::Camera::CB> H(global_camera);
		Handlers::Texture H(VoxelLighted);
		Handlers::Texture H(VoxelAlbedo);
		Handlers::Texture H(VoxelNormal);
		Handlers::Texture H(sky_cubemap_filtered);
	};


	graph.add_pass<Lighting>("Lighting", [this](Lighting& data, TaskBuilder& builder) {

		builder.need(data.global_depth, ResourceFlags::ComputeRead);
		builder.need(data.global_camera, ResourceFlags::ComputeRead);
		builder.need(data.sky_cubemap_filtered, ResourceFlags::PixelRead);


		builder.need(data.VoxelLighted, ResourceFlags::UnorderedAccess);
		builder.need(data.VoxelAlbedo, ResourceFlags::ComputeRead);
		builder.need(data.VoxelNormal, ResourceFlags::ComputeRead);


		}, [this, &graph](auto& data, FrameContext& _context) {

			auto& command_list = _context.get_list();

			auto sky_cubemap_filtered = *(data.sky_cubemap_filtered);

			MeshRenderContext::ptr context(new MeshRenderContext());
			context->current_time = 0;
			context->priority = TaskPriority::HIGH;
			context->list = command_list;
			context->cam = graph.cam;



			auto& list = *context->list;
			auto& compute = context->list->get_compute();

			compute.set_pipeline(GetPSO<PSOS::Lighting>(PSOS::Lighting::SecondBounce.Use((all_scene_regen_counter == 0) && multiple_bounces)));

			Slots::VoxelLighting ligthing;
			{

				ligthing.GetAlbedo() = albedo.tex_result->texture_3d()->texture3D;
				ligthing.GetNormals() = normal.tex_result->texture_3d()->texture3D;
				ligthing.GetOutput() = tex_lighting.tex_result->texture_3d()->rwTexture3D[0];
				ligthing.GetTex_cube() = sky_cubemap_filtered.texture�ube;
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

				pssm.GetLight_buffer() = data.global_depth->texture2D;

				auto buffer_view = data.global_camera->resource->create_view<StructuredBufferView<Table::Camera>>(*graph.builder.current_frame);

				pssm.GetLight_camera() = buffer_view.structuredBuffer;


				ligthing.set(compute);

			}
			//graph.scene->voxels_compiled.set(compute);
			graph.set_slot(SlotID::VoxelInfo, compute);

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
		Handlers::Texture H(VoxelLighted);
	};

	graph.add_pass<Mipmapping>("Mipmapping", [this](Mipmapping& data, TaskBuilder& builder) {

		 builder.need(data.VoxelLighted, ResourceFlags::UnorderedAccess);

		}, [this, &graph](Mipmapping& data, FrameContext& _context) {

			auto& command_list = _context.get_list();

			auto voxel_lighted = *(data.VoxelLighted);

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


			//graph.scene->voxels_compiled.set(compute);
			graph.set_slot(SlotID::VoxelInfo, compute);


			compute.set_pipeline(GetPSO<PSOS::VoxelZero>());
			{
				PROFILE_GPU(L"ZERO");
				for (int i = 1; i < tex_lighting.tex_result->get_desc().MipLevels; i++)
				{
					if (i >= gpu_tiles_buffer.size() || !gpu_tiles_buffer[i])
					{
						list.clear_uav(tex_lighting.tex_result->texture_3d()->rwTexture3D[i], vec4(0, 0, 0, 0));
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
					PROFILE_GPU(std::wstring(L"mip_") + std::to_wstring(mip_count));
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

	graph.builder.pass_texture("VoxelAlbedo", albedo.tex_result);
	graph.builder.pass_texture("VoxelNormal", normal.tex_result);
	graph.builder.pass_texture("VoxelLighted", tex_lighting.tex_result);

	Slots::VoxelInfo& voxel_info = scene->voxel_info;

	//if (all_scene_regen_counter)
	{
		min = scene->get_min() - float3(1, 1, 1);
		size = scene->get_max() + float3(1, 1, 1) - scene->get_min();
	}

	if (all_scene_regen_counter > 0)
		all_scene_regen_counter--;


	static bool prev = 0;
	bool cur = !!GetAsyncKeyState('P');

	if (!cur && prev)
	{
		need_start_new = true;
	}

	prev = cur;

	if(need_start_new)
		all_scene_regen_counter = 3;

	voxel_info.GetMin() = min;
	voxel_info.GetSize() = size;
	voxel_info.GetSize().x = voxel_info.GetSize().y = voxel_info.GetSize().z = max(200.0f, voxel_info.GetSize().max_element());

	voxel_info.GetVoxel_tiles_count() = tex_lighting.tex_result->get_tiles_count(0);
	voxel_info.GetVoxels_per_tile() = tex_lighting.tex_result->get_tile_shape();

	scene->voxels_compiled = scene->voxel_info.compile(*graph.builder.current_frame);

	graph.register_slot_setter(scene->voxels_compiled);

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



	//if (!downsampler) downsampler = std::make_shared<GBufferDownsampler>();
	//downsampler->generate(graph);

	screen(graph);

	if (reflecton)
		screen_reflection(graph);



	debug(graph);

}