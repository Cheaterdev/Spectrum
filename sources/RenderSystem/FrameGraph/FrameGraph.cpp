module FrameGraph;
import HAL;
import Core;
import <HAL.h>;

using namespace HAL;



namespace FrameGraph
{
	void ResourceAllocInfo::add_pass(Pass* pass, ResourceFlags flags)
	{
		PROFILE(L"add_pass");

		assert(pass);
		bool is_writer = check(flags & WRITEABLE_FLAGS);

		bool was_swap = false;

		if (is_writer || states.empty())
		{

			ResourceRWState state;
			state.write = is_writer;
			states.emplace_back(std::move(state));
			was_swap = true;
			last_writer = static_cast<int>(states.size() - 1);
		}
		else
		{
			if (states.back().write)
			{
				ResourceRWState state;
				state.write = false;
				states.emplace_back(std::move(state));
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
		states.reserve(16);
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



			for (auto p : state.passes)

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
	TaskBuilder::TaskBuilder() : frames(Device::get()), allocator(HAL::Device::get().get_heap_factory(), false), global_frame(Device::get())
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

	void TaskBuilder::pass_texture(std::string name, HAL::Texture::ptr tex, HAL::FenceWaiter fence, ResourceFlags flags)
	{

		tex->resource->disable_state_tracking();

		auto tex_desc = tex->get_desc().as_texture();
		if (tex_desc.is2D())
		{
			Handlers::Texture h(name);
			create(h, { ivec3(0,0,0), HAL::Format::UNKNOWN, 0 }, flags);
			auto& info = *h.info;
			info.passed = true;

			info.resource = tex->resource;
			info.fence = fence;
			info.d3ddesc = tex->get_desc();
			passed_resources.insert(&info);


			h.desc.array_count = tex->get_desc().as_texture().ArraySize;
			h.desc.format = tex->get_desc().as_texture().Format;
			h.desc.mip_count = tex->get_desc().as_texture().MipLevels;
			h.desc.size = tex->get_desc().as_texture().Dimensions;

			info.name = name;
			tex->resource->set_name(name);

			h.init_view(info, *current_frame);


			info.creation_state = info.last_state = info.resource->get_state_manager().copy_gpu();
		}
		else if (tex_desc.is3D())
		{
			Handlers::Texture3D h(name);
			create(h, { ivec3(0,0,0), HAL::Format::UNKNOWN, }, flags);
			auto& info = *h.info;

			info.name = name;
			tex->resource->set_name(name);


			info.passed = true;

			info.resource = tex->resource;
			info.d3ddesc = tex->get_desc();
			info.fence = fence;
			passed_resources.insert(&info);

			h.desc.format = tex->get_desc().as_texture().Format;
			h.desc.mip_count = tex->get_desc().as_texture().MipLevels;
			h.desc.size = tex->get_desc().as_texture().Dimensions;

			h.init_view(info, *current_frame);
			info.creation_state = info.last_state = info.resource->get_state_manager().copy_gpu();
		}
		else
			assert(false);
	}

	void TaskBuilder::reset()
	{
		current_frame = nullptr;
		current_pass = nullptr;
		//	allocator.reset();
	/*	for (auto& pair : alloc_resources)
		{
			pair.second.view = nullptr;
		}			 */
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


		}
		return list;
	}

	void FrameContext::begin(Graph* graph, Pass* pass, HAL::FrameResources::ptr& frame) {

		this->pass = pass;
		this->frame = frame;
		 this->graph = graph;

		bool need_list = !pass->used.resource_deletions_before.empty() || !pass->used.resource_creations.empty();

		if (need_list)
		{

			auto& _list = get_list();

			for (auto info : pass->used.resource_deletions_before)
			{
				if (!info->alloc_ptr.handle) continue;

				if (!info->enabled)
					continue;

				list->alias_end(info->resource.get());
			}

			for (auto info : pass->used.resource_creations)
			{
				if (!info->alloc_ptr.handle) continue;

				if (!info->enabled)
					continue;

				//			list->discard(info->resource.get());
				list->alias_begin(info->resource.get());
			}


		}
	}

	void FrameContext::end()
	{
		if (list)
		{

			for(auto [info, flags]:pass->used.resource_flags)
			{
				if(!check(flags&WRITEABLE_FLAGS)) continue;
				 info->process_debug_resource(pass, this);
			}
			for (auto info : pass->used.resource_deletions_after)
			{
				if (!info->alloc_ptr.handle) continue;

				if (!info->enabled)
					continue;

				list->alias_end(info->resource.get());
			}

			list->end();

		}
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

			PROFILE(L"one_pass_setup");

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

			PROFILE(L"optimizing");

			{
				PROFILE(L"sorting");


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

			/*	builder.enabled_passes.sort(
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
						 */

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


			//builder.enabled_passes = new_enabled_passes;
		}

		int i = 1;



		for (auto pass : builder.enabled_passes)
		{
			pass->call_id = i++;
			pass->wait_pass = nullptr;
			pass->prev_pass = nullptr;
		}

		Pass* prev_compute = nullptr;
		Pass* prev_graphics = nullptr;

		for (auto pass : builder.enabled_passes)
		{
			if (check(pass->flags & PassFlags::Compute))
			{
				if (prev_compute)
				{
					pass->sync_state.max(prev_compute);
					pass->sync_state.max(prev_compute->sync_state);
				}
				prev_compute = pass;
			}
			else

			{
				if (prev_graphics)
				{
					pass->sync_state.max(prev_graphics);
					pass->sync_state.max(prev_graphics->sync_state);
				}
				prev_graphics = pass;


			}

			pass->sync_state_with_self = pass->sync_state;
			pass->sync_state_with_self.max(pass);
		}


		for (auto& pair : builder.alloc_resources)
		{
			auto& info = pair.second;
			if (!info.enabled) continue;

			info.remove_inactive();

			info.used_begin.reset();
			info.used_end.reset();



			for (auto& state : info.states)
			{
				for (auto pass : state.passes)
				{
					info.used_begin.min(pass);
					info.used_end.max(pass);
				}
			}

			ResourceRWState* prev_state = nullptr;
			for (auto& state : info.states)
			{

				if (prev_state)
					for (auto pass : state.passes)
					{
						pass->sync_state.max(prev_state->to);

						pass->sync_state_with_self.max(prev_state->to);
					}

				prev_state = &state;
			}
		}
	}

	void Graph::compile(int frame)
	{
		PROFILE(L"Graphcompile");


		builder.create_resources();

		for (auto& f : pre_run)
			f(*this);

		//for (auto& pass : builder.enabled_passes)
		//	pass->compile(builder); 
	}

	void Graph::render()
	{
		PROFILE(L"Graphrender");


		{
			PROFILE(L"passes");

			for (auto& pass : builder.enabled_passes)
				pass->render(this, builder.current_frame);

		}

		{
			PROFILE(L"wait");

			for (auto& pass : builder.enabled_passes)
			{
				pass->wait();

				for (auto fence : pass->used.fences)
				{
					fence.wait();
				}
			}


		}

		builder.process_transitions();
		builder.process_fences();
		builder.compile_lists();
	}


	HAL::FenceWaiter Graph::commit_command_lists()
	{

		HAL::FenceWaiter result;


		

			
				PROFILE(L"submitting lists");

				std::map<CommandListType, std::list<CommandList::ptr>> queued_lists;


				for (auto& pass : builder.enabled_passes)
				{
					HAL::CommandListType list_type = pass->get_type();

					auto commandList = pass->context.list;

					if (commandList)
					{


						for (auto sync_pass : pass->sync_state.values)
						{
							if (!sync_pass) continue;

							HAL::Device::get().get_queue(list_type)->execute(queued_lists[list_type]);
							queued_lists[list_type].clear();

							HAL::Device::get().get_queue(list_type)->gpu_wait(sync_pass->fence_end);
						}



						queued_lists[list_type].emplace_back(commandList);

						//	if(pass->put_fence)		//////////////////////// ARGH!!!!
						{
							pass->fence_end = HAL::Device::get().get_queue(list_type)->execute(queued_lists[list_type]);

							queued_lists[list_type].clear();

							result = pass->fence_end;
						}
						//	pass->fence_end = commandList->execute();

						//	
					}


				}


				for (auto& [type, lists] : queued_lists)
				{
					if (lists.empty()) continue;
					result = HAL::Device::get().get_queue(type)->execute(lists);
				}
			

		 PROFILE(L"free resources");



		for (auto& pair : builder.alloc_resources)
		{
			auto info = &pair.second;
			if (!check(info->flags & ResourceFlags::Static))
			{
				info->resource = nullptr;
				//		info->view = nullptr;
			}

			if (info->heap_type != HAL::HeapType::DEFAULT)
			{
				info->alloc_ptr.Free();
			}

		}
		return result;
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
		//flags |=ResourceFlags::Static;
		info.reset();
		info.flags = flags;
		info.name = name;
		info.frame_id = current_frame->get_frame();
		info.is_new = false;
		//info.valid_from = info.valid_to = info.valid_to_start = nullptr;

		if (current_pass) {
			current_pass->used.resources.insert(&info);
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

		if (info.fence)
		{
			current_pass->used.fences.emplace_back(info.fence);
		}
	}



	void TaskBuilder::process_fences()
	{
		PROFILE(L"processing fences");
		enum_array<CommandListType, SyncState> queued_state;


		for (auto& pass : enabled_passes)
		{
			HAL::CommandListType list_type = pass->get_type();
			auto commandList = pass->context.list;

			if (commandList)
			{
				for (auto& sync_pass : pass->sync_state.values)
				{
					if (!sync_pass) continue;

					HAL::CommandListType other_type = sync_pass->get_type();

					if (!queued_state[list_type].is_in_sync(sync_pass, true))
					{
						assert(other_type != list_type);
						const_cast<Pass*>(sync_pass)->put_fence = true;
						queued_state[list_type].max(sync_pass);
					}
					else
					{
						sync_pass = nullptr;
					}
				}
			}
			else
			{
				//	auto prev_pass = queued_state[list_type].values[list_type];
				//	if (prev_pass)
				//		pass->fence_end = prev_pass->fence_end;
			}
			queued_state[list_type].max(pass);
		}
	}

	void TaskBuilder::compile_lists()
	{
		PROFILE(L"compile");



		//	std::list<std::future<void>> tasks;
		for (auto& pass : enabled_passes)
		{
			auto commandList = pass->context.list;
			if (!commandList) continue;

			commandList->compile_transitions();
		}



		//	std::list<std::future<void>> tasks;
		for (auto& pass : enabled_passes)
		{
			auto commandList = pass->context.list;
			if (!commandList) continue;

			pass->compile_task = thread_pool::get().enqueue([commandList, pass]() {
				PROFILE(pass->name);

				commandList->compile();
				});

		}

		for (auto& pass : enabled_passes)
		{
			auto commandList = pass->context.list;
			if (!commandList) continue;


			{
				PROFILE(L"pass_wait");
				pass->compile_task.wait();
			}
		}
	}



	void TaskBuilder::process_transitions()
	{
		PROFILE(L"optimizing transitions");
		for (auto& pair : alloc_resources)
		{
			auto& info = pair.second;
			if (!info.enabled) continue;

			///	if (info.passed) continue;///wtf

			auto& resource = info.resource;

			if (!resource) continue;
			if (resource->get_desc().is_buffer()) continue;

			if (info.heap_type != HAL::HeapType::DEFAULT) continue;

			   bool nb = info.name == "PSSM_Depths";
			auto pass_checker = [&](Pass* pass) {
				auto commandList = pass->context.list;
				if (!commandList)
					return true;

				auto& cpu_state = info.resource->get_state_manager().get_cpu_state(commandList.get());
				if (!cpu_state.used)
					return true;


				return false;
				};


			auto state_checker = [](const ResourceRWState& state) {
				return state.passes.empty();
				};
			// remove unused passes
			for (auto& state : info.states)
			{
				state.passes.erase(std::remove_if(state.passes.begin(), state.passes.end(), pass_checker), state.passes.end());
			}
			info.states.erase(std::remove_if(info.states.begin(), info.states.end(), state_checker), info.states.end());

			// merge resourcestate access in a same read or write state
			for (auto& state : info.states)
			{
				if (state.write) continue;
				state.merged_read_state.subres.resize(resource->get_state_manager().get_subres_count());

				// calculate merged state
				for (auto& pass : state.passes)
				{
					auto commandList = pass->context.list;
					auto& cpu_state = resource->get_state_manager().get_cpu_state(commandList.get());
					state.merged_read_state.merge(cpu_state);
				}

				// propagate merged state through passes
				for (auto& pass : state.passes)
				{
					auto commandList = pass->context.list;
					auto& cpu_state = resource->get_state_manager().get_cpu_state(commandList.get());
					cpu_state.merge_read_state(commandList->get_type(), state.merged_read_state);
				}
			}

			bool need_first_transition = true;
			// link statee between passes
			for (uint i = 0; i < info.states.size(); i++)
			{
				auto& state = info.states[i];
				if (!state.write)
				{

					for (auto& pass : state.passes)
					{
						auto commandList = pass->context.list;
						if (!commandList) continue;
						auto& cpu_state = resource->get_state_manager().get_cpu_state(commandList.get());
						if (!cpu_state.used) continue;

						need_first_transition = false;
						break;
					}

					continue;
				}
				assert(state.passes.size() == 1);
				auto pass = state.passes.front();
				auto commandList = pass->context.list;
				if (!commandList) continue;

				HAL::CommandListType list_type = pass->get_type();

				// first write synchronize with start=end
				if (i == 0 && (info.is_static() || info.passed))
				{
					auto target = ResourceStates::NO_ACCESS;
					auto layout = info.last_state.get_subres_state(0).layout;
					target.layout = layout;
					info.resource->get_state_manager().prepare_state(commandList.get(), target);
				}


				// check previous pass is read
				if (i > 0 && !info.states[i - 1].write)
				{
					auto prev_state = info.states[i - 1];
					auto best_type = prev_state.merged_read_state.get_best_list_type();
					//		its in 99% read to write compatible on all queues
					assert(IsCompatible(list_type, best_type));
					info.resource->get_state_manager().prepare_state(commandList.get(), prev_state.merged_read_state);
				}


				// check next pass is read
				if ((i < info.states.size() - 1) && !info.states[i + 1].write)
				{
					auto next_state = info.states[i + 1];
					auto best_type = next_state.merged_read_state.get_best_list_type();
					//		its in 99% write to read compatible on all queues
					assert(IsCompatible(list_type, best_type));
					info.resource->get_state_manager().prepare_after_state(commandList.get(), next_state.merged_read_state);
				}

				// cur pass is write, next pass is write ,what can be wrong?

				if ((i < info.states.size() - 1) && info.states[i + 1].write)
				{
					auto prev_writer = info.states[i];
					auto next_writer = info.states[i + 1];

					auto prev_pass = prev_writer.passes.front();
					auto next_pass = next_writer.passes.front();

					auto prev_cmd = prev_pass->context.list;
					auto next_cmd = next_pass->context.list;
					if (!prev_cmd || !next_cmd)
					{
						assert(false); // no write requested with no commandlist
						continue;
					}

					auto& prev_cpu_state = resource->get_state_manager().get_cpu_state(prev_cmd.get());
					auto& next_cpu_state = resource->get_state_manager().get_cpu_state(next_cmd.get());


					// TODO: try removing this write state if not used
					assert(prev_cpu_state.used);
					assert(next_cpu_state.used);

					HAL::CommandListType next_list_type = next_pass->get_type();
					HAL::CommandListType prev_list_type = prev_pass->get_type();

					HAL::SubResourcesGPU prev_gpu_state;
					prev_gpu_state.subres.resize(resource->get_state_manager().get_subres_count());
					prev_gpu_state.set_cpu_state(prev_cpu_state);


					HAL::SubResourcesGPU next_gpu_state;
					next_gpu_state.subres.resize(resource->get_state_manager().get_subres_count());
					next_gpu_state.set_cpu_state_first(next_cpu_state);


					auto transition_best_type_layout = Merge(prev_gpu_state.get_best_list_type(), next_gpu_state.get_best_list_type());


					bool compatible_next_layout = IsCompatible(next_list_type, transition_best_type_layout);
					bool compatible_prev_layout = IsCompatible(prev_list_type, transition_best_type_layout);
  					
	   			   auto transition_best_type = Merge(prev_cpu_state.get_best_list_type_last(), next_cpu_state.get_best_list_type_first());

					bool compatible_next = IsCompatible(next_list_type, transition_best_type);
					bool compatible_prev = IsCompatible(prev_list_type, transition_best_type);

					  // 	
					//info.resource->get_state_manager().connect(prev_cmd.get(), next_cmd.get());
					if (compatible_next && compatible_prev)	
					{
						// do a split transition
						info.resource->get_state_manager().connect(prev_cmd.get(), next_cmd.get());
					}
					else if (compatible_next_layout)
					{
						// next pass should rewrite its first state
						info.resource->get_state_manager().prepare_state(next_cmd.get(), prev_gpu_state);			  // add sync&access info
					}
					else if (compatible_prev_layout)
					{

						info.resource->get_state_manager().prepare_after_state(prev_cmd.get(), next_gpu_state);		 // add sync&access infoZ
					}else
						assert(false);



				}


				//last state is a write state
				if ((i == info.states.size() - 1) && info.states[i].write && (info.is_static() || info.passed))
				{
					auto layout = info.creation_state.get_subres_state(0).layout;
					auto target = ResourceStates::NO_ACCESS;

					target.layout = layout;
					info.resource->get_state_manager().transition(commandList.get(), target, ALL_SUBRESOURCES);
				}


			}






			// link end to start transition
			if (!info.states.empty() && (info.is_static() || info.passed))
			{
				Pass* pass = info.states.back().passes.back();
				auto commandList = pass->context.list;
				info.last_state.set_cpu_state(info.resource->get_state_manager().get_cpu_state(commandList.get()));

			}


		}
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
			ResourceAllocInfo* info = &pair.second;
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

			if (info->heap_type != HAL::HeapType::DEFAULT) continue;



			// if the resource is temp, first pass should create it -> write
			assert(info->states[0].passes.size() == 1);
			assert(info->states[0].write);


			Pass* best_creation_pass = info->states.front().passes.front();
			Pass* best_deletion_pass = nullptr;


			// create - easy
			events[best_creation_pass->call_id].create.insert(info);
			best_creation_pass->used.resource_creations.insert(info);

			// find last pass that uses the resource 
			// TODO: pass end is not in sync with events
			// cant delete just in place - need to delete at first pass that is in sync with this one
			// looks like this is improssble to wind such pass
		/*	for (auto pass : info->states.back().passes)
			{
				if (info->used_end.is_in_sync(pass->sync_state_with_self))
				{
					best_deletion_pass = pass;
					events[best_deletion_pass->call_id].free_after.insert(info);
					best_deletion_pass->used.resource_deletions_after.insert(info);
					break;
				}
			}
					  */
					  // if no pass found - find any pass that is synced to the usage
			if (!best_deletion_pass)
			{

				for (auto pass : enabled_passes)
				{
					if (info->used_end.is_in_sync(pass->sync_state, false))
					{
						best_deletion_pass = pass;
						events[best_deletion_pass->call_id].free_before.insert(info);
						best_deletion_pass->used.resource_deletions_before.insert(info);

						//	events[best_deletion_pass->call_id].free_after.insert(info);
						//best_deletion_pass->used.resource_deletions_after.insert(info);
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

				PROFILE(L"resource");



				if (info->alloc_ptr.handle)
				{
					auto& res = info->resource_places[info->alloc_ptr];

					if (!res || res.resource->get_desc() != info->d3ddesc)
					{
						PROFILE(L"placed");
						res.resource = HAL::create_resource(info->d3ddesc, info->alloc_ptr);
						//						 	res.resource->debug = info->name=="ResultTexture"; // TODO: move everywhere		
//res.resource->debug_transitions = info->name=="GBuffer_Normals"; // TODO: move everywhere		


						res.resource->set_name(info->name);
						res.view = nullptr;
					}

					if (info->resource != res.resource)
					{
						info->is_new = true;
					}

					info->resource = res.resource;
					info->view = res.view;

					if (!info->view)
					{
						info->handler->init_view(*info, global_frame);
						res.view = info->view;
					}
				}
				else
				{

					if (info->heap_type == HAL::HeapType::UPLOAD)
					{
						PROFILE(L"UPLOAD");
						info->resource = HAL::create_resource(info->d3ddesc, info->heap_type);
						info->is_new = true;
					}
					else if (info->heap_type == HAL::HeapType::READBACK)
					{
						PROFILE(L"READBACK");
						info->resource = HAL::create_resource(info->d3ddesc, info->heap_type);
						info->is_new = true;
					}
					else if (!info->resource || info->resource->get_desc() != info->d3ddesc)
					{
						PROFILE(L"DEFAULT");
						info->resource = HAL::create_resource(info->d3ddesc, info->heap_type);
						info->is_new = true;

					}





					//	info->resource = info->resource;
						//

					if (info->is_new)
					{
						info->creation_state =info->last_state = info->resource->get_state_manager().copy_gpu();
						info->last_state = TextureLayout::UNDEFINED;
						info->view = nullptr;
						info->resource->set_name(info->name);

						if (info->heap_type != HAL::HeapType::DEFAULT)
							info->handler->init_view(*info, *current_frame);
						else

							info->handler->init_view(*info, global_frame);
						//	res.view = info->view;
					}
				}

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


	bool SyncState::is_in_sync(const SyncState& state, bool equal)
	{

		for (auto type : magic_enum::enum_values<CommandListType>())
		{

			auto& v = values[type];
			auto& pass = state.values[type];


			auto a = v ? v->call_id : 0;
			auto b = (pass ? pass->call_id : 0);

			if (equal)
			{
				if ((a > 0 || b > 0) && a >= b) return false;


			}
			else
			{
				if ((a > 0 || b > 0) && a > b) return false;
			}
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

			if (!v)v = pass;
			else

				if (pass && v->call_id < pass->call_id)
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