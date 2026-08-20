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
		bool is_exclusive = check(flags & ResourceFlags::ExclusiveRead);
		bool needs_new_state = is_writer || is_exclusive
			|| states.empty() || states.back().write || states.back().exclusive;

		if (needs_new_state)
		{
			states.push(is_writer, max_passes);
			states.back().exclusive = is_exclusive;
		}

		states.back().passes.emplace_back(pass);
	}


	void ResourceAllocInfo::reset(size_t new_max_passes)
	{
		max_passes = new_max_passes;
		states.reset_frame();

		is_new = false;
		used_begin.reset();
		used_end.reset();
		enabled = false;
	}

	void TaskBuilder::link_history(ResourceID current, ResourceID prev)
	{
		for (auto& l : history_links)
			if (l.current == current && l.prev == prev) return; // already linked

		HistoryLink l;
		l.current = current;
		l.prev    = prev;
		history_links.push_back(std::move(l));
	}

	TaskBuilder::HistoryLink* TaskBuilder::history_by_current(ResourceID id)
	{
		for (auto& l : history_links)
			if (l.current == id) return &l;
		return nullptr;
	}

	TaskBuilder::HistoryLink* TaskBuilder::history_by_prev(ResourceID id)
	{
		for (auto& l : history_links)
			if (l.prev == id) return &l;
		return nullptr;
	}

	void TaskBuilder::roll_history()
	{
		// End of frame. For each link: free the carried slot (last frame's current,
		// consumed this frame as prev), then carry this frame's current allocation
		// forward to serve as next frame's prev. current's allocation was kept alive
		// by suppressing its free in create_resources (is_history_current), so it is
		// still valid here; detach it from the info so nothing else frees it.
		for (auto& l : history_links)
		{
			// Release the slot consumed TWO frames ago (see HistoryLink comment:
			// prev's compute-queue readers have no fence edge against next
			// frame's direct-queue transients, so the just-consumed range must
			// not return to the allocator yet).
			l.pending_release.alloc_ptr.handle.Free();

			// The slot consumed this frame as prev: keep the resource + range
			// alive one more frame, release on the next roll.
			l.pending_release = l.carried;
			l.carried = HistorySlot{};

			auto& cur_chain = alloc_resources[(size_t)l.current];
			if (cur_chain.empty()) continue;
			ResourceAllocInfo& cur = cur_chain.active();

			if (!cur.resource || !cur.alloc_ptr.handle) continue; // wasn't created this frame

			l.carried.resource   = cur.resource;
			l.carried.alloc_ptr  = cur.alloc_ptr;
			l.carried.desc       = cur.d3ddesc;

			// Detach so the normal free pass / next frame's realloc don't touch it.
			cur.alloc_ptr = HAL::ResourceHandle{};
		}
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



		states.remove_if([](ResourceRWState& s) { return s.passes.empty(); });

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

		// Handing a resource to the FrameGraph makes the FrameGraph responsible
		// for its transitions -- same contract as a resource the graph created
		// itself, so it carries the same marker and opts out of the per-group
		// return-to-rest in CommandListGroup::compile_transitions.
		tex->frame_graph_managed = true;

		auto tex_desc = tex->get_desc().as_texture();
		if (tex_desc.is2D())
		{
			Handlers::Texture h(id);
			create(h, { ivec3(0,0,0), HAL::Format::UNKNOWN, 0 }, flags);
			auto& info = *h.info;
			info.passed = true;
			info.enabled = true;
			info.resource = tex;
			info.fence = fence;
			info.d3ddesc = tex->get_desc();

			// From the resource, not assumed: create_resources() -- the only other
			// place that sets heap_type -- skips passed resources entirely, so
			// without this the field keeps its default-constructed value. Anything
			// filtering on it then silently drops every passed resource;
			// process_transitions did exactly that, which is why a passed texture
			// was linked by nobody and rested by nobody (pass_texture also marks it
			// frame_graph_managed, opting it out of the per-group rest).
			info.heap_type = tex->get_heap_type();
			passed_resources.insert(&info);


			h.desc.array_count = tex->get_desc().as_texture().ArraySize;
			h.desc.format = tex->get_desc().as_texture().Format;
			h.desc.mip_count = tex->get_desc().as_texture().MipLevels;
			h.desc.size = tex->get_desc().as_texture().Dimensions;

			tex->set_name(FrameGraph::resource_id_name(id));

			h.init_view(info, *current_frame);


		}
		else if (tex_desc.is3D())
		{
			Handlers::Texture3D h(id);
			create(h, { ivec3(0,0,0), HAL::Format::UNKNOWN, }, flags);
			auto& info = *h.info;

			tex->set_name(FrameGraph::resource_id_name(id));


			info.passed = true;
			info.enabled = true;
			info.resource = tex;
			info.d3ddesc = tex->get_desc();
			info.fence = fence;
			info.heap_type = tex->get_heap_type();   // see the 2D branch above
			passed_resources.insert(&info);

			h.desc.format = tex->get_desc().as_texture().Format;
			h.desc.mip_count = tex->get_desc().as_texture().MipLevels;
			h.desc.size = tex->get_desc().as_texture().Dimensions;

			h.init_view(info, *current_frame);
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

			for (auto [info, flags] : pass->used.resource_flags)
			{
				// Debug preview fires on the producing (write) pass. A history `prev`
				// is never written by a pass — fire it on a reading pass instead so
				// its thumbnail can be captured from the adopted resource.
				if (!check(flags & WRITEABLE_FLAGS) && !info->is_history_prev) continue;
				info->process_debug_resource(pass, this);

				// The ExternalPass exists only so the debugger can capture; it does
				// no rendering of its own and is deliberately NOT part of any
				// resource's state timeline, so process_transitions never links it
				// to the passes around it. Linking it would mean a fence for a pass
				// with no GPU work, which is not worth paying for a debug feature.
				//
				// So it has to be transparent instead: the capture moved the
				// resource (to SHADER_RESOURCE, to sample it), and nothing else
				// will move it back -- a frame_graph_managed resource is exempt
				// from the per-group return to rest. Hand it back at rest here,
				// which is the state it was found in, and the state the next group
				// assumes for its first touch. Without this the back buffer stayed
				// in SHADER_RESOURCE while UI_Render_0 opened its group declaring
				// PRESENT (#1334, once per frame).
				if (pass->id == std::numeric_limits<UINT>::max() && info->resource)
					list->transition_to_rest(info->resource.get());
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

			// Promote any freshly-uploaded resources into their canonical read state
			// before passes record (so seeds see the read state) and before this
			// frame's lists are submitted, so uploaded assets rest in a read state
			// rather than COMMON and never rely on implicit promotion (#1334).
			RenderSystem::get().device().flush_uploads();

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

		// Setup only recorded resource desc/flags + ordered touches; build the
		// per-resource RW-state timelines now, after every setup has run.
		builder.build_resource_states();

		{

			PROFILE(L"enabled");
	
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
			{
				// A later chain version being Required implies every earlier
				// version it was recreated from is Required too (they produce the
				// content the final version clones/depends on). Propagate the flag
				// backward through the chain before enabling creators.
				bool required = false;
				for (auto& info : chain.active_span() | std::ranges::views::reverse)
				{
					required = required || check(info.flags & ResourceFlags::Required);
					if (required)
						info.flags |= ResourceFlags::Required;
				}

				for (auto& info : chain.active_span())
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

		}

		for (auto pass : builder.passes)
		{
			if (!pass->active()) continue;
			auto pass_ptr = pass.get();

			builder.enabled_passes.emplace_back(pass_ptr);

			if (!optimize)
			{
				pass->flags = pass->flags & ~(PassFlags::Compute);
			}

		}

		int i = 1;


		{

			PROFILE(L"sync_state");

			for (auto pass : builder.enabled_passes)
			{
				pass->call_id = i++;

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


		}


		// Inject a fake pass that owns passed and static resources as their creator.
		// Added after call_id/sync-state assignment so it doesn't disturb scheduling;
		// call_id=0 places it before all real passes in the timeline.
		{
				PROFILE(L"ExternalPass");
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

			PROFILE(L"sync_state final");

		for (auto& chain : builder.alloc_resources)
			for (auto& info : chain.active_span())
			{
				if (!info.enabled) continue;

				info.remove_inactive();


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

		// Runs after every pass has recorded: process_transitions appends
		// producer-side hand-off and resting transitions to lists that are
		// already complete, so it needs their final operation sequence.
		builder.process_transitions();
		builder.process_fences();

		builder.compile_lists();



		on_compile(*this);
	}


	HAL::FenceWaiter Graph::commit_command_lists()
	{

		HAL::FenceWaiter result;






		PROFILE(L"submitting lists");

		// enum_array, not std::map: the key is a 3-value enum, so a red-black
		// tree rebuilt every frame buys nothing over direct indexing.
		// Batches waiting to be submitted, per queue. A batch IS the barrier
		// group: it is exactly the set of lists that will go in one
		// ExecuteCommandLists, and the flush points below (a gpu_wait, a fence,
		// end of frame) are precisely the group boundaries.
		enum_array<CommandListType, HAL::CommandListGroup> queued_lists;

		// Resolve a group's recorded Transition points into plain data for the
		// debugger. Must run after the group's barriers are computed (so the
		// groups are filled) and BEFORE it is submitted: barrier_point points
		// into the list's `operations`, which Transitions::on_execute clears as
		// soon as the list finishes executing on the submit thread.
		std::map<HAL::CommandList*, Pass*> pass_of_list;
		if constexpr (BuildOptions::Dev)
			for (auto& pass : builder.enabled_passes)
				if (pass->context.list)
					pass_of_list[pass->context.list.get()] = pass;

		auto snapshot_group = [&](HAL::CommandListGroup& group)
		{
			if constexpr (BuildOptions::Dev)
			{
				PROFILE(L"debug_snapshot");
				for (auto& list : group.get_lists())
				{
					auto it = pass_of_list.find(list.get());
					if (it == pass_of_list.end()) continue;

					auto records = list->take_debug_records();

					// One name lookup per distinct resource instead of one per
					// barrier. A resource is typically barriered many times in a
					// list, and most names are past the small-string limit, so
					// this is the difference between a heap allocation per
					// barrier and one per resource.
					//
					// The name has to be captured eagerly rather than read from
					// b.resource later: these records outlive the frame that
					// produced them, and a transient resource can be freed or
					// aliased away in the meantime. That is why resource_id is
					// documented as never dereferenced.
					std::unordered_map<const HAL::Resource*, std::string> names;

					for (auto& rec : records)
					{
						if (rec.type != HAL::CommandType::Transition || !rec.barrier_point) continue;

						const auto& barriers = rec.barrier_point->get_barriers();
						rec.description = "Barriers: " + std::to_string(barriers.size());
						rec.barrier_details.reserve(barriers.size());
						for (const auto& b : barriers)
						{
							HAL::CommandRecord::BarrierDetail detail;

							auto name_it = names.find(b.resource);
							if (name_it == names.end())
								name_it = names.emplace(b.resource, b.resource ? b.resource->name : "?").first;
							detail.resource_name = name_it->second;

							detail.before = b.before;
							detail.after = b.after;
							detail.subres = HAL::representative_subres(b.resource, b.range);
							detail.range  = b.range;
							detail.flags = b.flags;
							detail.resource_id = reinterpret_cast<uint64>(b.resource);   // opaque instance id
							rec.barrier_details.push_back(std::move(detail));
						}
						rec.barrier_point = nullptr;
					}

					it->second->debug_commands = std::move(records);
				}
			}
		};

		// Frame-boundary cross-queue sync: each queue waits for the OTHER
		// queues' end of the previous frame before this frame's first
		// submission. Transient heap ranges freed at frame N's tail are
		// re-placed at frame N+1's head; without this, N+1's direct-queue
		// writers can race N's async-compute tail still reading the same
		// memory (confirmed via the '5' no-aliasing experiment). Costs the
		// cross-frame queue overlap — revisit when moving to an epoch-tagged
		// allocator for the async-compute frame-split plans.
		for (auto wait_type : magic_enum::enum_values<CommandListType>())
			for (auto signal_type : magic_enum::enum_values<CommandListType>())
			{
				if (wait_type == signal_type) continue;
				if (!prev_frame_end_fence[signal_type]) continue;
				RenderSystem::get().device().get_queue(wait_type)->gpu_wait(prev_frame_end_fence[signal_type]);
			}

		enum_array<CommandListType, HAL::FenceWaiter> frame_end_fence;

		// One batch that will become a single ExecuteCommandLists, with the
		// ordering work that has to happen around it. Barrier computation is
		// the expensive part and is pure once plan_resources() has run, so it
		// is hoisted out of this loop and done for every batch at once, in
		// parallel; submission below stays strictly ordered because fences
		// flow between batches.
		struct PendingSubmit
		{
			HAL::CommandListGroup    group;
			HAL::CommandListType     type;
			std::vector<const Pass*> waits;                // gpu_wait these before submitting
			Pass*                    fence_pass = nullptr; // receives fence_end, if any
		};
		std::vector<PendingSubmit> submits;

		// Close the batch open on `type`: decide its first-use questions (this
		// is the only part of the barrier work that must happen in submission
		// order) and queue it for compilation.
		auto close_batch = [&](HAL::CommandListType type, Pass* fence_pass, std::vector<const Pass*> waits = {})
		{
			auto& group = queued_lists[type];
			if (group.empty() && waits.empty()) return;

		

			submits.push_back(PendingSubmit{ std::move(group), type, std::move(waits), fence_pass });
			group.clear();
		};

		for (auto& pass : builder.enabled_passes)
		{
			HAL::CommandListType list_type = pass->get_type();

			auto commandList = pass->context.list;

			if (commandList)
			{
				std::vector<const Pass*> waits;
				for (auto sync_pass : pass->sync_state.values)
					if (sync_pass) waits.push_back(sync_pass);

				if (!waits.empty())
					close_batch(list_type, nullptr, std::move(waits));

				queued_lists[list_type].add(commandList);

				// Diagnostic: full queue serialization — every pass gets a
				// fence and every OTHER queue waits on it, so no two passes
				// ever run concurrently across queues. If garbage persists
				// with this on, the corruption is NOT an async/lifetime race
				// (look for an out-of-bounds writer instead).
				if (serialize_queues)
					pass->put_fence = true;

				if (pass->put_fence)		//////////////////////// ARGH!!!!
					close_batch(list_type, pass);
			}


		}

		// Whatever is still open at end of frame.
		for (auto type : magic_enum::enum_values<CommandListType>())
			close_batch(type, nullptr);

		// Resolve every batch's first-use questions FIRST, on this thread, in
		// submission order.
		//
		// This cannot move into the parallel fan-out below: plan_resources()
		// reads and mutates Resource::virgin / Resource::initialized, which are
		// per-RESOURCE and shared by every group that touches them. Running it
		// concurrently races on "which group owns this resource's first-ever
		// write", so the initializing discard lands on a nondeterministic group
		// -- and in submission order it may land on a group that executes after
		// the one that needed it.
		{
			PROFILE(L"plan_groups");
			for (auto& submit : submits)
				submit.group.plan_resources();
		}

		// Every batch's barriers, computed in parallel. plan_resources() has
		// already resolved everything order-dependent, so these touch nothing
		// shared: `current`/`wanted` are group-local and the Barriers they fill
		// belong to their own lists.
		{
			PROFILE(L"compile_groups");

			std::vector<std::future<void>> tasks;
			tasks.reserve(submits.size());

			for (auto& submit : submits)
				tasks.emplace_back(thread_pool::get().enqueue([&submit]()
				{
					submit.group.compile_transitions();
					submit.group.compile();
				}));

			for (auto& t : tasks)
				t.wait();
		}

		// Ordered submission. Cheap now -- the batches are already compiled --
		// but it must stay in order: a batch's fence_end is consumed by a later
		// batch's gpu_wait, and gpu_wait/execute derive their relative order
		// from the order they are posted to the queue's executor thread.
		{
			PROFILE(L"submit_groups");

			for (auto& submit : submits)
			{
				auto queue = RenderSystem::get().device().get_queue(submit.type);

				for (const auto* sync_pass : submit.waits)
					queue->gpu_wait(sync_pass->fence_end);

				if (submit.group.empty()) continue;

				snapshot_group(submit.group);
				result = queue->execute(submit.group);

				frame_end_fence[submit.type] = result;
				if (submit.fence_pass)
					submit.fence_pass->fence_end = result;

				if (serialize_queues)
					for (auto other : magic_enum::enum_values<CommandListType>())
					{
						if (other == submit.type) continue;
						RenderSystem::get().device().get_queue(other)->gpu_wait(result);
					}
			}
		}

		// Remember each queue's final fence for next frame's boundary wait.
		prev_frame_end_fence = frame_end_fence;


		PROFILE(L"free resources");

		// Carry each history current's allocation forward and free the consumed
		// prev — before the normal free pass, so current's detached allocation
		// isn't touched by it (and the carried resource keeps a live reference).
		builder.roll_history();

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
		added_pipelines.clear();

	}



	// Verifies the imperative create/need write-flag against the pass's static
	// [Write] declaration (resource_accesses[]). Only checks resources that
	// appear exactly once in the declaration — recreate/chained resources (id
	// listed more than once) are skipped for now, and undeclared resources
	// (count 0) are a separate concern.
	static void verify_declared_access(Pass* pass, ResourceID id, ResourceFlags flags)
	{
		if (!pass) return;

		auto declared = pass->declared_accesses();
		int  found = 0;
		bool declared_write = false;
		for (const auto& a : declared)
			if (a.id == id) { ++found; declared_write = a.write; }

		if (found != 1) return;

		bool actual_write = check(flags & WRITEABLE_FLAGS);
		if (declared_write != actual_write)
		{
			Log::get() << "[FrameGraph] [Write] mismatch: pass '" << pass->name.ptr
				<< "' resource '" << resource_id_name(id)
				<< "': SIG declares " << (declared_write ? "write" : "read")
				<< " but runtime does " << (actual_write ? "write" : "read")
				<< Log::endl;
			ASSERT(declared_write == actual_write);
		}
	}

	void TaskBuilder::init(ResourceAllocInfo& info, ResourceFlags flags)
	{
		verify_declared_access(current_pass, info.id, flags);

		// All passes must be registered (add_passes) before any resource is
		// created: max_passes is captured here from passes.size() and used to
		// pre-size the per-state concurrent_vector append storage. Creating a
		// resource while passes is still empty (e.g. passing the swapchain in
		// before add_passes) under-sizes that storage and overflows on later
		// need()s from real passes.
		ASSERT(!passes.empty());

		info.reset(passes.size());
		info.flags = flags;
		info.frame_id = current_frame->get_frame();

		// History roles (see create_resources / roll_history). current allocates
		// fresh but defers its free; prev adopts the carried allocation.
		info.is_history_current = history_by_current(info.id) != nullptr;
		info.is_history_prev    = history_by_prev(info.id)    != nullptr;
	
		//info.valid_from = info.valid_to = info.valid_to_start = nullptr;

		if (current_pass) {
			// Setup only records what the pass touched + the flags; states are
			// built after all setups (build_resource_states).
			current_pass->used.resources.insert(&info);
			current_pass->used.resource_flags[&info] = flags;
		}
	}

	void TaskBuilder::init_pass(ResourceAllocInfo& info, ResourceFlags flags)
	{
		verify_declared_access(current_pass, info.id, flags);
		current_pass->used.resources.insert(&info);
		current_pass->used.resource_flags[&info] = flags;
		info.is_new = false;
		info.flags = info.flags | flags;

		if (info.fence)
		{
			current_pass->used.fences.emplace_back(info.fence);
		}
	}

	// Builds each resource's ResourceRWState timeline after all setups have run,
	// by replaying what every pass touched (used.resources) in pipeline order.
	// This is O(touches) with no intermediate maps: add_pass appends into the
	// exact ResourceAllocInfo (so recreate chain versions stay separate), and a
	// pass that conditionally skipped a resource simply isn't in used.resources.
	//
	// The per-pipeline precomputed resource_infos aren't used here: filtering the
	// static template against actual touches (mandatory for conditional need()s)
	// costs more than this direct replay saves.
	void TaskBuilder::build_resource_states()
	{
		PROFILE(L"build_resource_states");

		for (auto& pass : passes)
			for (auto* info : pass->used.resources)
				info->add_pass(pass.get(), pass->used.resource_flags[info]);
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

						// The producer must be able to signal. commit_command_lists
						// only assigns fence_end inside its `if (commandList)` block,
						// so a producer without a list leaves fence_end unset — and
						// Queue::gpu_wait() opens with `if (!waiter) return;`. The
						// cross-queue wait would then vanish with no error anywhere.
						// remove_inactive() filters sync sources by active()
						// (enabled && renderable), which does NOT imply a list.
						ASSERT(sync_pass->context.list);

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


		descriptor_commit_task = thread_pool::get().enqueue([this]()
			{
				current_frame->commit_descriptors_to_gpu();
			});


		// Lists are compiled by their CommandListGroup at submission time --
		// compile() consumes the barrier groups, so it has to follow the
		// group's compile_transitions(). Nothing per-pass to do here.


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



	// Cross-pass linking for FrameGraph-owned resources.
	//
	// A command-list group can only reason about what it can see: it enters a
	// resource from its resting layout and leaves it there. That works for
	// resources nobody else schedules, but a FrameGraph resource is written by
	// one pass and read by the next, often with a fence between them -- routing
	// it through the resting layout at every boundary would be both wrong (the
	// group cannot know where it actually is) and expensive.
	//
	// So the graph, which knows the whole pass timeline, does three things here:
	//   1. merges concurrent readers onto one shared state, so several passes
	//      reading the same resource cost one transition rather than one each;
	//   2. tells each consuming list what state the producing pass left the
	//      resource in (set_entry_state), which is what lets a group see over a
	//      boundary it would otherwise stop at;
	//   3. returns the resource to its resting layout once, at the last pass
	//      that touches it -- not at every group boundary.
	//
	// Replaces the pre-2026-08 version of this function, which did the same jobs
	// through ResourceStateManager's prepare_state / prepare_after_state seed
	// nodes and per-subresource SubResourcesGPU snapshots.
	void TaskBuilder::process_transitions()
	{
		PROFILE(L"process_transitions");

		for (auto& chain : alloc_resources)
			for (auto& info : chain.active_span())
			{
				if (!info.enabled) continue;

				auto& resource = info.resource;


				if (!resource) continue;
				// Skip only the CPU-visible heaps, which carry no layout to link.
				// NOT "anything that isn't DEFAULT": RESERVED covers both tiled
				// resources and natively imported ones (the swapchain arrives that
				// way -- Resource::init(NativeImportHandle) never assigns a heap
				// type, so it keeps the RESERVED sentinel), and those need linking
				// exactly as much as a DEFAULT resource does. This mirrors the same
				// choice HAL makes when deciding what to barrier-track at all
				// (Transitions::add_resource_usage).
				if (info.heap_type == HAL::HeapType::UPLOAD ||
					info.heap_type == HAL::HeapType::READBACK) continue;
				if (!resource->frame_graph_managed) continue;   // the group rests these itself

				// Drop passes that never actually touched the resource -- a pass
				// can declare a resource and then not use it, and linking through
				// it would hand state to a list that has nothing to receive it.
				auto untouched = [&](Pass* pass)
				{
					auto& list = pass->context.list;
					if (!list) return true;
					return !list->get_exit_state(resource.get()).has_value();
				};

				for (auto& state : info.states)
				{
					std::vector<Pass*> kept;
					kept.reserve(state.passes.size());
					for (auto* pass : state.passes)
						if (!untouched(pass)) kept.push_back(pass);

					if (kept.size() == state.passes.size()) continue;

					state.passes.clear();
					for (auto* pass : kept) state.passes.push_back(pass);
				}

				info.states.remove_if([](const ResourceRWState& s) { return s.passes.empty(); });

				if (info.states.empty()) continue;

				// A barrier's BEFORE state has to be expressible on the queue that
				// records it: a compute list cannot barrier a resource out of
				// RENDER_TARGET, and asserting on that is exactly what
				// Barriers::transition does. Linking is the only thing that can
				// hand a list a state some OTHER queue produced, so every state
				// this function hands over is checked first.
				auto supported = [&](Pass* pass, const HAL::ResourceState& state)
				{
					return HAL::IsFullySupport(pass->context.list->get_type(), state);
				};

				// The pass in a phase that actually runs LAST. Not passes.back():
				// ResourceRWState::passes is a concurrent_vector appended from
				// several threads during setup, so its order is arbitrary. Using
				// the back element put a phase's hand-off (and the return to rest)
				// on whichever pass happened to be appended last, which for a
				// multi-pass phase lands mid-frame with other passes still to
				// come -- the back buffer went to PRESENT while UI_Render_1..4
				// were still writing it. call_id is the execution order.
				auto last_of = [](const auto& passes) -> Pass*
				{
					Pass* best = nullptr;
					for (auto* p : passes)
						if (!best || p->call_id > best->call_id) best = p;
					return best;
				};

				// --- 1. one shared state per read phase --------------------------
				//
				// Readers of the same version can disagree on sync (a pixel-shader
				// read vs a compute read). Left alone each would transition the
				// resource again; declaring the union on every reader makes them
				// agree, so only the first emits a barrier and the rest are no-ops.
				std::vector<std::optional<HAL::ResourceState>> merged_read(info.states.size());

				for (uint i = 0; i < info.states.size(); i++)
				{
					auto& state = info.states[i];
					if (state.write || state.exclusive) continue;

					std::optional<HAL::ResourceState> merged;
					for (auto* pass : state.passes)
					{
						auto exit = pass->context.list->get_exit_state(resource.get());
						if (!exit) continue;

						if (!merged) { merged = *exit; continue; }
						if (auto both = HAL::merge_state(*merged, *exit)) merged = *both;
					}

					if (!merged) continue;

					// Readers on different queues can merge into a state one of
					// them cannot record (a pixel-shader read unioned onto a
					// compute list). Then there is no single state to agree on --
					// leave every reader with its own and pay the extra barrier.
					bool all_can = true;
					for (auto* pass : state.passes)
						if (!supported(pass, *merged)) { all_can = false; break; }
					if (!all_can) continue;

					merged_read[i] = merged;

					// Propagate it back so every reader asks for the same thing.
					for (auto* pass : state.passes)
						pass->context.list->add_resource_usage(resource.get(), *merged, HAL::ALL_SUBRESOURCES);
				}

				// --- 2. link each state to the one before it ---------------------
				//
				// The consumer is told where the producer left the resource. Only
				// used when the consumer's group has not already tracked it, so a
				// producer and consumer that land in the SAME group cost nothing
				// here -- the group threads the state itself.
				for (uint i = 1; i < info.states.size(); i++)
				{
					auto& prev = info.states[i - 1];
					auto& cur  = info.states[i];
					if (prev.passes.empty() || cur.passes.empty()) continue;

					Pass* producer = last_of(prev.passes);
					auto& producer_list = producer->context.list;

					// The hand-off state has to be the SAME for every pass in the
					// consuming phase: they are told what they will find, and a
					// phase with several readers has no ordering among them. So
					// the resource is converged at the producer -- one barrier on
					// the producing list, none on any reader -- rather than
					// leaving the first reader to transition it and the rest to
					// discover it already transitioned.
					//
					// The phase's merged read state is the natural target: step 1
					// already made every reader ask for exactly that.
					std::optional<HAL::ResourceState> handoff = merged_read[i];

					// Must also be recordable on the producer's own queue -- a
					// compute list cannot transition anything into a pixel-shader
					// read. Resting works from and to any queue, so it is the
					// fallback for every case the direct hand-off cannot express.
					if (handoff && !HAL::IsFullySupport(producer_list->get_type(), *handoff))
						handoff.reset();

					// Not a read phase, so there is no merged read state to aim
					// for. Aim instead for what the consuming passes need at
					// their FIRST use -- the state they would otherwise each
					// transition into themselves.
					//
					// This has to be a state they ALL agree on. Handing them the
					// producer's exit instead is only true for whichever consumer
					// happens to run first; the others find it already
					// transitioned. (Tried 2026-08-19: 6269 x #1334 on
					// VoxelLighted, GBuffer_DepthMips, BlueNoise.) Converging the
					// producer into their common first-use state costs the
					// consumers no barrier at all, so no ordering among them is
					// needed -- the same reason merged_read works for readers.
					if (!handoff)
					{
						std::optional<HAL::ResourceState> common;
						bool agree = true;

						for (auto* pass : cur.passes)
						{
							auto first = pass->context.list->get_first_use_state(resource.get());
							if (!first) continue;

							if (!common) common = first;
							else if (!(*common == *first)) { agree = false; break; }
						}

						if (agree && common && HAL::IsFullySupport(producer_list->get_type(), *common))
							handoff = common;
					}

					const bool rested = !handoff;
					if (rested)
						handoff = HAL::state_at_rest(HAL::resting_layout(resource.get()));


					producer_list->transition_to(resource.get(), *handoff);

					for (auto* pass : cur.passes)
						pass->context.list->set_entry_state(resource.get(), *handoff);
				}


				// --- 3. rest once, at the last pass that touches it --------------
				//
				// Read or write -- whichever comes last. This is the single point
				// where an FG resource returns to a state any queue can pick up,
				// replacing the per-group convergence that CommandListGroup skips
				// for frame_graph_managed resources.
				//
				// Deliberately unconditional, and deliberately the resting state
				// rather than whatever the last pass happened to leave behind.
				// The resting layout is HAL's global contract -- every group
				// enters a resource from it, and code outside the graph does too
				// -- so it is the ONLY end state that stays valid no matter who
				// picks the resource up next. Carrying the real end state into
				// next frame instead would be strictly worse: a resource left in
				// DEPTH_STENCIL_WRITE cannot be barriered out of by a compute
				// queue at all, so next frame's first pass could be handed a
				// before-state it has no way to express.
				//
				// That also means there is nothing to carry across frames: the
				// next frame's first pass needs no entry state, because the
				// barrier system's own resting-layout default is already right.
				Pass* last_pass = last_of(info.states.back().passes);
				if (auto& list = last_pass->context.list)
				{
					// Resting here is correct only because last_pass is the phase's
					// LAST-EXECUTING pass. Getting that wrong is not a lost
					// optimisation but a correctness bug: a back buffer rested to
					// PRESENT while later passes still write it makes every
					// subsequent use find PRESENT where it expected a readable
					// layout (#1334 "layout(COMMON) does not match expected
					// (SHADER_RESOURCE)").
					list->transition_to_rest(resource.get());
				}
			}
	}

	void TaskBuilder::create_resources()
	{
		  bool delete_resources = !GetAsyncKeyState('5');
		struct Events
		{
			std::set<ResourceAllocInfo*> create;
			std::set<ResourceAllocInfo*> free_before;
			std::set<ResourceAllocInfo*> free_after;

		};

		std::map<int, Events> events;
		std::set<ResourceAllocInfo*> non_deleted;

		// Which pass actually writes each resource first (after the active()
		// fall-forward below). The aliasing validation needs this pass's
		// sync_state: it is the one that touches recycled memory, and it is NOT
		// the pass the free-side guard is checked against.
		std::map<ResourceAllocInfo*, Pass*> creation_pass_of;

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

			// A history `prev` doesn't allocate — it adopts the carried allocation
			// in the placement loop below. Skip all alloc/free scheduling (and the
			// first-state-is-write assert, since prev is read-only this frame).
			if (info->is_history_prev) continue;

			//	



				// if the resource is temp, first pass should create it -> write
			ASSERT(info->states[0].passes.size() == 1);
			ASSERT(info->states[0].write);


			Pass* best_creation_pass = info->states.front().passes.front();
			Pass* best_deletion_pass = nullptr;

			// The creating pass must actually execute. A pass whose setup()
			// returned false (renderable == false — e.g. ResultCreation, which
			// only declares ResultTexture and records nothing) never runs
			// FrameContext::begin, so its alias_begin — the DISCARD that
			// initializes a placed/aliased resource — would never be emitted.
			// The first real use then hits an uninitialized resource carrying a
			// stale tracked layout (D3D12 #1422 + INCOMPATIBLE_BARRIER_LAYOUT).
			// Fall forward to the first pass that actually records commands.
			if (!best_creation_pass->active())
			{
				for (auto& st : info->states)
				{
					for (auto p : st.passes)
						if (p->active()) { best_creation_pass = p; break; }

					if (best_creation_pass->active()) break;
				}
			}

			// create - easy
			events[best_creation_pass->call_id].create.insert(info);
			best_creation_pass->used.resource_creations.insert(info);
			creation_pass_of[info] = best_creation_pass;


			if (info->heap_type != HAL::HeapType::DEFAULT) continue;

			// A history `current` allocates fresh (above) but its allocation is
			// carried one frame forward (roll_history) to become next frame's prev,
			// so it must NOT be freed/aliased this frame — skip free scheduling.
			if (info->is_history_current) continue;


			bool alias_ended = false;
		/*	if (delete_resources)
			for (auto pass : info->states.back().passes)
			{
				if (info->used_end.is_in_sync(pass->sync_state_with_self))
				{
					pass->used.resource_deletions_after.insert(info);
					alias_ended = true;
					break;
				}
			}*/

			// if no pass found - find any pass that is synced to the usage
			if (!best_deletion_pass)
			{
				  if(delete_resources)
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

			// ---- transient-aliasing safety --------------------------------------
			// When a freed heap range is recycled, the NEW owner's first writer must
			// be fence-ordered after the OLD owner's last use on EVERY queue. The
			// scheduler only guarantees that for the pass that FREES a range (the
			// used_end.is_in_sync(...) search above); the pass that CREATES into it
			// is picked as "first writer" with no sync check at all, and the heap
			// allocator is queue-agnostic. So a range freed after a DIRECT-queue
			// resource could be handed to a COMPUTE-queue creator with no fence
			// between them — both queues then touch the same bytes, which faults the
			// GPU and removes the device (classically surfacing at the next
			// CreatePlacedResource2, and far more likely while resizing because every
			// transient changes size at once).
			//
			// Provenance is tracked HERE rather than inside the allocator: the Core
			// allocators coalesce and split free blocks freely (which would destroy
			// per-block tags) and are shared with descriptor/query heaps. Keeping the
			// tags FrameGraph-side and testing raw byte OVERLAP is immune to both
			// coalescing and partial reuse, and needs no changes to shared code.
			// `freed_ranges` is per-frame by construction, which is also exactly the
			// right lifetime: the frame-boundary cross-queue wait in
			// commit_command_lists already makes any range last used in an EARLIER
			// frame safe to reuse, so only intra-frame provenance has to be precise.
			//
			// Enforcement never adds a barrier or a fence — it relocates instead. An
			// unsafe range is HELD (not freed) and the allocation retried, so the
			// allocator is forced to hand out different memory: another block,
			// another page, or ultimately a fresh page, which by definition has no
			// previous owner. Held ranges are released once a safe one is found.
			struct FreedRange
			{
				const void* heap  = nullptr;
				uint64      begin = 0;
				uint64      end   = 0;      // inclusive
				SyncState   used_end;
				const char* name  = "";
			};

			struct Range
			{
				const void* heap  = nullptr;
				uint64      begin = 0;
				uint64      end   = 0;      // inclusive
				bool        valid = false;
			};

			std::vector<FreedRange> freed_ranges;
			int alias_hazards = 0;

			// Per-queue call_id watermarks of a SyncState. A missing watermark is 0,
			// which is maximally restrictive — exactly the semantics is_in_sync()
			// applies. (SyncState::min(SyncState) is not usable here: it dereferences
			// null watermarks.)
			constexpr size_t QueueCount = magic_enum::enum_count<CommandListType>();

			auto sync_of = [](const SyncState& s)
			{
				std::array<uint, QueueCount> v{};
				for (size_t q = 0; q < QueueCount; q++)
				{
					auto* p = s.values[(CommandListType)q];
					v[q] = p ? (uint)p->call_id : 0u;
				}
				return v;
			};

			// Everything released before this point belongs to an earlier frame and is
			// unconstrained: the frame-boundary cross-queue wait in commit_command_lists
			// already orders it against anything this frame does.
			++alias_epoch;

			// A SyncState as the allocator's lane watermarks, stamped with this frame.
			auto tag_of = [&](const SyncState& s)
			{
				auto v = sync_of(s);

				AllocSyncTag tag;
				tag.epoch = alias_epoch;
				for (size_t q = 0; q < QueueCount && q < AllocSyncTag::MaxLanes; q++)
					tag.lane[q] = v[q];

				return tag;
			};

			auto range_of = [](ResourceAllocInfo* info) -> Range
			{
				Range r;
				if (!info->alloc_ptr.handle) return r;
				auto heap = info->alloc_ptr.get_heap();
				if (!heap) return r;
				uint64 sz = info->alloc_ptr.get_size();
				if (!sz) return r;

				r.heap  = heap.get();
				r.begin = info->alloc_ptr.get_offset();
				r.end   = r.begin + sz - 1;
				r.valid = true;
				return r;
			};

			auto overlaps = [](const FreedRange& fr, const Range& r)
			{
				return fr.heap == r.heap && r.end >= fr.begin && r.begin <= fr.end;
			};

			auto record_free = [&](ResourceAllocInfo* info)
			{
				if constexpr (!BuildOptions::Dev) return;   // verification-only bookkeeping

				Range r = range_of(info);
				if (!r.valid) return;
				freed_ranges.push_back({ r.heap, r.begin, r.end, info->used_end, info->name() });
			};

			// The freed range this candidate would unsafely recycle, if any.
			auto unsafe_predecessor = [&](const Range& r, Pass* creator) -> const FreedRange*
			{
				if (!r.valid || !creator) return nullptr;

				for (auto& fr : freed_ranges)
					if (overlaps(fr, r) && !SyncState(fr.used_end).is_in_sync(creator->sync_state, false))
						return &fr;

				return nullptr;
			};

			// Take ownership of the bytes: drop them from the free-range list so a
			// later reuse is validated against its IMMEDIATE predecessor only, and
			// keep whatever parts of the old ranges are still unclaimed.
			auto claim = [&](const Range& r)
			{
				if constexpr (!BuildOptions::Dev) return;   // verification-only bookkeeping

				if (!r.valid) return;

				std::vector<FreedRange> remainder;
				remainder.reserve(freed_ranges.size());

				for (auto& fr : freed_ranges)
				{
					if (!overlaps(fr, r))
					{
						remainder.push_back(fr);
						continue;
					}

					if (fr.begin < r.begin) remainder.push_back({ fr.heap, fr.begin,  r.begin - 1, fr.used_end, fr.name });
					if (fr.end   > r.end)   remainder.push_back({ fr.heap, r.end + 1, fr.end,      fr.used_end, fr.name });
				}

				freed_ranges.swap(remainder);
			};

			// Releasing a range stamps it with the point its owner finished, so the
			// allocator can decide for itself who may take it next.
			auto free_now = [&](ResourceAllocInfo* info)
			{
				record_free(info);

				// Hand the allocator the point this resource's last use finished, so it
				// can decide for itself who may take the range next.
				auto& handle = info->alloc_ptr.handle;

				if (auto* owner = handle.get_owner())
					owner->Free(handle, tag_of(info->used_end));
				else
					handle.Free();
			};

			for (auto [id, e] : events)
			{
				for (auto info : e.free_before)
					free_now(info);


				for (auto info : e.create)
				{

					auto creation_info = RenderSystem::get().device().get_alloc_info(info->d3ddesc);
					HeapIndex index = { HAL::MemoryType::COMMITED , info->heap_type };

					Pass* creator = nullptr;
					if (auto it = creation_pass_of.find(info); it != creation_pass_of.end())
						creator = it->second;

					// Who is asking. The allocator skips memory whose previous owner this
					// pass is not ordered after, relocating rather than returning bytes
					// that would need a cross-queue barrier. With no creator we pass a
					// bare epoch tag, which accepts only unconstrained memory.
					AllocSyncTag requester;
					requester.epoch = alias_epoch;
					if (creator) requester = tag_of(creator->sync_state);

					info->alloc_ptr = allocator.alloc(creation_info.size, creation_info.alignment, index, &requester);

					// Verification only — the allocator should make this unreachable.
					Range r = range_of(info);

					if constexpr (BuildOptions::Dev)
					{
						if (const FreedRange* clash = unsafe_predecessor(r, creator))
						{
							++alias_hazards;

							std::string msg = std::string("ALIAS-HAZARD recycled '") + clash->name
							                + "' -> '" + info->name()
							                + "' creator_queue=" + std::to_string((int)creator->get_type())
							                + " creator_call=" + std::to_string((int)creator->call_id);

							for (auto q : magic_enum::enum_values<CommandListType>())
							{
								auto* prev = clash->used_end.values[q];
								auto* sync = creator->sync_state.values[q];
								msg += " | q" + std::to_string((int)q)
								     + " prev_used_end=" + std::to_string(prev ? (int)prev->call_id : 0)
								     + " creator_sync="  + std::to_string(sync ? (int)sync->call_id : 0);
							}

							Log::get() << msg << Log::endl;
						}
					}

					claim(r);
				}


				for (auto info : e.free_after)
					free_now(info);
			}

			if (alias_hazards)
				Log::get() << (std::string("ALIAS-HAZARD survived: ")
				               + std::to_string(alias_hazards)) << Log::endl;

			// Transient heap footprint — the cost of relocating instead of fencing.
			// Logged only when it moves, so a growth trend is visible without spam.
			if constexpr (BuildOptions::Dev)
			{
				uint64 heap_pages = 0;
				uint64 heap_bytes = 0;
				allocator.for_each([&](const HeapIndex&, uint64, uint64, std::shared_ptr<HAL::Heap> heap)
				{
					heap_pages++;
					if (heap) heap_bytes += heap->get_size();
				});

				static uint64 logged_heap_bytes = 0;

				if (heap_bytes != logged_heap_bytes)
				{
					logged_heap_bytes = heap_bytes;

					Log::get() << (std::string("ALIAS heap_pages=") + std::to_string(heap_pages)
					               + " heap_bytes=" + std::to_string(heap_bytes)) << Log::endl;
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

				// History `prev`: adopt the allocation `current` wrote last frame,
				// with a freshly-created view (views are frame-linked). On the first
				// frame or a resize the carried slot is absent/mismatched, so fall
				// back to a fresh cleared resource (is_new -> consumers clear).
				if (info->is_history_prev)
				{
					HistoryLink* link = history_by_prev(info->id);
					// Adopt the carried resource whenever one exists — even at a
					// different size. An SRV doesn't encode resolution (just format +
					// mip/array range), so a view built with this frame's desc over
					// last frame's resource is valid; the shader just samples the old
					// resolution. is_new fires only when there is no history at all
					// (first frame), so consumers clear only then, not on resize.
					if (link && link->carried.valid())
					{
						info->resource   = link->carried.resource;
						info->alloc_ptr  = link->carried.alloc_ptr;
						info->is_new     = false;


					}
					else
					{
						info->resource   = HAL::create_resource(RenderSystem::get().device(), info->d3ddesc, info->heap_type);
						info->resource->frame_graph_managed = true;
						info->resource->set_name(info->name());
						info->alloc_ptr  = HAL::ResourceHandle{};
						info->is_new     = true;
					}

					info->view = nullptr;
					info->handler->init_view(*info, global_frame);
					id++;
					continue;
				}

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

	ByteBufferViewDesc Handlers::ByteBufferDesc::as_view(uint64 offset, ResourceFlags resflags)
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
				tsize = uint3::max(tsize / 2, { 1,1,1 });
				mip_count++;
			}
		}
		return HAL::ResourceDesc::Tex2D(format, size.xy, array_count, mip_count, flags);
	}

	HAL::TextureViewDesc Handlers::TextureDesc::as_view(uint64 offset, ResourceFlags resflags)
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

	HAL::Texture3DViewDesc Handlers::Texture3DDesc::as_view(uint64 offset, ResourceFlags resflags)
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

	HAL::CubeViewDesc Handlers::CubeDesc::as_view(uint64 offset, ResourceFlags resflags)
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

		render_task = std::future<void>();

		// debug_commands is deliberately NOT cleared here.
		//
		// It is filled by the debug snapshot in commit_command_lists, at the END
		// of a frame, but the debugger copies it from graph.on_compile, near the
		// START of one. Clearing here emptied it between those two points, so the
		// debugger only ever copied a vector this frame had not filled yet and
		// the Transitions panel showed nothing.
		//
		// Leaving it means the debugger shows the PREVIOUS frame's transitions,
		// which is what it can actually have. The snapshot assigns over it every
		// frame, so nothing accumulates.

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
		name = s_name;
		id = std::numeric_limits<UINT>::max();
		enabled = true;
		renderable = true;
		flags = PassFlags::General;
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

	void Graph::set_pipeline(Pipelines::PipelineBase* p) { current_pipeline = p; added_pipelines.push_back(p); }
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