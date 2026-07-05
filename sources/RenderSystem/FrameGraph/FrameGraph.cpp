module;
#include "autogen/resource_ids.h"
module FrameGraph;
import RenderSystem;
import HAL;
import Core;


using namespace HAL;



namespace FrameGraph
{
	const char* ResourceAllocInfo::name() const { return resource_id_name(id); }

	// Stage 1: record this pass's touch against the resource's RW-state timeline.
	void ResourceAllocInfo::add_pass(Pass* pass, ResourceFlags flags)
	{
		PROFILE(L"add_pass");

		ASSERT(pass);
		bool is_writer = check(flags & WRITEABLE_FLAGS);
		bool needs_new_state = is_writer || states.empty() || states.back().write;

		if (needs_new_state)
		{
			states.push_back({ .write = is_writer });
			if (is_writer)
				last_writer = static_cast<int>(states.size() - 1);

			ResourceRWState& new_state = states.back();
			new_state.passes.reserve(max_passes);
			new_state.compute.reserve(max_passes);
			new_state.graphics.reserve(max_passes);
		}

		ResourceRWState& state = states.back();
		state.passes.emplace_back(pass);
		(check(pass->flags & PassFlags::Compute) ? state.compute : state.graphics).emplace_back(pass);
	}

	// Stage 2: once every pass has run add_pass for this resource, derive prev/next
	// pass edges from the completed states timeline. Gathers edges into a caller-owned
	// buffer instead of writing into Pass directly, so this can run concurrently across
	// resources (each resource only ever touches its own buffer slot).
	void ResourceAllocInfo::resolve_dependencies(std::vector<std::pair<Pass*, Pass*>>& edges)
	{
		PROFILE(L"resolve");

		std::unordered_set<Pass*> related;

		for (auto& state : states)
		{
			for (auto pass : state.passes)
			{
				for (auto p : related)
				{
					ASSERT(p != pass);
					edges.emplace_back(p, pass);
				}
			}
			related.insert(state.passes.begin(), state.passes.end());
		}
	}


	void ResourceAllocInfo::reset(size_t new_max_passes)
	{
		last_writer = 0;
 		max_passes = new_max_passes;
		states.clear();
		states.reserve(16);
	}

	void ResourceAllocInfo::remove_inactive()
	{

		// Filter by enabled (reachable), not active() (enabled && renderable).
		// A pass can be enabled but choose renderable=false (nothing to render
		// this frame) while still needing its create()/need() touches to count —
		// otherwise it gets stripped from its own creation state here, which can
		// prune the whole state and leave the resource never created downstream.
		// process_transitions() independently filters by context.list nullness,
		// so passes that never actually got a command list still don't
		// participate in real GPU barrier scheduling.
		auto fn = [](Pass* pass) {return !pass->enabled; };

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
				// state.passes now keeps enabled-but-not-renderable passes (needed
				// for resource creation, see fn above) — but sync points must only
				// ever reference passes that actually execute and signal a fence.
				// Waiting on one that never renders would hang forever.
				if (!p->active()) continue;

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
	TaskBuilder::TaskBuilder() : frames(RenderSystem::get().device()), allocator(RenderSystem::get().device().get_heap_factory(), false), global_frame(RenderSystem::get().device())
	{

	}

	Pass* TaskBuilder::get_pass(LiteralWStr name)		 const
	{
		for (auto& p : passes)
			if (p->name.ptr == name.ptr)
				return p.get();
		return nullptr;
	}

	void TaskBuilder::begin(Pass* pass)
	{
		current_pass = pass;
	}

	void TaskBuilder::end(Pass* pass)
	{
		current_pass = nullptr;
	}

	void TaskBuilder::pass_texture(ResourceID id, HAL::TextureResource::ptr tex, HAL::FenceWaiter fence, ResourceFlags flags)
	{

		tex->disable_state_tracking();

		auto tex_desc = tex->get_desc().as_texture();
		if (tex_desc.is2D())
		{
			Handlers::Texture h(id);
			create(h, { ivec3(0,0,0), HAL::Format::UNKNOWN, 0 }, flags);
			auto& info = *h.info;
			info.passed = true;

			info.resource = tex;
			info.fence = fence;
			info.d3ddesc = tex->get_desc();
			passed_resources.insert(&info);


			h.desc.array_count = tex->get_desc().as_texture().ArraySize;
			h.desc.format = tex->get_desc().as_texture().Format;
			h.desc.mip_count = tex->get_desc().as_texture().MipLevels;
			h.desc.size = tex->get_desc().as_texture().Dimensions;

			tex->set_name(FrameGraph::resource_id_name(id));

			h.init_view(info, *current_frame);


			info.creation_state = info.last_state = info.resource->get_state_manager().copy_gpu();
		}
		else if (tex_desc.is3D())
		{
			Handlers::Texture3D h(id);
			create(h, { ivec3(0,0,0), HAL::Format::UNKNOWN, }, flags);
			auto& info = *h.info;

			tex->set_name(FrameGraph::resource_id_name(id));


			info.passed = true;

			info.resource = tex;
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
			ASSERT(false);
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
		external_pass.reset();
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
			for (auto& chain : builder.alloc_resources) chain.reset_frame();

		}
	}

	void Graph::setup()
	{

		PROFILE(L"Graphsetup");

		for (auto& pass : builder.passes)
		{
			PROFILE(L"one_pass_setup");
			pass->enabled = false;
			pass->renderable = pass->setup(builder);
		}

		{
			PROFILE(L"resolve_dependencies");

			// Flip to benchmark: work per resource is tiny, so thread-pool dispatch
			// overhead may or may not be worth paying depending on pass/resource count.
			constexpr bool resolve_dependencies_mt = false;

			if constexpr (resolve_dependencies_mt)
			{
				// Work per resource is tiny, so per-submit overhead dominates well before
				// hardware_concurrency() chunks are reached — cap chunk count hard instead
				// of scaling with core count.
				constexpr size_t max_chunks = 4;
				size_t n = builder.alloc_resources.size();
				size_t worker_count = std::min<size_t>(n, max_chunks);
				size_t chunk_size = (n + worker_count - 1) / worker_count;

				std::vector<std::vector<std::pair<Pass*, Pass*>>> gathered_edges(worker_count);
				std::atomic<size_t> pending{ 0 };

				// Chunk 0 runs inline on the calling thread — one fewer dispatch round trip,
				// and the caller does useful work instead of idling in the wait loop below.
				for (size_t w = 1; w < worker_count; ++w)
				{
					size_t begin = w * chunk_size;
					size_t end = std::min(begin + chunk_size, n);
					if (begin >= end) break;
					PROFILE(L"enqueue_detached");

					pending.fetch_add(1, std::memory_order_relaxed);
					thread_pool::get().enqueue_detached([this, &gathered_edges, &pending, w, begin, end]()
					{
						auto& edges = gathered_edges[w];
						for (size_t i = begin; i < end; ++i)
						{
							auto& chain = builder.alloc_resources[i];
							for (auto& info : chain.active_span())
								info.resolve_dependencies(edges);
						}
						pending.fetch_sub(1, std::memory_order_acq_rel);
					});
				}

				{
					PROFILE(L"inline_chunk");
					size_t end0 = std::min(chunk_size, n);
					auto& edges0 = gathered_edges[0];
					for (size_t i = 0; i < end0; ++i)
					{
						auto& chain = builder.alloc_resources[i];
						for (auto& info : chain.active_span())
							info.resolve_dependencies(edges0);
					}
				}

				{
					PROFILE(L"wait");
					while (pending.load(std::memory_order_acquire) != 0)
						std::this_thread::yield();
				}

				PROFILE(L"complete");
				for (auto& edges : gathered_edges)
					for (auto [prev, next] : edges)
					{
						prev->next_passes.insert(next);
						next->prev_passes.insert(prev);
					}
			}
			else
			{
				std::vector<std::pair<Pass*, Pass*>> edges;
				for (auto& chain : builder.alloc_resources)
					for (auto& info : chain.active_span())
						info.resolve_dependencies(edges);

				PROFILE(L"complete");
				for (auto [prev, next] : edges)
				{
					prev->next_passes.insert(next);
					next->prev_passes.insert(prev);
				}
			}
		}

		for (auto& chain : builder.alloc_resources)
			for (auto& info : chain.active_span())
			{

				if (info.passed) continue;
				info.enabled = false;
			}

		builder.enabled_resources.clear();

		auto process_resource = [&](this auto&& self, ResourceAllocInfo& info, UINT pass_id) -> void {

		//	if (info.enabled) return;
			if (!info.enabled) builder.enabled_resources.push_back(&info);
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
						self(*info, pass->id);
					}
				}
			}

			};

		for (auto& chain : builder.alloc_resources)
			for (auto& info : chain.active_span())
			{

				if (check(info.flags & ResourceFlags::Required))
					process_resource(info, (int)builder.passes.size());
			}


		for (auto& pass : builder.required_passes)
		{
			pass->enabled = true;
			for (auto& info : pass->used.resources)
			{
				process_resource(*info, pass->id);
			}
		}

		for (auto pass : builder.passes)
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

			auto insert_pass = [&](this auto&& self, Pass* pass) -> void {
				for (auto prev : pass->prev_passes)
				{
					if (!prev->inserted) self(prev);
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

		// Inject a fake pass that owns passed and static resources as their creator.
		// Added after call_id/sync-state assignment so it doesn't disturb scheduling;
		// call_id=0 places it before all real passes in the timeline.
		{
			auto ext = std::make_shared<ExternalPass>();
			ext->call_id = 0;

			for (auto* alloc : builder.passed_resources)
			{
				ext->used.resources.insert(alloc);
				ext->used.resource_flags[alloc] = ResourceFlags::RenderTarget;
				ext->used.resource_creations.insert(alloc);
			}

			for (auto& chain : builder.alloc_resources)
			{
				if (chain.empty()) continue;
				auto& alloc = chain.active();
				if (!alloc.is_static()) continue;
				ext->used.resources.insert(&alloc);
				ext->used.resource_flags[&alloc] = ResourceFlags::RenderTarget;
				ext->used.resource_creations.insert(&alloc);
			}

			if (!ext->used.resources.empty())
			{
				builder.external_pass = ext;
				builder.enabled_passes.push_front(ext.get());
			}
		}

		for (auto& chain : builder.alloc_resources)
			for (auto& info : chain.active_span())
			{

			if (!info.enabled) continue;

			info.remove_inactive();

			info.used_begin.reset();
			info.used_end.reset();



			for (auto& state : info.states)
			{
				for (auto pass : state.passes)
				{
					// Same rule as remove_inactive(): state.passes now keeps
					// enabled-but-not-renderable passes for resource-creation
					// purposes, but sync/fence state must only ever reference
					// passes that actually execute and signal something.
					if (!pass->active()) continue;

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
						if (!pass->active()) continue;

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



		on_compile(*this);
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

							RenderSystem::get().device().get_queue(list_type)->execute(queued_lists[list_type]);
							queued_lists[list_type].clear();

							RenderSystem::get().device().get_queue(list_type)->gpu_wait(sync_pass->fence_end);
						}



						queued_lists[list_type].emplace_back(commandList);

							if(pass->put_fence)		//////////////////////// ARGH!!!!
						{
							pass->fence_end = RenderSystem::get().device().get_queue(list_type)->execute(queued_lists[list_type]);

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
					result = RenderSystem::get().device().get_queue(type)->execute(lists);
				}
			

		 PROFILE(L"free resources");



		for (auto& chain : builder.alloc_resources)
			for (auto& info : chain.active_span())
			{

				if (!check(info.flags & ResourceFlags::Static) && !info.passed)
					info.resource = nullptr;
				if (info.heap_type != HAL::HeapType::DEFAULT)
					info.alloc_ptr.Free();
			}
		return result;
	}

	void Graph::reset()
	{

		for (auto& pass : builder.enabled_passes)
		{
			pass->wait();
		}

		builder.passes.clear();
		builder.required_passes.clear();
		builder.enabled_passes.clear();
		builder.reset();

		// Release cached passes' per-frame state (context.frame/list in particular
		// hold shared_ptrs into this frame's HAL::FrameResources/command list, which
		// wrap the swapchain back buffer) right away instead of deferring it to next
		// frame's add_passes() — otherwise those references outlive the frame across
		// the gap until reuse, which blocks swapchain resize.
		for (auto& slots : builder.pass_cache)
			for (auto& slot : slots)
				if (slot) slot->reset_frame();

		pre_run.clear();
		slot_setters.clear();

	}



	void TaskBuilder::init(ResourceAllocInfo& info, ResourceFlags flags)
	{
		// All passes must be registered (add_passes) before any resource is
		// created: max_passes is captured here from passes.size() and used to
		// pre-size the per-state concurrent_vector append storage. Creating a
		// resource while passes is still empty (e.g. passing the swapchain in
		// before add_passes) under-sizes that storage and overflows on later
		// need()s from real passes.
		ASSERT(!passes.empty());

		//flags |=ResourceFlags::Static;
		info.reset(passes.size());
		info.flags = flags;
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
						ASSERT(other_type != list_type);
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


	  {

			PROFILE(L"compile_transitions");
		for (auto& pass : enabled_passes)
		{
			auto commandList = pass->context.list;
			if (!commandList) continue;

			
		}


		}

				descriptor_commit_task = thread_pool::get().enqueue([this]()
			{
				current_frame->commit_descriptors_to_gpu();
			});


	    {

			PROFILE(L"compile_passes");
		for (auto& pass : enabled_passes)
		{
			auto commandList = pass->context.list;
			if (!commandList) continue;

			pass->compile_task = thread_pool::get().enqueue([commandList, pass]() {
				PROFILE(pass->name);
			   commandList->compile_transitions();
				commandList->compile();
				});

		}
	  }

		for (auto& pass : enabled_passes)
		{
			auto commandList = pass->context.list;
			if (!commandList) continue;

			{
				PROFILE(L"pass_wait");
				pass->compile_task.wait();
			}

			// Snapshot debug records now: compile_transitions has filled barrier data,
			// compile() has run. Resolve Transition descriptions so no raw pointers survive.
			if constexpr (BuildOptions::Dev)
			{
				auto records = commandList->get_debug_records();
				for (auto& rec : records)
				{
					if (rec.type == HAL::CommandType::Transition && rec.barrier_point)
					{
						const auto& barriers = rec.barrier_point->transitions.get_barriers();
						rec.description = "Barriers: " + std::to_string(barriers.size());
						rec.barrier_details.reserve(barriers.size());
						for (const auto& b : barriers)
						{
							HAL::CommandRecord::BarrierDetail detail;
							detail.resource_name = b.resource ? b.resource->name : "?";
							detail.before        = b.before;
							detail.after         = b.after;
							detail.subres        = b.subres;
							detail.flags         = b.flags;
							rec.barrier_details.push_back(std::move(detail));
						}
						rec.barrier_point = nullptr;
					}
				}
				pass->debug_commands = std::move(records);
			}
		}

				// Descriptors must be visible to the GPU before any list below actually executes.
		// This is the last possible point to wait - everything since render() kicked the
		// commit off (queued_lists bookkeeping, this call's own dispatch) ran concurrently
		// with it for free.
		if (descriptor_commit_task.valid())
		{
			PROFILE(L"wait descriptor_commit");
			descriptor_commit_task.wait();
		}


	}



	void TaskBuilder::process_transitions()
	{
		PROFILE(L"optimizing transitions");
		for (auto& chain : alloc_resources)
			for (auto& info : chain.active_span())
			{

			if (!info.enabled) continue;

			///	if (info.passed) continue;///wtf

			auto& resource = info.resource;

			if (!resource) continue;
			if (resource->get_desc().is_buffer()) continue;

			if (info.heap_type != HAL::HeapType::DEFAULT) continue;

			   bool nb = info.id == ResourceID::PSSM_Depths;
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
				ASSERT(state.passes.size() == 1);
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
					ASSERT(IsCompatible(list_type, best_type));
					info.resource->get_state_manager().prepare_state(commandList.get(), prev_state.merged_read_state);
				}


				// check next pass is read
				if ((i < info.states.size() - 1) && !info.states[i + 1].write)
				{
					auto next_state = info.states[i + 1];
					auto best_type = next_state.merged_read_state.get_best_list_type();
					//		its in 99% write to read compatible on all queues
					ASSERT(IsCompatible(list_type, best_type));
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
						ASSERT(false); // no write requested with no commandlist
						continue;
					}

					auto& prev_cpu_state = resource->get_state_manager().get_cpu_state(prev_cmd.get());
					auto& next_cpu_state = resource->get_state_manager().get_cpu_state(next_cmd.get());


					// TODO: try removing this write state if not used
					ASSERT(prev_cpu_state.used);
					ASSERT(next_cpu_state.used);

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
				/*	if (compatible_next && compatible_prev)	
					{
						// do a split transition
						info.resource->get_state_manager().connect(prev_cmd.get(), next_cmd.get());
					}
					else*/ if (compatible_next_layout)
					{
						// next pass should rewrite its first state
						info.resource->get_state_manager().prepare_state(next_cmd.get(), prev_gpu_state);			  // add sync&access info
					}
					else if (compatible_prev_layout)
					{

						info.resource->get_state_manager().prepare_after_state(prev_cmd.get(), next_gpu_state);		 // add sync&access infoZ
					}else
						ASSERT(false);



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


		for (auto& pass : enabled_passes)
		{
			auto commandList = pass->context.list;
			if (!commandList) continue;
			
			for(auto info:pass->used.resource_deletions_before)
			{

			if(info->states.empty())continue;
				auto& last_state = info->states.back();
				if(last_state.write)
				{
						auto prev_pass = last_state.passes.front();
						 	auto prev_cmd = prev_pass->context.list;
								auto& prev_cpu_state = info->resource->get_state_manager().get_cpu_state(prev_cmd.get());
				

								   	HAL::SubResourcesGPU prev_gpu_state;
					prev_gpu_state.subres.resize(info->resource->get_state_manager().get_subres_count());
					prev_gpu_state.set_cpu_state(prev_cpu_state);

					info->resource->get_state_manager().prepare_state(commandList.get(), prev_gpu_state);
				}else
				{
					info->resource->get_state_manager().prepare_state(commandList.get(), last_state.merged_read_state);
	
				}
			//	


		
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
		for (auto* info : enabled_resources)
		{
			if (info->passed) continue;


			info->handler->init(*info);
			info->heap_type = HAL::HeapType::DEFAULT;

			/*if (check(info->flags & ResourceFlags::GenCPU))
			{
				info->heap_type = HAL::HeapType::UPLOAD;
			}

			if (check(info->flags & ResourceFlags::ReadCPU))
			{
				info->heap_type = HAL::HeapType::READBACK;
			}
					  */
			if (check(info->flags & ResourceFlags::Static)) continue;

		//	



			// if the resource is temp, first pass should create it -> write
			ASSERT(info->states[0].passes.size() == 1);
			ASSERT(info->states[0].write);


			Pass* best_creation_pass = info->states.front().passes.front();
			Pass* best_deletion_pass = nullptr;


			// create - easy
			events[best_creation_pass->call_id].create.insert(info);
			best_creation_pass->used.resource_creations.insert(info);


			if (info->heap_type != HAL::HeapType::DEFAULT) continue;


			bool alias_ended = false;
			for (auto pass : info->states.back().passes)
			{
				if (info->used_end.is_in_sync(pass->sync_state_with_self))
				{
					pass->used.resource_deletions_after.insert(info);
					alias_ended = true;
					break;
				}
			}
					  
					  // if no pass found - find any pass that is synced to the usage
			if (!best_deletion_pass)
			{

				for (auto pass : enabled_passes)
				{
					if (info->used_end.is_in_sync(pass->sync_state, false))
					{
						best_deletion_pass = pass;
						events[best_deletion_pass->call_id].free_before.insert(info);

						if (!alias_ended)
						best_deletion_pass->used.resource_deletions_before.insert(info);

						//	events[best_deletion_pass->call_id].free_after.insert(info);
						//best_deletion_pass->used.resource_deletions_after.insert(info);
						break;
					}
				}
					   
				if (!best_deletion_pass)
				{
					info->non_deleted = true;
					non_deleted.insert(info);
				}

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

					auto creation_info = RenderSystem::get().device().get_alloc_info(info->d3ddesc);
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

			for (auto* info : enabled_resources)
			{
				if (info->passed) continue;

				PROFILE(L"resource");



				if (info->alloc_ptr.handle)
				{
					auto& res = info->resource_places[info->alloc_ptr];

					if (!res || res.resource->get_desc() != info->d3ddesc)
					{
						PROFILE(L"placed");
						res.resource = HAL::create_resource(RenderSystem::get().device(), info->d3ddesc, info->alloc_ptr);
						//						 	res.resource->debug = info->name=="ResultTexture"; // TODO: move everywhere


	//					if (debug)
//res.resource->debug_transitions = info->name=="ShadowMask"; // TODO: move everywhere		


						res.resource->set_name(info->name());
						res.view = nullptr;
						res.resource->frame_graph_managed = true;
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

				/*	if (info->heap_type == HAL::HeapType::UPLOAD)
					{
						PROFILE(L"UPLOAD");

							auto creation_info = RenderSystem::get().device().get_alloc_info(info->d3ddesc);

						UploadInfo ui = current_frame->get_storage()->place_data(creation_info.size,creation_info.alignment);

						info->resource = ui.resource->get_ptr();
						info->offset_in_bytes = ui.resource_offset;
						info->is_new = true;
					}
					else if (info->heap_type == HAL::HeapType::READBACK)
					{
						PROFILE(L"READBACK");
						info->resource = HAL::create_resource(RenderSystem::get().device(), info->d3ddesc, info->heap_type);
						info->is_new = true;
					}
					else
						*/
						if (!info->resource || info->resource->get_desc() != info->d3ddesc)
					{
						PROFILE(L"DEFAULT");
						info->resource = HAL::create_resource(RenderSystem::get().device(), info->d3ddesc, info->heap_type);
						info->is_new = true;

					}





					//	info->resource = info->resource;
						//

					if (info->is_new)
					{
						info->resource->frame_graph_managed = true;
						info->creation_state =info->last_state = info->resource->get_state_manager().copy_gpu();
						info->last_state = TextureLayout::UNDEFINED;
						info->view = nullptr;
						info->resource->set_name(info->name());

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

	SyncState::SyncState() { reset(); }

	bool ResourceAllocInfo::is_static() const { return check(flags & ResourceFlags::Static); }
	bool ResourceAllocInfo::is_dynamic() const { return !check(flags & ResourceFlags::Static); }

	ResourceAllocInfo::CompiledResource::operator bool()
	{
		return !!resource;
	}

	HAL::ResourceDesc Handlers::ByteBufferDesc::create_resource_desc(ResourceFlags resflags)
	{
		HAL::ResFlags flags = HAL::ResFlags::ShaderResource;
		if (check(resflags & ResourceFlags::UnorderedAccess))
			flags |= HAL::ResFlags::UnorderedAccess;
		return HAL::ResourceDesc::Buffer(count, flags);
	}

	ByteBufferViewDesc Handlers::ByteBufferDesc::as_view(uint64 offset,ResourceFlags resflags)
	{
		return { offset, count };
	}

	HAL::ResourceDesc Handlers::TextureDesc::create_resource_desc(ResourceFlags resflags)
	{
		HAL::ResFlags flags = HAL::ResFlags::None;

		if (check(resflags & ResourceFlags::RenderTarget))
			flags |= HAL::ResFlags::RenderTarget;
		if (check(resflags & ResourceFlags::DepthStencil))
			flags |= HAL::ResFlags::DepthStencil;
		if (check(resflags & ResourceFlags::UnorderedAccess))
			flags |= HAL::ResFlags::UnorderedAccess;
		if (format.is_shader_visible())
			flags |= HAL::ResFlags::ShaderResource;

		if (mip_count == 0) {
			mip_count = 1;
			auto tsize = size;
			while (tsize.x != 1 && tsize.y != 1)
			{
				tsize = uint3::max(tsize/2, {1,1,1});
				mip_count++;
			}
		}
		return HAL::ResourceDesc::Tex2D(format, size.xy, array_count, mip_count, flags);
	}

	HAL::TextureViewDesc Handlers::TextureDesc::as_view(uint64 offset,ResourceFlags resflags)
	{
		return { 0, mip_count, 0, array_count };
	}

	HAL::ResourceDesc Handlers::Texture3DDesc::create_resource_desc(ResourceFlags resflags)
	{
		HAL::ResFlags flags = HAL::ResFlags::None;

		if (check(resflags & ResourceFlags::RenderTarget))
			flags |= HAL::ResFlags::RenderTarget;
		if (check(resflags & ResourceFlags::DepthStencil))
			flags |= HAL::ResFlags::DepthStencil;
		if (check(resflags & ResourceFlags::UnorderedAccess))
			flags |= HAL::ResFlags::UnorderedAccess;
		if (format.is_shader_visible())
			flags |= HAL::ResFlags::ShaderResource;

		if (mip_count == 0) {
			mip_count = 1;
			auto tsize = size;
			while (tsize.x != 1 && tsize.y != 1 && tsize.z != 1)
			{
				tsize /= 2;
				mip_count++;
			}
		}
		return HAL::ResourceDesc::Tex3D(format, size, mip_count, flags);
	}

	HAL::Texture3DViewDesc Handlers::Texture3DDesc::as_view(uint64 offset,ResourceFlags resflags)
	{
		return { 0, mip_count };
	}

	HAL::ResourceDesc Handlers::CubeDesc::create_resource_desc(ResourceFlags resflags)
	{
		HAL::ResFlags flags = HAL::ResFlags::None;

		if (check(resflags & ResourceFlags::RenderTarget))
			flags |= HAL::ResFlags::RenderTarget;
		if (check(resflags & ResourceFlags::DepthStencil))
			flags |= HAL::ResFlags::DepthStencil;
		if (check(resflags & ResourceFlags::UnorderedAccess))
			flags |= HAL::ResFlags::UnorderedAccess;
		if (format.is_shader_visible())
			flags |= HAL::ResFlags::ShaderResource;

		if (mip_count == 0) {
			mip_count = 1;
			auto tsize = size;
			while (tsize.x != 1 && tsize.y != 1)
			{
				tsize /= 2;
				mip_count++;
			}
		}
		return HAL::ResourceDesc::Tex2D(format, size.xy, array_count * 6, mip_count, flags);
	}

	HAL::CubeViewDesc Handlers::CubeDesc::as_view(uint64 offset,ResourceFlags resflags)
	{
		return { 0, mip_count, 0, array_count * 6 };
	}

	uint32_t Pass::GetPassIndex() const { return pass_index; }

	HAL::CommandListType Pass::get_type() const
	{
		HAL::CommandListType type = HAL::CommandListType::DIRECT;
		if (check(flags & PassFlags::Compute))
			type = HAL::CommandListType::COMPUTE;
		return type;
	}

	bool Pass::active()
	{
		return enabled && renderable;
	}

	void Pass::reset_frame()
	{
		enabled = false;
		renderable = true;
		dependency_level = 0;

		used.fences.clear();
		used.resources.clear();
		used.resource_flags.clear();
		used.resource_creations.clear();
		used.resource_deletions_before.clear();
		used.resource_deletions_after.clear();

		context.graph = nullptr;
		context.pass = nullptr;
		context.frame = nullptr;
		context.list = nullptr;

		sync_state.reset();
		sync_state_with_self.reset();

		prev_passes.clear();
		next_passes.clear();

		render_task = std::future<void>();
		compile_task = std::future<void>();

		debug_commands.clear();

		fence_end = HAL::FenceWaiter();

		graphic_count = 0;
		compute_count = 0;
		wait_pass = nullptr;

		put_fence = false;
		prev_pass = nullptr;

		inserted = false;
	}

	ExternalPass::ExternalPass()
	{
		name       = s_name;
		id         = std::numeric_limits<UINT>::max();
		enabled    = true;
		renderable = true;
		flags      = PassFlags::General;
	}

	bool ExternalPass::setup(TaskBuilder&) { return true; }

	void ExternalPass::render(Graph* graph, HAL::FrameResources::ptr& frame)
	{
		render_task = thread_pool::get().enqueue([this, &frame, graph]()
		{
			context.begin(graph, this, frame);
			// No GPU work — context.end() fires process_debug_resource for each
			// write-flagged passed resource, which triggers thumbnail capture.
			context.end();
		});
	}

	void SlotContext::set_slot(SlotID id, HAL::SignatureDataSetter& setter)
	{
		//ASSERT(slot_setters.contains(id));
		slot_setters[id](setter);
	}

	HAL::StructuredBufferView<DispatchArguments>& FrameContext::get_indirect_dispatch_args()
	{
		return graph->indirect_dispatch_args[pass->get_type()];
	}

	Graph::Graph() : VariableContext(L"Graph")
	{
		builder.graph = this;

		auto& device = RenderSystem::get().device();
		for (auto& buf : indirect_dispatch_args)
			buf = HAL::StructuredBufferView<DispatchArguments>(device, 1);

		DispatchArguments init{ 0, 1, 1 };
		auto upload = device.get_upload_list();
		for (auto& buf : indirect_dispatch_args)
			upload->get_copy().update(buf, 0, std::span{ &init, 1 });
		upload->execute_and_wait();
	}

	void Graph::set_pipeline(Pipelines::PipelineBase* p) { current_pipeline = p; }
	Pipelines::PipelineBase* Graph::get_pipeline() const { return current_pipeline; }

	Pass* TaskBuilder::get_pass(uint id) const
	{
		auto it = id_to_pass.find(id);
		return it != id_to_pass.end() ? it->second : nullptr;
	}

	ResourceAllocInfo* TaskBuilder::get(ResourceID id)
	{
		auto& chain = alloc_resources[(size_t)id];
		if (chain.empty()) return nullptr;
		ResourceAllocInfo& info = chain.active();
		if (!info.enabled) return nullptr;
		return &info;
	}
}