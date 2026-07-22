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
			states.push(is_writer, max_passes);

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
			l.carried.last_state = cur.last_state; // best-effort; barrier priming handled later

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

		tex->disable_state_tracking();

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
			info.enabled = true;
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

			for (auto [info, flags] : pass->used.resource_flags)
			{
				// Debug preview fires on the producing (write) pass. A history `prev`
				// is never written by a pass — fire it on a reading pass instead so
				// its thumbnail can be captured from the adopted resource.
				if (!check(flags & WRITEABLE_FLAGS) && !info->is_history_prev) continue;
				info->process_debug_resource(pass, this);
			}

			// Close the pass's open op-batch before deactivating freed resources,
			// so alias_end lands after the last op instead of inside its batch.
			list->close_op();

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
			// frame's lists are submitted. This is what removes the class-1 #1334
			// flood (uploaded assets no longer rest in COMMON / rely on implicit
			// promotion) — validated 2026-07-21.
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

		// Close every pass list's open op-batch before the FrameGraph appends
		// its cross-pass transitions (decay-to-resting, prepare_after_state).
		// Those are recorded at the list's back point; without this they would
		// fall inside the last op's still-open batch and be mis-positioned.
		for (auto& pass : builder.enabled_passes)
			if (pass->context.list)
				pass->context.list->close_op();

		builder.process_transitions();
		builder.process_fences();

		// DISABLED — phase 4 step 2 (multiple command lists per ExecuteCommandLists).
		//
		// link_list_groups() runs AFTER process_transitions(), which has already
		// finalized the persistent GPU layout via set_cpu_state (gpu.layout =
		// last usage's layout). Chaining then retroactively suppresses barriers
		// that computation depended on, so the tracked layout and the real layout
		// drift apart and the next frame's seed mismatches (D3D12 #1334).
		//
		// The fix is not another suppression rule: group membership has to be
		// known BEFORE transitions are generated, so releases are never emitted
		// for intra-group hand-offs in the first place and set_cpu_state sees the
		// truth. Kept compiled but unreferenced until that reordering is done.
		// Requirement 6: chain resource state across command-list boundaries so
		// multiple lists share one ExecuteCommandLists scope. Depends on the
		// UploadManager (class-1 assets rest in a read state) and chain_lists'
		// all-none handling (class-2 FG transients). #1417/#1334 eliminated
		// 2026-07-22 (2 frame-1-transient #1334 on a pre-promote deserialize
		// texture remain).
		builder.link_list_groups();

		builder.compile_lists();



		on_compile(*this);
	}


	HAL::FenceWaiter Graph::commit_command_lists()
	{

		HAL::FenceWaiter result;






		PROFILE(L"submitting lists");

		std::map<CommandListType, std::list<CommandList::ptr>> queued_lists;

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

		for (auto& pass : builder.enabled_passes)
		{
			HAL::CommandListType list_type = pass->get_type();

			auto commandList = pass->context.list;

			if (commandList)
			{


				for (auto sync_pass : pass->sync_state.values)
				{
					if (!sync_pass) continue;

					if(!queued_lists[list_type].empty())  
					{				
						RenderSystem::get().device().get_queue(list_type)->execute(queued_lists[list_type]);
						queued_lists[list_type].clear();
					}

					RenderSystem::get().device().get_queue(list_type)->gpu_wait(sync_pass->fence_end);
				}



				queued_lists[list_type].emplace_back(commandList);

				// Diagnostic: full queue serialization — every pass gets a
				// fence and every OTHER queue waits on it, so no two passes
				// ever run concurrently across queues. If garbage persists
				// with this on, the corruption is NOT an async/lifetime race
				// (look for an out-of-bounds writer instead).
				if (serialize_queues)
					pass->put_fence = true;

				if (pass->put_fence)		//////////////////////// ARGH!!!!
				{
					pass->fence_end = RenderSystem::get().device().get_queue(list_type)->execute(queued_lists[list_type]);

					queued_lists[list_type].clear();

					result = pass->fence_end;
					frame_end_fence[list_type] = pass->fence_end;

					if (serialize_queues)
						for (auto other : magic_enum::enum_values<CommandListType>())
						{
							if (other == list_type) continue;
							RenderSystem::get().device().get_queue(other)->gpu_wait(pass->fence_end);
						}
				}
				//	pass->fence_end = commandList->execute();

				//	
			}


		}


		for (auto& [type, lists] : queued_lists)
		{
			if (lists.empty()) continue;
			result = RenderSystem::get().device().get_queue(type)->execute(lists);
			frame_end_fence[type] = result;

			if (serialize_queues)
				for (auto other : magic_enum::enum_values<CommandListType>())
				{
					if (other == type) continue;
					RenderSystem::get().device().get_queue(other)->gpu_wait(result);
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

	void TaskBuilder::link_list_groups()
	{
		PROFILE(L"link_list_groups");

		// Lists that will be submitted together in one ExecuteCommandLists, per
		// queue. Mirrors commit_command_lists: a pass that must gpu_wait flushes
		// the pending batch before it, and a pass with put_fence closes one after.
		enum_array<HAL::CommandListType, std::vector<HAL::CommandList*>> pending;

		auto close_group = [&](HAL::CommandListType type)
		{
			auto& lists = pending[type];

			// Within a group, chain each resource from the list that used it last
			// to the one using it next. That replaces the SYNC_NONE release +
			// SYNC_NONE re-seed pair at the boundary with a direct state hand-off,
			// leaving exactly one SYNC_NONE entry (first user) and one SYNC_NONE
			// exit (last user) per resource per group.
			if (lists.size() > 1)
			{
				std::map<HAL::Resource*, HAL::CommandList*> last_user;

				for (auto* cmd : lists)
					for (auto* res : cmd->get_used_resources())
					{
						auto it = last_user.find(res);
						if (it != last_user.end() && it->second != cmd)
							res->get_state_manager().chain_lists(it->second, cmd);

						last_user[res] = cmd;
					}
			}

			lists.clear();
		};

		for (auto& pass : enabled_passes)
		{
			auto commandList = pass->context.list;
			if (!commandList) continue;

			HAL::CommandListType list_type = pass->get_type();

			bool waits = false;
			for (auto sync_pass : pass->sync_state.values)
				if (sync_pass) waits = true;

			if (waits) close_group(list_type);      // gpu_wait flushes the batch

			pending[list_type].push_back(commandList.get());

			if (pass->put_fence) close_group(list_type);   // fence closes the batch
		}

		for (auto type : magic_enum::enum_values<HAL::CommandListType>())
			close_group(type);
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
							detail.before = b.before;
							detail.after = b.after;
							detail.subres = b.subres;
							detail.flags = b.flags;
							detail.resource_id = reinterpret_cast<uint64>(b.resource);   // opaque instance id
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
				info.states.remove_if(state_checker);

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
					  }
					  else
	ASSERT(false);



					}


					// (Last-touch decay to the resting state now happens once,
					// after the states loop — see below — so it also covers a
					// read-last touch, not just write-last.)


				}






				// Canonical resting-state contract for passed/static resources:
				// at the LAST pass that touches the resource (read OR write), decay
				// it back to creation_state.layout — its per-resource CanonicalRead
				// (SHADER_RESOURCE for read resources, PRESENT for the swapchain).
				// Within the graph they're FG-scheduled like transients; this is the
				// single point where they return to a state any queue/list can pick
				// up. A no-op when the last touch already left it there (merge_state
				// collapses it). Must run before the end-state capture below so
				// last_state carries the resting layout into next frame.
				if (!info.states.empty() && (info.is_static() || info.passed))
				{
					Pass* last_pass = info.states.back().passes.back();
					auto commandList = last_pass->context.list;
					if (commandList)
					{
						auto target = ResourceStates::NO_ACCESS;
						target.layout = info.creation_state.get_subres_state(0).layout;
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

			for (auto info : pass->used.resource_deletions_before)
			{

				if (info->states.empty())continue;
				auto& last_state = info->states.back();
				if (last_state.write)
				{
					auto prev_pass = last_state.passes.front();
					auto prev_cmd = prev_pass->context.list;
					auto& prev_cpu_state = info->resource->get_state_manager().get_cpu_state(prev_cmd.get());


					HAL::SubResourcesGPU prev_gpu_state;
					prev_gpu_state.subres.resize(info->resource->get_state_manager().get_subres_count());
					prev_gpu_state.set_cpu_state(prev_cpu_state);

					info->resource->get_state_manager().prepare_state(commandList.get(), prev_gpu_state);
				}
				else
				{
					info->resource->get_state_manager().prepare_state(commandList.get(), last_state.merged_read_state);

				}
				//	



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
						info->last_state = link->carried.last_state;
						info->is_new     = false;


					}
					else
					{
						info->resource   = HAL::create_resource(RenderSystem::get().device(), info->d3ddesc, info->heap_type);
						info->resource->frame_graph_managed = true;
						info->creation_state = info->last_state = info->resource->get_state_manager().copy_gpu();
						info->last_state = TextureLayout::UNDEFINED;
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
						info->creation_state = info->last_state = info->resource->get_state_manager().copy_gpu();
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