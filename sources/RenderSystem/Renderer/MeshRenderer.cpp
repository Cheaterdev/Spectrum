module Graphics:MeshRenderer;
import RenderSystem;


import :EngineAssets;
import :MipMapGenerator;

import HAL;

using namespace HAL;
void mesh_renderer::render(MeshRenderContext::ptr mesh_render_context, Scene::ptr scene)
{

	// return;
	auto& graphics = mesh_render_context->list->get_graphics();
	auto& compute = mesh_render_context->list->get_compute();
	auto& copy = mesh_render_context->list->get_copy();
	auto& list = *mesh_render_context->list;
	GBuffer* gbuffer = mesh_render_context->g_buffer;
	PROFILE_GPU(L"mesh_renderer");

	instances_count = 0;
		
	mesh_render_context->begin();

	Slots::SceneData::Compiled& compiledScene = scene->compiledScene;

	UINT meshes_count = (UINT)scene->command_ids[(int)mesh_render_context->render_mesh].size();
	graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);


	std::map<size_t, materials::Pipeline::ptr> pipelines_local;
	std::map<size_t, materials::Pipeline::ptr>* pipelines_ptr = &pipelines_local;

	pipelines_ptr = &scene->pipelines;

	std::map<size_t, materials::Pipeline::ptr>& pipelines = *pipelines_ptr;

	if (meshes_count == 0) return;

	commands_boxes->reserve(list, meshes_count);
	visible_boxes->reserve(list, meshes_count);
	meshes_ids->reserve(list, meshes_count);
	meshes_invisible_ids->reserve(list, meshes_count);
	for (int i = 0; i < 8; i++)
		commands_buffer[i]->reserve(list, meshes_count);

	graphics.set(compiledScene);
	compute.set(compiledScene);

	if (!gbuffer || !use_gpu_occlusion)
	{
		// Scene mesh count is CPU-known — direct dispatch, no indirect args.
		render_meshes(mesh_render_context, scene, pipelines, scene->compiledGather[(int)mesh_render_context->render_mesh], (mesh_render_context->render_type != RENDER_TYPE::VOXEL), nullptr, meshes_count, false);
		return;
	}

	{

		PROFILE_GPU(L"first stage");
		// The AS samples the Hi-Z pyramid (IsOccludedHiZ) through the global
		// FrameInfo slot, so its read state isn't tracked per-pass -- make it
		// explicit, or it is still UNORDERED_ACCESS from the last build.
		if (use_meshlet_hiz_occlusion)
			list.transition(gbuffer->HalfBuffer.hiZ_depth_uav.resource, HAL::ResourceStates::SHADER_RESOURCE);
		// Stage 1 walks the full scene list: CPU-known count, direct dispatch.
		generate_boxes(mesh_render_context, scene, scene->compiledGather[(int)mesh_render_context->render_mesh], nullptr, meshes_count);

		draw_boxes(mesh_render_context, scene);
		gather_rendered_boxes(mesh_render_context, scene, true);

		render_meshes(mesh_render_context, scene, pipelines, gather_visible, false, &render_args, 0, false);
		MipMapGenerator::get().downsample_depth(compute, gbuffer->depth, gbuffer->HalfBuffer.hiZ_depth_uav);
		// Coarser mips on top of mip 0, for the AS's per-meshlet test below.
		// Skipped with the toggle off so disabling it is a clean baseline.
		if (use_meshlet_hiz_occlusion)
			MipMapGenerator::get().build_hiz_pyramid(compute, gbuffer->HalfBuffer.hiZ_depth_uav);
		MipMapGenerator::get().write_to_depth(graphics, gbuffer->HalfBuffer.hiZ_depth_uav, gbuffer->HalfBuffer.hiZ_depth);
	}


	{
		PROFILE_GPU(L"second stage");
		// Same as stage 1: stage 1 left the pyramid in UNORDERED_ACCESS.
		if (use_meshlet_hiz_occlusion)
			list.transition(gbuffer->HalfBuffer.hiZ_depth_uav.resource, HAL::ResourceStates::SHADER_RESOURCE);
		// Stage 2 retests the invisible list: sized by retest_args, written by
		// stage 1's GatherMeshes.
		generate_boxes(mesh_render_context, scene, gather_invisible, &retest_args, 0);

		draw_boxes(mesh_render_context, scene);
		gather_rendered_boxes(mesh_render_context, scene, false);

		render_meshes(mesh_render_context, scene, pipelines, gather_visible, false, &render_args, 0, use_meshlet_hiz_occlusion);

		MipMapGenerator::get().downsample_depth(compute, gbuffer->depth, gbuffer->HalfBuffer.hiZ_depth_uav);
		// Coarser mips on top of mip 0, for the AS's per-meshlet test below.
		// Skipped with the toggle off so disabling it is a clean baseline.
		if (use_meshlet_hiz_occlusion)
			MipMapGenerator::get().build_hiz_pyramid(compute, gbuffer->HalfBuffer.hiZ_depth_uav);
		MipMapGenerator::get().write_to_depth(graphics, gbuffer->HalfBuffer.hiZ_depth_uav, gbuffer->HalfBuffer.hiZ_depth);

	}


}

void  mesh_renderer::gather_rendered_boxes(MeshRenderContext::ptr mesh_render_context, Scene::ptr scene, bool invisibleToo)
{
	PROFILE_GPU(L"gather_rendered_boxes");

	auto& graphics = mesh_render_context->list->get_graphics();
	auto& compute = mesh_render_context->list->get_compute();
	auto& copy = mesh_render_context->list->get_copy();
	auto& list = *mesh_render_context->list;
	UINT meshes_count = (UINT)scene->command_ids[(int)mesh_render_context->render_mesh].size();
	Slots::SceneData::Compiled& compiledScene = scene->compiledScene;

	if (invisibleToo)
	{
		// Counter only: stage 2 reads the list strictly counter-bounded (the
		// GatherPipelineGlobal binding is gather_boxes_commands / list counters),
		// so entries past the counter are never touched — the old full-buffer
		// 999 clear of the DATA was defensive armor for the wrong-count bug.
		compute.clear_counter(meshes_invisible_ids->buffer);

		// Zero the stage-2 retest args this dispatch is about to bump (CLEAR
		// sync -> real barrier before the COMPUTE producer).
		compute.clear(retest_args);
	}

	{

		compute.set_pipeline<PSOS::GatherMeshes>(PSOS::GatherMeshes::Invisible.Use(invisibleToo));
		// Bound check must be the commands_boxes counter (what the indirect
		// dispatch was sized from) — binding the scene's total mesh count lets
		// the tail threads of the last group read stale BoxInfo entries past
		// the counter and append garbage mesh ids.
		compute.set(gather_boxes_commands);
		compute.set(gather_neshes_boxes_compiled);

		graphics.exec_indirect(gather_meshes_args, 1);
	}

}


void  mesh_renderer::generate_boxes(MeshRenderContext::ptr mesh_render_context, Scene::ptr scene, Slots::GatherPipelineGlobal::Compiled& gatherData, HAL::StructuredBufferView<DispatchArguments>* dispatch_args, UINT direct_count)
{
	PROFILE_GPU(L"generate_boxes");

	auto& graphics = mesh_render_context->list->get_graphics();
	auto& compute = mesh_render_context->list->get_compute();
	auto& copy = mesh_render_context->list->get_copy();
	auto& list = *mesh_render_context->list;
	Slots::SceneData::Compiled& compiledScene = scene->compiledScene;


	{
		compute.clear_counter(commands_boxes->buffer);
		compute.clear_counter(meshes_ids->buffer);

		// Zero the args this stage's producers bump (box draw / gather /
		// render). clear_uav carries CLEAR sync, so the tracker emits a real
		// barrier before the COMPUTE producers — a reset dispatch would be
		// same-state UAV->UAV and race. Producers restore the constant
		// components (y/z=1, IndexCount=36). Replaces InitDispatch + the
		// counter->InstanceCount copy.
		compute.clear(draw_boxes_first);
		compute.clear(gather_meshes_args);
		compute.clear(render_args);


		compute.set_pipeline<PSOS::GatherBoxes>();
		compute.set(gather_boxes_compiled);
		compute.set(gatherData);

		{
			PROFILE_GPU(L"dispatch");
			if (dispatch_args)
				graphics.exec_indirect(*dispatch_args, 1);
			else
				compute.dispatch((int)Math::DivideByMultiple(direct_count, 64), 1, 1);
		}



	}


}

void  mesh_renderer::draw_boxes(MeshRenderContext::ptr mesh_render_context, Scene::ptr scene)
{
	PROFILE_GPU(L"draw_boxes");

	auto& graphics = mesh_render_context->list->get_graphics();
	auto& compute = mesh_render_context->list->get_compute();
	auto& copy = mesh_render_context->list->get_copy();
	auto& list = *mesh_render_context->list;

	GBuffer* gbuffer = mesh_render_context->g_buffer;
	UINT meshes_count = (UINT)scene->command_ids[(int)mesh_render_context->render_mesh].size();

	graphics.set_rtv(gbuffer->HalfBuffer.compiled);

	graphics.set_pipeline<PSOS::RenderBoxes>();
	graphics.set_index_buffer(index_buffer.get_index_buffer_view());
	graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);

	// visible_boxes sentinel (999) is initialized per candidate by CS_boxes —
	// no full-buffer clear here. InstanceCount was bumped there too.

	graphics.set(draw_boxes_compiled);

	graphics.exec_indirect(draw_boxes_first, 1);


	graphics.set_rtv(gbuffer->compiled);
}
void  mesh_renderer::render_meshes(MeshRenderContext::ptr mesh_render_context, Scene::ptr scene, std::map<size_t, materials::Pipeline::ptr>& pipelines, Slots::GatherPipelineGlobal::Compiled& gatherData, bool needCulling, HAL::StructuredBufferView<DispatchArguments>* dispatch_args, UINT direct_count, bool hiz_occlusion)
{
	PROFILE_GPU(L"render_meshes");

	auto& graphics = mesh_render_context->list->get_graphics();
	auto& compute = mesh_render_context->list->get_compute();
	auto& copy = mesh_render_context->list->get_copy();
	auto& list = *mesh_render_context->list;

	UINT meshes_count = (UINT)scene->command_ids[(int)mesh_render_context->render_mesh].size();

	Slots::GatherPipeline gather;

	auto begin = pipelines.begin();
	auto end = begin;




	graphics.set_index_buffer(HAL::Views::IndexBuffer());// universal_index_manager::get().buffer->get_index_buffer_view(true));
	while (end != pipelines.end())
	{
		begin = end;

		int total = 0;
		{
			PROFILE_GPU(L"first 8");
			 while (total < 8)
		{
			((UINT*)gather.GetPip_ids())[total] = end->second->get_id();
			gather.GetCommands()[total] = commands_buffer[total]->buffer;
			end++; total++;
			if (end == pipelines.end()) break;
		}
		
		}
		
				   {
			PROFILE_GPU(L"last 8");
		for (int i = total; i < 8; i++)
		{
			((UINT*)gather.GetPip_ids())[i] = std::numeric_limits<uint>::max();
			gather.GetCommands()[i] = commands_buffer[i]->buffer;
		}

		}


		{
			PROFILE_GPU(L"clear");
			for (int i = 0; i < total; i++)
				compute.clear_counter(commands_buffer[i]->buffer);
		}



		{
			PROFILE_GPU(L"GatherMats");

			compute.set_pipeline<PSOS::GatherPipeline>(PSOS::GatherPipeline::CheckFrustum.Use(needCulling));
			compute.set(gatherData);
			compute.set(gather);

			{
				PROFILE_GPU(L"dispatch");

				if (dispatch_args)
					compute.exec_indirect(*dispatch_args, 1);
				else
					compute.dispatch((int)Math::DivideByMultiple(direct_count, 64), 1, 1);
			}

		}

		//if(0)
		{
			PROFILE_GPU(L"YO");
			int current = 0;

{
					PROFILE_GPU(L"compile");

					if (mesh_render_context->render_type == RENDER_TYPE::VOXEL)
						graphics.set(mesh_render_context->voxelization_compiled);
				}

			for (auto it = begin; it != end; it++)
			{
				{
					PROFILE_GPU(L"flush");
			
					it->second->set(mesh_render_context->render_type, mesh_render_context->render_mesh, graphics, hiz_occlusion);
				}

				{
					PROFILE_GPU(L"execute_indirect");

					graphics.exec_indirect( commands_buffer[current]->buffer, meshes_count);

				}

				current++;

			}
		}
	}
}
void mesh_renderer::iterate(MESH_TYPE mesh_type, std::function<void(scene_object::ptr&)> f)
{

}


mesh_renderer::mesh_renderer() :VariableContext(L"mesh_renderer")
{
	best_fit_normals = EngineAssets::best_fit_normals.get_asset();

	UINT max_meshes = 1024 * 1024;

	commands_boxes = std::make_shared<virtual_gpu_buffer<Table::BoxInfo>>(RenderSystem::get().device(), max_meshes, counterType::HELP_BUFFER, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);
	visible_boxes = std::make_shared<virtual_gpu_buffer<UINT>>(RenderSystem::get().device(), max_meshes, counterType::NONE, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);
	meshes_ids = std::make_shared<virtual_gpu_buffer<UINT>>(RenderSystem::get().device(), max_meshes, counterType::HELP_BUFFER, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);
	meshes_invisible_ids = std::make_shared<virtual_gpu_buffer<UINT>>(RenderSystem::get().device(), max_meshes, counterType::HELP_BUFFER, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);
	for (int i = 0; i < 8; i++)
		commands_buffer[i] = std::make_shared<virtual_gpu_buffer<Table::CommandData>>(RenderSystem::get().device(), max_meshes, counterType::HELP_BUFFER, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);

		auto list = (RenderSystem::get().device().get_upload_list());
			
	//meshes_ids->buffer->get_native()->SetName(L"meshes_ids");
	{

		std::vector<unsigned int> data = { 3, 1, 0,
		2, 1, 3,
		0, 5, 4,
		1, 5, 0,
		3, 4, 7,
		0, 4, 3,
		1, 6, 5,
		2, 6, 1,
		2, 7, 6,
		3, 7, 2,
		6, 4, 5,
		7, 4, 6,
		};



		std::vector<vec4> verts(8);
		vec3 v0(-0.5, -0.5, 0.5);
		vec3 v1(0.5, 0.5, 0.5);
		verts[0] = vec4(-1.0f, 1.0f, -1.0f, 0);
		verts[1] = vec4(1.0f, 1.0f, -1.0f, 0);
		verts[2] = vec4(1.0f, 1.0f, 1.0f, 0);
		verts[3] = vec4(-1.0f, 1.0f, 1.0f, 0);
		verts[4] = vec4(-1.0f, -1.0f, -1.0f, 0);
		verts[5] = vec4(1.0f, -1.0f, -1.0f, 0);
		verts[6] = vec4(1.0f, -1.0f, 1.0f, 0);
		verts[7] = vec4(-1.0f, -1.0f, 1.0f, 0);
		index_buffer = Helpers::make_buffer<unsigned int>(RenderSystem::get().device(), data);

		index_buffer.resource->set_name("mesh_renderer::index_buffer");
		vertex_buffer = HAL::StructuredBufferView<vec4>(RenderSystem::get().device(), 8);

			vertex_buffer.resource->set_name("mesh_renderer::vertex_buffer");
		list->get_copy().update(vertex_buffer, 0, verts);

		// Written by CS_boxes (InterlockedMax on InstanceCount, IndexCount
		// restored alongside), zeroed per stage via clear_uav — needs UAV.
		draw_boxes_first = HAL::StructuredBufferView<DrawIndexedArguments>(RenderSystem::get().device(), 1, counterType::NONE, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);

	}


	{
		gather_meshes_args = HAL::StructuredBufferView<DispatchArguments>(RenderSystem::get().device(), 1, counterType::NONE, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);
		render_args        = HAL::StructuredBufferView<DispatchArguments>(RenderSystem::get().device(), 1, counterType::NONE, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);
		retest_args        = HAL::StructuredBufferView<DispatchArguments>(RenderSystem::get().device(), 1, counterType::NONE, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);
	}
	{
		Slots::GatherPipelineGlobal gather;
		gather.GetCommands() = meshes_ids->buffer.resource->create_view<HAL::FormattedBufferView<UINT, HAL::Format::R32_UINT>>(RenderSystem::get().device().get_static_gpu_data()).buffer;

		gather.GetMeshes_count() = meshes_ids->buffer.counter_view;

		gather_visible = gather.compile(RenderSystem::get().device().get_static_gpu_data());
		//	gather_visible = meshes_ids->buffer->help_buffer->get_resource_address();
	}

	{
		Slots::GatherPipelineGlobal gather;
		gather.GetCommands() = meshes_invisible_ids->buffer.resource->create_view<HAL::FormattedBufferView<UINT, HAL::Format::R32_UINT>>(RenderSystem::get().device().get_static_gpu_data()).buffer;
		gather.GetMeshes_count() = meshes_invisible_ids->buffer.counter_view;
		gather_invisible = gather.compile(RenderSystem::get().device().get_static_gpu_data());
		//	gather_invisible = meshes_invisible_ids->buffer->help_buffer->get_resource_address();
	}

	{
		Slots::GatherPipelineGlobal gather;
		gather.GetMeshes_count() = commands_boxes->buffer.counter_view;

		//gather.GetCommands() = // supposed to be null
		gather_boxes_commands = gather.compile(RenderSystem::get().device().get_static_gpu_data());
		//gather_boxes_commands = commands_boxes->buffer->help_buffer->get_resource_address();
	}

	{
		Slots::GatherMeshesBoxes gather_neshes_boxes;
		gather_neshes_boxes.GetInput_meshes() = commands_boxes->buffer;
		gather_neshes_boxes.GetVisible_boxes() = visible_boxes->buffer;

		gather_neshes_boxes.GetVisibleMeshes() = meshes_ids->buffer;
		gather_neshes_boxes.GetVisibleCount() = meshes_ids->buffer.counter_view;
		gather_neshes_boxes.GetRenderArgs() = render_args;

		gather_neshes_boxes.GetInvisibleMeshes() = meshes_invisible_ids->buffer;
		gather_neshes_boxes.GetInvisibleCount() = meshes_invisible_ids->buffer.counter_view;
		gather_neshes_boxes.GetRetestArgs() = retest_args;

		gather_neshes_boxes_compiled = gather_neshes_boxes.compile(RenderSystem::get().device().get_static_gpu_data());
	}

	{
		Slots::DrawBoxes draw_boxes;
		draw_boxes.GetInput_meshes() = commands_boxes->buffer;
		draw_boxes.GetVisible_meshes() = visible_boxes->buffer;
		draw_boxes.GetVertices() = vertex_buffer;

		draw_boxes_compiled = draw_boxes.compile(RenderSystem::get().device().get_static_gpu_data());
	}
	{
		Slots::GatherBoxes gather;
		gather.GetCulledMeshes() = commands_boxes->buffer;
		gather.GetCulledCount() = commands_boxes->buffer.counter_view;
		gather.GetVisible_boxes() = visible_boxes->buffer;
		gather.GetDrawBoxesArgs() = draw_boxes_first;
		gather.GetGatherMeshesArgs() = gather_meshes_args;

		gather.GetVisibleMeshes() = meshes_ids->buffer;
		gather.GetVisibleCount() = meshes_ids->buffer.counter_view;
		gather.GetRenderArgs() = render_args;

		gather_boxes_compiled = gather.compile(RenderSystem::get().device().get_static_gpu_data());
	}


			list->execute_and_wait();

}

renderer::~renderer()
{
}

void main_renderer::register_renderer(renderer::ptr r)
{
    renderers.insert(r);
}

void main_renderer::render(MeshRenderContext::ptr c, Scene::ptr obj)
{
    for (auto && r : renderers)
        r->render(c, obj);
}

void main_renderer::iterate(MESH_TYPE mesh_type, std::function<void(scene_object::ptr&)> f)
{
    for (auto && r : renderers)
        r->iterate(mesh_type, f);
}

bool mesh_renderer::add_object(Graphics::renderable* obj)
{
    auto instance = dynamic_cast<MeshAssetInstance*>(obj);
    return true;
}

