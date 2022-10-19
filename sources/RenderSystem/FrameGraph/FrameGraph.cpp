
#include "pch_render.h"

#include "FrameGraph.h"
import Graphics;

using namespace Graphics;



namespace FrameGraph
{
	void ResourceAllocInfo::add_pass(Pass* pass, ResourceFlags flags)
	{
		assert(pass);
		bool is_writer = check(flags & WRITEABLE_FLAGS);

		bool was_swap = false;

		if (is_writer || states.empty())
		{

			ResourceRWState state;
			state.write = is_writer;
			states.emplace_back(state);
			was_swap = true;
			last_writer = static_cast<int>(states.size() - 1);
		}
		else
		{
			if (states.back().write)
			{
				ResourceRWState state;
				state.write = false;
				states.emplace_back(state);
				was_swap = true;
			}
		}

		ResourceRWState& state = states.back();
		state.passes.emplace_back(pass);

		if (check(pass->flags & PassFlags::Compute))
			state.compute.emplace_back(pass);
		else
			state.graphics.emplace_back(pass);


		if (was_swap)
		{
			related.insert(related_read.begin(), related_read.end());
			related_read.clear();
		}

		pass->related.insert(related.begin(), related.end());
		related_read.insert(pass);
	}


	void ResourceAllocInfo::reset()
	{
		last_writer = 0;
		states.clear();
		related.clear();
		related_read.clear();
	}

	void ResourceAllocInfo::remove_inactive()
	{

		auto fn = [](Pass* pass) {return !pass->active(); };

		for (auto& state : states)
		{

			{
				const auto ret2 = std::ranges::remove_if(state.graphics, fn);
				state.graphics.erase(ret2.begin(), ret2.end());
			}

			{
				const auto ret2 = std::ranges::remove_if(state.compute, fn);
				state.compute.erase(ret2.begin(), ret2.end());
			}


			{
				const auto ret2 = std::ranges::remove_if(state.passes, fn);
				state.passes.erase(ret2.begin(), ret2.end());
			}

		}



	}
	TaskBuilder::TaskBuilder() : allocator(false), static_allocator(false)
	{

	}

	void TaskBuilder::begin(Pass* pass)
	{
		current_pass = pass;
	}

	void TaskBuilder::end(Pass* pass)
	{
		current_pass = nullptr;
	}

	void TaskBuilder::pass_texture(std::string name, Graphics::Texture::ptr tex, ResourceFlags flags)
	{
		auto tex_desc = tex->get_desc().as_texture();
		if (tex_desc.is2D())
		{
			Handlers::Texture h(name);
			create(h, { ivec3(0,0,0), Graphics::Format::UNKNOWN, 0 }, flags);
			auto& info = *h.info;
			info.passed = true;

			info.resource = tex;
			info.d3ddesc = tex->get_desc();
			passed_resources.insert(&info);

			h.desc.array_count = tex->get_desc().as_texture().ArraySize;
			h.desc.format = tex->get_desc().as_texture().Format;
			h.desc.mip_count = tex->get_desc().as_texture().MipLevels;
			h.desc.size = tex->get_desc().as_texture().Dimensions;

			h.init_view(info, *current_frame);

		}
		else if (tex_desc.is3D())
		{
			Handlers::Texture3D h(name);
			create(h, { ivec3(0,0,0), Graphics::Format::UNKNOWN, }, flags);
			auto& info = *h.info;
			info.passed = true;

			info.resource = tex;
			info.d3ddesc = tex->get_desc();
			passed_resources.insert(&info);

			h.desc.format = tex->get_desc().as_texture().Format;
			h.desc.mip_count = tex->get_desc().as_texture().MipLevels;
			h.desc.size = tex->get_desc().as_texture().Dimensions;

			h.init_view(info, *current_frame);
		}
		else
			assert(false);
	}

	void TaskBuilder::reset()
	{
		current_pass = nullptr;
		//	allocator.reset();

		for (auto& info : passed_resources)
		{
			info->resource = nullptr;
			info->view.reset();
			info->passed = false;
			info->valid_from = nullptr;
			info->valid_to = nullptr;
			info->valid_to_start = nullptr;
		}
		passed_resources.clear();
		//resources.clear();
	}



	/*void FrameContext::request_resources(UsedResources& resources, TaskBuilder& builder)
	{
		for (auto& uses : resources.textures)
		{
			textures[uses] = builder.request_texture(uses);
		}
	}
	*/

	Graphics::CommandList::ptr& FrameContext::get_list()
	{

		if (!list)
		{
			Graphics::CommandListType type = pass->get_type();

			list = frame->start_list(pass->name, type);

			for (auto info : pass->used.resource_creations)
			{
				if (!info->alloc_ptr.handle) continue;

				if (!info->enabled)
					continue;

				list->transition(nullptr, info->resource.get());

				//if (!handle->info->texture) continue;
			/* {
					auto &tex = info->texture;

					if (tex)
						list->transition(nullptr, tex.resource.get());

				}
				{
					auto& tex = info->buffer;

					if (tex)
						list->transition(nullptr, tex.resource.get());
				}
				*/
			}
		}
		return list;
	}

	void FrameContext::begin(Pass* pass, Graphics::FrameResources::ptr& frame) {

		this->pass = pass;
		this->frame = frame;
	}

	void FrameContext::end()
	{
		if (list)list->end();
	}


	void FrameContext::execute()
	{
		if (list)list->execute();
		list = nullptr;
	}


	void Pass::compile(TaskBuilder& builder)
	{
		//	builder.begin(this);
		//	context.request_resources(used, builder);
		//	builder.end(this);
	}

	void Pass::wait()
	{
		if (!enabled || !renderable)  return;

		render_task.wait();
	}
	void Pass::execute()

	{

		if (!enabled || !renderable) {
			return;

		}

		{
			PROFILE(L"execute");
			context.execute();
		}
	}
	void Graph::start_new_frame()
	{
		{
			PROFILE(L"begin_frame");

			builder.current_frame = builder.frames.begin_frame();
		}
	}

	void Graph::setup()
	{

		PROFILE(L"Graphsetup");

		for (auto& pass : passes)
		{
			pass->enabled = false;

			pass->renderable = pass->setup(builder);
		}

		for (auto& pair : builder.alloc_resources)
		{
			if (pair.second.passed) continue;

			// here need to delete unused info
		//	if (pair.second.frame_id != builder.current_frame->get_frame())
			//	continue;

			auto info = &pair.second;

			info->enabled = false;

		}

		std::function<void(ResourceAllocInfo&, int)> process_resource;

		process_resource = [&, this](ResourceAllocInfo& info, UINT pass_id) {
			info.enabled = true;

			for (auto& s : info.states)
			{
				if (s.write)
				{

					auto& pass = s.passes.front();

					if (pass->enabled) continue;

					if (!check(info.flags & ResourceFlags::Static) && pass->id > pass_id) continue;
					pass->enabled = true;

					for (auto& info : pass->used.resources)
					{
						process_resource(*info, pass->id);
					}
				}
			}

		};

		for (auto res : builder.alloc_resources)
		{
			if (check(res.second.flags & ResourceFlags::Required))
				process_resource(res.second, (int)passes.size());
		}


		for (auto pass : required_passes)
		{
			pass->enabled = true;
			for (auto& info : pass->used.resources)
			{
				process_resource(*info, pass->id);
			}
		}

		for (auto pass : passes)
		{
			if (!pass->active()) continue;
			auto pass_ptr = pass.get();

			pass->dependency_level = 0;
			enabled_passes.emplace_back(pass_ptr);

			if (!optimize)
			{
				pass->flags = pass->flags & ~(PassFlags::Compute);
			}

		}

		if (optimize)
		{
			bool sorted = false;

			bool cyclic = false;
			while (!sorted)
			{

				sorted = true;
				for (auto pass : enabled_passes)
				{

					for (auto related : pass->related)
					{
						if (!related->active()) continue;

						if (related == pass)
						{
							cyclic = true;
							continue;
						}

						if (pass->dependency_level <= related->dependency_level)
						{
							pass->dependency_level = related->dependency_level + 1;
							sorted = false;
						}
					}
				}
			}

			for (auto pass : enabled_passes | std::ranges::views::reverse)
			{

				for (auto dep : pass->related)
				{
					if (!dep->active()) continue;

					dep->graphic_count += pass->graphic_count;
					dep->compute_count += pass->compute_count;

					if (check(pass->flags & PassFlags::Compute))
						dep->compute_count++;
					else
						dep->graphic_count++;
				}
			}

			enabled_passes.sort(
				[](const Pass* a, const Pass* b)
				{

					if (a->dependency_level == b->dependency_level)
					{
						if (!check(a->flags & PassFlags::Compute) && !check(b->flags & PassFlags::Compute))
						{
							return a->compute_count > b->compute_count;
						}

						return check(a->flags & PassFlags::Compute) > check(b->flags & PassFlags::Compute);
					}
					return (a->dependency_level < b->dependency_level);
				});
		}

		int i = 0;



		for (auto pass : enabled_passes)
		{
			pass->call_id = i++;
			pass->wait_pass = nullptr;
			pass->prev_pass = nullptr;
		}


		for (auto& pair : builder.alloc_resources)
		{
			auto& info = pair.second;
			if (!info.enabled) continue;

			info.remove_inactive();
			info.valid_from = nullptr;// info.states.front().passes.front();
			info.valid_to = nullptr;//info.states.back().passes.back();

			for (auto state : info.states)
			{
				for (auto pass : state.passes)
				{
					if (!pass->active()) continue;

					if (!info.valid_from || info.valid_from->call_id > pass->id)
					{
						info.valid_from = pass;
					}
				}
				if (info.valid_from) break;
			}

			for (auto state : info.states | std::ranges::views::reverse)
			{
				for (auto pass : state.passes | std::ranges::views::reverse)
				{
					if (!pass->active()) continue;

					if (!info.valid_to || info.valid_to->call_id < pass->id)
					{
						info.valid_to = pass;
					}
				}
				if (info.valid_to) break;
			}



			for (int i = 1; i < info.states.size(); i++)
			{
				auto state = info.states[i];
				auto prev_state = info.states[i - 1];

				Pass* prev_pass_graphics = prev_state.graphics.empty() ? nullptr : prev_state.graphics.back();
				Pass* prev_pass_compute = prev_state.compute.empty() ? nullptr : prev_state.compute.back();

				Pass* pass_graphics = state.graphics.empty() ? nullptr : state.graphics.front();
				Pass* pass_compute = state.compute.empty() ? nullptr : state.compute.front();

				if (pass_graphics && prev_pass_compute)
				{
					if (!pass_graphics->wait_pass || pass_graphics->wait_pass->call_id < prev_pass_compute->call_id)
						pass_graphics->wait_pass = prev_pass_compute;
				}


				if (pass_compute && prev_pass_graphics)
				{
					if (!pass_compute->wait_pass || pass_compute->wait_pass->call_id < prev_pass_graphics->call_id)
						pass_compute->wait_pass = prev_pass_graphics;
				}
			}

		}


		Pass* last_graphics = nullptr;
		Pass* last_compute = nullptr;

		for (auto pass : enabled_passes)
		{
			Graphics::CommandListType type = pass->get_type();

			bool sync_to_graphics = last_graphics && type == Graphics::CommandListType::DIRECT;
			bool sync_to_compute = last_compute && type == Graphics::CommandListType::COMPUTE;



			if (type == Graphics::CommandListType::COMPUTE)
				pass->prev_pass = last_compute;
			else
				pass->prev_pass = last_graphics;

			if (sync_to_compute)
			{
				for (auto& info : last_compute->used.resources)
				{
					if (info->valid_to == last_compute)
					{
						info->valid_to = pass;
					}
				}
			}


			if (sync_to_graphics)
			{
				for (auto& info : last_graphics->used.resources)
				{

					if (info->valid_to == last_graphics)
					{
						info->valid_to = pass;
					}
				}
			}

			if (pass->wait_pass && last_compute)
			{
				for (auto& info : last_compute->used.resources)
				{
					if (info->valid_to == pass->wait_pass)
					{
						info->valid_to = pass;
					}
				}
			}

			if (pass->wait_pass && last_graphics)
			{
				for (auto& info : last_graphics->used.resources)
				{
					if (info->valid_to == pass->wait_pass)
					{
						info->valid_to = pass;
					}
				}
			}


			if (type == Graphics::CommandListType::COMPUTE)
				last_compute = pass;
			else
				last_graphics = pass;

		}
	}

	void Graph::compile(int frame)
	{
		PROFILE(L"Graphcompile");
		//builder.allocator.begin_frame(frame);

		builder.current_alloc = &builder.frame_allocs[frame];


		builder.create_resources();

		for (auto& f : pre_run)
			f(*this);

		for (auto& pass : enabled_passes)
			pass->compile(builder);
	}

	void Graph::render()
	{
		PROFILE(L"Graphrender");


		{
			PROFILE(L"passes");

			for (auto& pass : enabled_passes)
				pass->render(builder.current_frame);

		}


	}


	void Graph::commit_command_lists()
	{
		{
			PROFILE(L"wait");

			for (auto& pass : enabled_passes)
			{
				pass->wait();
			}


		}


		//Profiler::get().enabled = true;


		{
			PROFILE(L"execute");

			/*	for (auto& pass : enabled_passes | std::ranges::views::reverse)
				{
					auto commandList = pass->context.list;
					if (!commandList) continue;

					Graphics::CommandListType list_type = commandList->get_type();

					if (pass->wait_pass)
					{
						auto waitCommandList = pass->wait_pass->context.list;

						//if(list_type == Graphics::CommandListType::COMPUTE)
							waitCommandList->prepare_transitions(commandList.get(), false);
					}
					else

						if (pass->prev_pass)
					{
						auto waitCommandList = pass->prev_pass->context.list;

					//	if (list_type == Graphics::CommandListType::DIRECT)
						if(waitCommandList) waitCommandList->prepare_transitions(commandList.get(), true);
					}


				}*/
			if (optimize)
				for (auto& pair : builder.alloc_resources)
				{
					auto& info = pair.second;
					if (!info.enabled) continue;

					auto& resource = info.resource;

					if (!resource) continue;

					//		if(false)
					for (auto& state : info.states)
					{


						HAL::SubResourcesGPU merged_state;

						merged_state.subres.resize(resource->get_state_manager().get_subres_count());


						for (auto& pass : state.passes)
						{
							auto commandList = pass->context.list;
							if (!commandList) continue;

							auto& cpu_state = resource->get_state_manager().get_cpu_state(commandList.get());


							merged_state.merge(cpu_state);
							/*
							for (int i = 0; i < merged_state.size();i++)
							{
								if (!cpu_state.subres[i].used) continue;
								merged_state[i] = merge_state(merged_state[i], cpu_state.subres[i].get_first_state());
							}*/
						}

						for (auto& pass : state.passes)
						{
							auto commandList = pass->context.list;
							if (!commandList) continue;

							auto& cpu_state = resource->get_state_manager().get_cpu_state(commandList.get());

							cpu_state.prepare_for(commandList->get_type(), merged_state);
							/*for (int i = 0; i < merged_state.size(); i++)
							{
								if (!cpu_state.subres[i].used)	continue;
								cpu_state.subres[i].first_transition->wanted_state = merge_state(merged_state[i], cpu_state.subres[i].first_transition->wanted_state);
								assert(cpu_state.subres[i].first_transition->wanted_state == merged_state[i]);
							}*/
						}

						// TODO: propagate first state to next writer
					}


					for (int i = 1; i < info.states.size(); i++)
					{
						auto state = info.states[i];
						auto prev_state = info.states[i - 1];

						Pass* prev_pass_graphics = prev_state.graphics.empty() ? nullptr : prev_state.graphics.back();
						Pass* prev_pass_compute = prev_state.compute.empty() ? nullptr : prev_state.compute.back();

						Pass* pass_graphics = state.graphics.empty() ? nullptr : state.graphics.front();
						Pass* pass_compute = state.compute.empty() ? nullptr : state.compute.front();

						if (pass_graphics && prev_pass_compute)
						{
							if (!pass_graphics->wait_pass || pass_graphics->wait_pass->call_id < prev_pass_compute->call_id)
								pass_graphics->wait_pass = prev_pass_compute;
						}

						if (pass_compute && prev_pass_graphics)
						{
							if (!pass_compute->wait_pass || pass_compute->wait_pass->call_id < prev_pass_graphics->call_id)
								pass_compute->wait_pass = prev_pass_graphics;

							auto prevCL = prev_pass_graphics->context.list;
							auto curCL = pass_compute->context.list;


							if (prevCL && curCL)prevCL->merge_transition(curCL.get(), info.resource.get());
						}

						if (pass_compute && prev_pass_compute)
						{
							auto prevCL = prev_pass_compute->context.list;
							auto curCL = pass_compute->context.list;


							if (prevCL && curCL)prevCL->merge_transition(curCL.get(), info.resource.get());
						}
						if (pass_graphics && prev_pass_graphics)
						{
							auto prevCL = prev_pass_graphics->context.list;

							auto curCL = pass_graphics->context.list;
							if (prevCL && curCL)
								prevCL->merge_transition(curCL.get(), info.resource.get());

						}

					}

					auto gpu_state = info.resource->get_state_manager().copy_gpu();

					for (int i = 0; i < info.states.size(); i++)
					{
						auto state = info.states[i];
						for (auto pass : state.passes)
						{
							auto commandList = pass->context.list;
							if (!commandList) continue;

							info.resource->get_state_manager().prepare_state(commandList.get(), gpu_state);
						}
					}
				}

				{
					PROFILE(L"compile");

					std::list<std::future<void>> tasks;
					for (auto& pass : enabled_passes)
					{
						auto commandList = pass->context.list;
						if (!commandList) continue;

						tasks.emplace_back(scheduler::get().enqueue([commandList, pass]() {
							commandList->compile();
							}, std::chrono::steady_clock::now()));


					}


					for (auto& t : tasks)
					{
						t.wait();
					}

				}


				{
					std::list<Pass*> graphics;
					std::list<Pass*> compute;

					UINT transitions = 0;

					enum_array<Graphics::CommandListType, enum_array<Graphics::CommandListType, UINT>> call_ids = {};


					auto flush = [&]()
					{
						for (auto pass : compute)
						{
							auto commandList = pass->context.list;
							if (commandList)
							{


								transitions += commandList->transition_count;
								pass->fence_end = commandList->execute().get();
							}
							else
							{
								pass->fence_end = Graphics::Device::get().get_queue(pass->get_type())->get_last_fence();
							}
						}

						for (auto pass : graphics)
						{
							auto commandList = pass->context.list;
							if (commandList)
							{
								transitions += commandList->transition_count;
								pass->fence_end = commandList->execute().get();
							}
							else
							{
								pass->fence_end = Graphics::Device::get().get_queue(pass->get_type())->get_last_fence();
							}

						}
						graphics.clear();
						compute.clear();
					};


					for (auto& pass : enabled_passes)
					{

						Graphics::CommandListType list_type = pass->get_type();

						if (pass->wait_pass)
						{
							auto type = pass->wait_pass->get_type();

							if (call_ids[list_type][type] < pass->wait_pass->call_id + 1)
							{
								flush();
								Graphics::Device::get().get_queue(list_type)->gpu_wait(pass->wait_pass->fence_end);

								call_ids[list_type][type] = pass->wait_pass->call_id + 1;
							}

						}

						if (list_type == Graphics::CommandListType::DIRECT)
							graphics.emplace_back(pass);
						else
							compute.emplace_back(pass);

						call_ids[list_type][list_type] = pass->call_id + 1;
					}
					flush();

				}

		}



		for (auto& pair : builder.alloc_resources)
		{
			auto info = &pair.second;
			if (!check(info->flags & ResourceFlags::Static))
			{
				info->resource = nullptr;
				info->view = nullptr;
			}

			if (info->heap_type != Graphics::HeapType::DEFAULT)
			{
				info->alloc_ptr.Free();
			}

		}

	}

	void Graph::reset()
	{

		for (auto& pass : enabled_passes)
		{
			pass->wait();
		}

		passes.clear();
		required_passes.clear();
		enabled_passes.clear();
		builder.reset();

		pre_run.clear();
	}



	void TaskBuilder::init(ResourceAllocInfo& info, std::string name, ResourceFlags flags)
	{
		info.reset();
		info.flags = flags;
		info.name = name;
		info.frame_id = current_frame->get_frame();
		info.is_new = false;
		info.valid_from = info.valid_to = info.valid_to_start = nullptr;

		if (current_pass) {
			current_pass->used.resources.insert(&info);
			current_pass->used.resource_creations.insert(&info);
			current_pass->used.resource_flags[&info] = flags;
			info.add_pass(current_pass, flags);
		}
	}

	void TaskBuilder::init_pass(ResourceAllocInfo& info, ResourceFlags flags)
	{
		current_pass->used.resources.insert(&info);
		current_pass->used.resource_flags[&info] = flags;
		info.is_new = false;
		info.flags = info.flags | flags;
		info.add_pass(current_pass, flags);

	}


	void TaskBuilder::create_resources()
	{

		struct Events
		{
			std::set<ResourceAllocInfo*> create;
			std::set<ResourceAllocInfo*> free;
		};

		std::map<int, Events> events;

		for (auto& pair : alloc_resources)
		{
			if (pair.second.passed) continue;

			// here need to delete unused info
			if (pair.second.frame_id != current_frame->get_frame())
				continue;
			auto info = &pair.second;
			if (!info->enabled)
				continue;




			info->handler->init(*info);
			info->heap_type = Graphics::HeapType::DEFAULT;

			if (check(info->flags & ResourceFlags::GenCPU))
			{
				info->heap_type = Graphics::HeapType::UPLOAD;
			}

			if (check(info->flags & ResourceFlags::ReadCPU))
			{
				info->heap_type = Graphics::HeapType::READBACK;
			}

			if (check(info->flags & ResourceFlags::Static)) continue;

			if (info->heap_type != Graphics::HeapType::DEFAULT)
			{
				//	auto creation_info = Graphics::Device::get().get_alloc_info(info->d3ddesc);
				//	auto alloc_ptr = allocator.alloc(creation_info.size, creation_info.alignment, creation_info.flags, info->heap_type);

				//	info->need_recreate = info->alloc_ptr != alloc_ptr;
				//	info->alloc_ptr = alloc_ptr;
			}
			else
			{
				assert(info->valid_from != info->valid_to);
				assert(pair.second.valid_from->active());
				assert(pair.second.valid_to->active());
				events[pair.second.valid_from->call_id].create.insert(info);
				events[pair.second.valid_to->call_id].free.insert(info);
			}

			//	if (info->heap_type != Graphics::HeapType::DEFAULT || check(info->flags & ResourceFlags::Static)) continue;


		}

		{
			PROFILE(L"allocate memory");


			for (auto [id, e] : events)
			{
				for (auto info : e.free)
				{
					info->alloc_ptr.handle.Free();
				}


				for (auto info : e.create)
				{

					auto creation_info = Graphics::Device::get().get_alloc_info(info->d3ddesc);
					info->alloc_ptr = allocator.alloc(creation_info.size, creation_info.alignment, HAL::MemoryType::COMMITED, info->heap_type);
				}
			}
		}

		{
			PROFILE(L"create resources");
			int id = 0;

			for (auto& pair : alloc_resources)
			{
				auto info = &pair.second;

				// here need to delete unused info
				if (pair.second.frame_id != current_frame->get_frame())
					continue;

				if (info->passed) continue;
				if (!info->enabled)
					continue;

				if (info->alloc_ptr.handle)
				{
					auto& res = info->resource_places[info->alloc_ptr];

					if (!res || res->get_desc() != info->d3ddesc)
					{
						res = std::make_shared<HAL::Resource>(info->d3ddesc, info->alloc_ptr);
						res->set_name(info->name);
					}

					if (info->resource != res)
					{
						info->is_new = true;
					}
					info->resource = res;
				}
				else
				{
					if (info->heap_type == Graphics::HeapType::UPLOAD)
					{
						info->resource = std::make_shared<HAL::Resource>(info->d3ddesc, info->heap_type);
					}
					else if (info->heap_type == Graphics::HeapType::READBACK)
					{
						info->resource = std::make_shared<HAL::Resource>(info->d3ddesc, info->heap_type);
					}
					else if (!info->resource || info->resource->get_desc() != info->d3ddesc)
					{
						assert(info->name != "id_buffer");
						info->resource = std::make_shared<HAL::Resource>(info->d3ddesc, info->heap_type);
						info->is_new = true;
					}

				}
				info->resource->set_name(info->name);

				info->handler->init_view(*info, *current_frame);
				id++;
			}
		}


	}


	bool ResourceHandler::is_new()
	{
		return info->is_new;
	}

	void ResourceHandler::changed()
	{
		info->flags = info->flags | ResourceFlags::Changed;
	}

	bool ResourceHandler::is_changed()
	{
		return check(info->flags & ResourceFlags::Changed);
	}
}