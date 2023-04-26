module HAL:FrameGraph;
import HAL;
import Core;

using namespace HAL;



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
		//	assert(pass->prev_passes.empty());

		pass->prev_passes.insert(related.begin(), related.end());
		related_read.insert(pass);

		for (auto p : related)
		{
			p->next_passes.insert(pass);
			//pass->sync_state.max(p);
		}
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



			for(auto p:state.passes)

			{
			state.from.min(p);
			state.to.max(p);
			}
		}


	
			{	
			auto fn = [](ResourceRWState& s) {return s.passes.empty(); };

				const auto ret2 = std::ranges::remove_if(states, fn);
				states.erase(ret2.begin(), ret2.end());
			}

	}
	TaskBuilder::TaskBuilder() : frames(Device::get()), allocator(HAL::Device::get().get_heap_factory(), false)
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

	void TaskBuilder::pass_texture(std::string name, HAL::Texture::ptr tex, ResourceFlags flags)
	{
		auto tex_desc = tex->get_desc().as_texture();
		if (tex_desc.is2D())
		{
			Handlers::Texture h(name);
			create(h, { ivec3(0,0,0), HAL::Format::UNKNOWN, 0 }, flags);
			auto& info = *h.info;
			info.passed = true;

			info.resource = tex->resource;
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
			create(h, { ivec3(0,0,0), HAL::Format::UNKNOWN, }, flags);
			auto& info = *h.info;
			info.passed = true;

			info.resource = tex->resource;
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
		current_frame = nullptr;
		current_pass = nullptr;
		//	allocator.reset();
		for (auto& pair : alloc_resources)
		{
			pair.second.view = nullptr;
		}
		for (auto& info : passed_resources)
		{
			info->resource = nullptr;
			info->view.reset();
			info->passed = false;

			info->used_begin.reset();
			info->used_end.reset();
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

	HAL::CommandList::ptr& FrameContext::get_list()
	{

		if (!list)
		{
			HAL::CommandListType type = pass->get_type();

			list = (frame->start_list(pass->name, type));

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

	void FrameContext::begin(Pass* pass, HAL::FrameResources::ptr& frame) {

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

		for (auto& res : builder.alloc_resources)
		{
			if (check(res.second.flags & ResourceFlags::Required))
				process_resource(res.second, (int)passes.size());
		}


		for (auto& pass : builder.required_passes)
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
			builder.enabled_passes.emplace_back(pass_ptr);

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
				for (auto pass : builder.enabled_passes)
				{

					for (auto prev_passes : pass->next_passes)
					{
						if (!prev_passes->active()) continue;

						if (prev_passes == pass)
						{
							cyclic = true;
							continue;
						}

						if (pass->dependency_level >= prev_passes->dependency_level)
						{
							prev_passes->dependency_level = pass->dependency_level + 1;
							sorted = false;
						}
					}
				}
			}

			for (auto pass : builder.enabled_passes | std::ranges::views::reverse)
			{

				for (auto dep : pass->prev_passes)
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

			builder.enabled_passes.sort(
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


			std::list<Pass*> graphics;
			std::list<Pass*> compute;
			std::list<Pass*> new_enabled_passes;



			for (auto pass : builder.enabled_passes)
			{
				if (check(pass->flags & PassFlags::Compute))
					compute.emplace_back(pass);
				else
					graphics.emplace_back(pass);
			}


			std::function<void(Pass* pass)> insert_pass;

			insert_pass = [&](Pass* pass) {
				for (auto prev : pass->prev_passes)
				{
					if (!prev->inserted)	insert_pass(prev);

				}
				new_enabled_passes.emplace_back(pass);
				pass->inserted = true;
			};

			for (auto pass : compute)
			{
				if (!pass->inserted) insert_pass(pass);
			}


			for (auto pass : graphics)
			{
				if (!pass->inserted) insert_pass(pass);
			}


			builder.enabled_passes = new_enabled_passes;
		}

		int i = 1;



		for (auto pass : builder.enabled_passes)
		{
			pass->call_id = i++;
			pass->wait_pass = nullptr;
			pass->prev_pass = nullptr;


			for (auto s : pass->prev_passes)
			{
				if (!s->active()) continue;

				pass->sync_state.max(s);
			}
		}


		for (auto& pair : builder.alloc_resources)
		{
			auto& info = pair.second;
			if (!info.enabled) continue;

			info.remove_inactive();

			info.used_begin.reset();
			info.used_end.reset();

			for (auto state : info.states)
			{
				for (auto pass : state.passes)
				{
					info.used_begin.min(pass);
					info.used_end.max(pass);
				}
			}
		}
	}

	void Graph::compile(int frame)
	{
		PROFILE(L"Graphcompile");


		builder.create_resources();

		for (auto& f : pre_run)
			f(*this);

		for (auto& pass : builder.enabled_passes)
			pass->compile(builder);
	}

	void Graph::render()
	{
		PROFILE(L"Graphrender");


		{
			PROFILE(L"passes");

			for (auto& pass : builder.enabled_passes)
				pass->render(builder.current_frame);

		}


	}


	void Graph::commit_command_lists()
	{
		{
			PROFILE(L"wait");

			for (auto& pass : builder.enabled_passes)
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

					HAL::CommandListType list_type = commandList->get_type();

					if (pass->wait_pass)
					{
						auto waitCommandList = pass->wait_pass->context.list;

						//if(list_type == HAL::CommandListType::COMPUTE)
							waitCommandList->prepare_transitions(commandList.get(), false);
					}
					else

						if (pass->prev_pass)
					{
						auto waitCommandList = pass->prev_pass->context.list;

					//	if (list_type == HAL::CommandListType::DIRECT)
						if(waitCommandList) waitCommandList->prepare_transitions(commandList.get(), true);
					}


				}*/

			auto find_pass = [&](SyncState from, SyncState to, CommandListType wanted_type)->Pass*{

				for (auto& pass : builder.enabled_passes)
				{
					auto type = pass->get_type();

					auto commandList = pass->context.list;
							if (!commandList) continue;


					if (!IsCompatible(type, wanted_type)) continue;
					if (from.is_in_sync(pass)) continue;

					if (!to.is_in_sync(pass)) continue;

					return pass;
				}


				return nullptr;

			};
			if (optimize)
				for (auto& pair : builder.alloc_resources)
				{
					auto& info = pair.second;
					if (!info.enabled) continue;

					auto& resource = info.resource;

					if (!resource) continue;

				if (info.heap_type != HAL::HeapType::DEFAULT) continue;

					// merge resourcestate access in a same read or write state
					for (auto& state : info.states)
					{
						state.merged_state.subres.resize(resource->get_state_manager().get_subres_count());

						// calculate merged state
						for (auto& pass : state.passes)
						{
							auto commandList = pass->context.list;
							if (!commandList) continue;

							auto& cpu_state = resource->get_state_manager().get_cpu_state(commandList.get());
							state.merged_state.merge(cpu_state);
						}

						// propagate merged state through passes
						for (auto& pass : state.passes)
						{
							auto commandList = pass->context.list;
							if (!commandList) continue;
							auto& cpu_state = resource->get_state_manager().get_cpu_state(commandList.get());
							cpu_state.prepare_for(commandList->get_type(), state.merged_state);							
						}
					}

					// link statee between passes
					for (uint i = 0; i < info.states.size(); i++)
					{
						auto& state = info.states[i];
						if (!state.write) continue;

						assert(state.passes.size()==1);
						auto pass = state.passes.front();
							auto commandList = pass->context.list;
							if (!commandList) continue;

									HAL::CommandListType list_type = pass->get_type();


						if(i>0&&!info.states[i-1].write)
						{
							auto prev_state = info.states[i];
							auto best_type = prev_state.merged_state.get_best_list_type();

					//		assert(IsCompatible(list_type,best_type));
							if(IsCompatible(list_type,best_type))
							info.resource->get_state_manager().prepare_state(commandList.get(), prev_state.merged_state);
							else
								Log::get()<<"unwanted prev transitions " << pass->name << " "<<info.resource->name <<Log::endl; 
						}


						if(i<info.states.size()-1/*&&!info.states[i+1].write*/)
						{
							auto next_state = info.states[i+1];

							auto best_type = Merge(next_state.merged_state.get_best_list_type(),state.merged_state.get_best_list_type());
				
					//	assert(IsCompatible(list_type,best_type));
							if(IsCompatible(list_type,best_type))
							info.resource->get_state_manager().prepare_after_state(commandList.get(), next_state.merged_state);
							else
							{
								auto best_pass = find_pass(state.to, next_state.from, best_type);
								assert(best_pass);

								info.resource->get_state_manager().prepare_after_state(best_pass->context.list.get(), next_state.merged_state);
							}
						//		Log::get()<<"unwanted next transitions " << pass->name << " "<<info.resource->name <<Log::endl; 
						
						}
					}
				
	

						// link end to start transition
						if (info.states.size() > 0)
						{			
					HAL::SubResourcesGPU first_state=info.states.data()->merged_state;
							auto last_state = info.states[info.states.size() - 1];


							Pass* last_pass = nullptr;
							for (auto pass : last_state.passes)
							{
								if (!last_pass) last_pass = pass;

								if (pass->sync_state.is_in_sync(last_pass,true))
								{
									last_pass = pass;
								}
							}


							if (first_state.is_valid()&&last_pass&&!info.passed)
							{
								auto commandList = last_pass->context.list;
								if (!commandList) continue;
											HAL::CommandListType list_type = last_pass->get_type();

								auto best_type = Merge(first_state.get_best_list_type(),list_type);
				
								best_type = Merge(last_state.merged_state.get_best_list_type(),best_type);
				
							if(IsCompatible(list_type,best_type))
								info.resource->get_state_manager().prepare_after_state(commandList.get(), first_state);
							else if(first_state.get_best_list_type() == CommandListType::COMPUTE)
							{
							Log::get()<<"unwanted window transitions " <<info.resource->name <<Log::endl; 
							}
							}

						


					}
				}

				{
					PROFILE(L"compile");

					std::list<std::future<void>> tasks;
					for (auto& pass : builder.enabled_passes)
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

					enum_array<CommandListType, SyncState> queued_state;

					//enum_array<HAL::CommandListType, enum_array<HAL::CommandListType, UINT>> call_ids = {};

					for (auto& pass : builder.enabled_passes)
					{
						HAL::CommandListType list_type = pass->get_type();

						auto commandList = pass->context.list;


						if (commandList)
						{

							for (auto sync_pass : pass->sync_state.values)
							{

								if (!sync_pass) continue;
								
								HAL::CommandListType other_type = sync_pass->get_type();

								if (!queued_state[list_type].is_in_sync(sync_pass, true))
								{
									assert(other_type != list_type);

									HAL::Device::get().get_queue(list_type)->gpu_wait(sync_pass->fence_end);

									queued_state[list_type].max(sync_pass);

								}
							}


							pass->fence_end = commandList->execute();
						}
						else
						{
							auto prev_pass=queued_state[list_type].values[list_type];
							if(prev_pass)pass->fence_end = prev_pass->fence_end;

						}

						queued_state[list_type].max(pass);

					}

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

			if (info->heap_type != HAL::HeapType::DEFAULT)
			{
				info->alloc_ptr.Free();
			}

		}

	}

	void Graph::reset()
	{

		for (auto& pass : builder.enabled_passes)
		{
			pass->wait();
		}

		passes.clear();
		builder.required_passes.clear();
		builder.enabled_passes.clear();
		builder.reset();

		pre_run.clear();
		slot_setters.clear();

	}



	void TaskBuilder::init(ResourceAllocInfo& info, std::string name, ResourceFlags flags)
	{
		info.reset();
		info.flags = flags;
		info.name = name;
		info.frame_id = current_frame->get_frame();
		info.is_new = false;
		//info.valid_from = info.valid_to = info.valid_to_start = nullptr;

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
			std::set<ResourceAllocInfo*> free_before;
			std::set<ResourceAllocInfo*> free_after;

		};

		std::map<int, Events> events;
		std::set<ResourceAllocInfo*> non_deleted;
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
			info->heap_type = HAL::HeapType::DEFAULT;

			if (check(info->flags & ResourceFlags::GenCPU))
			{
				info->heap_type = HAL::HeapType::UPLOAD;
			}

			if (check(info->flags & ResourceFlags::ReadCPU))
			{
				info->heap_type = HAL::HeapType::READBACK;
			}

			if (check(info->flags & ResourceFlags::Static)) continue;

			if (info->heap_type != HAL::HeapType::DEFAULT)
			{
				//	auto creation_info = HAL::Device::get().get_alloc_info(info->d3ddesc);
				//	auto alloc_ptr = allocator.alloc(creation_info.size, creation_info.alignment, creation_info.flags, info->heap_type);

				//	info->need_recreate = info->alloc_ptr != alloc_ptr;
				//	info->alloc_ptr = alloc_ptr;
			}
			else
			{

				// if the resource is temp, first pass should create it -> write
				assert(info->states[0].passes.size() == 1);


				const Pass* best_creation_pass = info->states.front().passes.front();
				const Pass* best_deletion_pass = nullptr;

				events[best_creation_pass->call_id].create.insert(info);


				//if (info->states.back().passes.size() == 1)
				//{
				//	best_deletion_pass = info->states.back().passes.front();
				//	events[best_creation_pass->call_id].free_after.insert(info);
				//}
				//else // find first synced pass
				for (auto pass : enabled_passes)
				{
					if (info->used_end.is_in_sync(pass->sync_state))

					{
						best_deletion_pass = pass;
						events[best_deletion_pass->call_id].free_before.insert(info);
						break;
					}

				}
				if (!best_deletion_pass)
					non_deleted.insert(info);
			}
		}

		{
			PROFILE(L"allocate memory");


			for (auto [id, e] : events)
			{
				for (auto info : e.free_before)
				{
					info->alloc_ptr.handle.Free();
				}


				for (auto info : e.create)
				{

					auto creation_info = HAL::Device::get().get_alloc_info(info->d3ddesc);
					HeapIndex index = { HAL::MemoryType::COMMITED , info->heap_type };

					info->alloc_ptr = allocator.alloc(creation_info.size, creation_info.alignment, index);
				}


				for (auto info : e.free_after)
				{
					info->alloc_ptr.handle.Free();
				}
			}



			for (auto info : non_deleted)
			{
				info->alloc_ptr.handle.Free();
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
					if (info->heap_type == HAL::HeapType::UPLOAD)
					{
						info->resource = std::make_shared<HAL::Resource>(info->d3ddesc, info->heap_type);
					}
					else if (info->heap_type == HAL::HeapType::READBACK)
					{
						info->resource = std::make_shared<HAL::Resource>(info->d3ddesc, info->heap_type);
					}
					else if (!info->resource || info->resource->get_desc() != info->d3ddesc)
					{
						info->resource = std::make_shared<HAL::Resource>(info->d3ddesc, info->heap_type);
						info->is_new = true;
					}

				}

				info->resource->debug = info->name=="GBuffer_DepthPrev";
				info->resource->set_name(info->name);
				info->resource->get_state_manager().manual_controlled = true;
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


	void SyncState::set_synced(const Pass* pass)
	{
		HAL::CommandListType type = pass->get_type();

		auto& v = values[type];


		if (!v || v->call_id > pass->call_id)
		{
			v = pass;
		}
	}


	bool SyncState::is_in_sync(const SyncState& state)
	{

		for (auto type : magic_enum::enum_values<CommandListType>())
		{

			auto& v = values[type];
			auto& pass = state.values[type];

			auto a = v ? v->call_id : 0;
			auto b = pass ? pass->call_id : 0;


			if (a > b) return false;

		}

		return true;
	}

	bool SyncState::is_in_sync(const Pass* pass, bool equal)
	{
		HAL::CommandListType type = pass->get_type();

		auto& v = values[type];

		auto a = v ? v->call_id : 0;
		if (equal)return pass->call_id <= a;
		return pass->call_id < a;
	}

	void SyncState::min(const Pass* pass)
	{
		HAL::CommandListType type = pass->get_type();

		auto& v = values[type];

		if (!v || v->call_id > pass->call_id) v = pass;
	}
	void SyncState::max(const Pass* pass)
	{
		HAL::CommandListType type = pass->get_type();

		auto& v = values[type];

		if (!v || v->call_id < pass->call_id) v = pass;
	}

	void SyncState::min(const SyncState& state)
	{

		for (auto type : magic_enum::enum_values<CommandListType>())
		{
			auto& v = values[type];
			auto& pass = state.values[type];
			if ((!v && pass) || v->call_id > pass->call_id)
				v = pass;

		}
	}
	void SyncState::max(const SyncState& state)
	{

		for (auto type : magic_enum::enum_values<CommandListType>())
		{
			auto& v = values[type];
			auto& pass = state.values[type];
			if ((!v && pass) || v->call_id < pass->call_id)
				v = pass;

		}
	}
	void SyncState::reset()
	{
		for (auto type : magic_enum::enum_values<CommandListType>())
		{
			auto& v = values[type];
			v = nullptr;
		}
	}
}