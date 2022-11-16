module HAL:CommandList;

import :GPUTimer;

import :Autogen;

import :Buffer;

import Core;

import HAL;

//using namespace HAL;
namespace HAL

{

	GPUTimer::GPUTimer()
	{
		id = HAL::Device::get().get_time_manager().get_id();
	}

	GPUTimer::~GPUTimer()
	{
		HAL::Device::get().get_time_manager().put_id(id);
	}

	void GPUTimer::start(Eventer* list)
	{
		HAL::Device::get().get_time_manager().start(*this, list);
	}

	void GPUTimer::end(Eventer* list)
	{
		HAL::Device::get().get_time_manager().end(*this, list);
		list = nullptr;
	}

	float GPUTimer::get_time()
	{
		return HAL::Device::get().get_time_manager().get_time(*this);
	}

	double GPUTimer::get_start()
	{
		return HAL::Device::get().get_time_manager().get_start(*this);
	}

	double GPUTimer::get_end()
	{
		return HAL::Device::get().get_time_manager().get_end(*this);
	}

	CommandList::CommandList(CommandListType type):GPUCompiledManager<Thread::Free>(Device::get()), HAL::Readbacker<Thread::Free>(Device::get())

	{
		this->type = type;

		compiler.create(type);

		if (type == CommandListType::DIRECT || type == CommandListType::COMPUTE)
			compute.reset(new ComputeContext(*this));

		//	if (type == CommandListType::DIRECT || type == CommandListType::COPY)
		copy.reset(new CopyContext(*this));

		if (type == CommandListType::DIRECT)
			graphics.reset(new GraphicsContext(*this));

		compiler.SetName(L"SpectrumCommandList");


		//debug_buffer = std::make_shared<HAL::StructureBuffer<Table::DebugStruct>>(64, HAL::counterType::NONE, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);
	}

	void CommandList::setup_debug(SignatureDataSetter* setter)
	{
		//if (!current_pipeline->debuggable) return;
		//Slots::DebugInfo info;
		//HAL::StructureBuffer<Table::DebugStruct>* structured = static_cast<HAL::StructureBuffer<Table::DebugStruct>*>(debug_buffer.get());
		//info.GetDebug() = structured->rwStructuredBuffer;
		//info.set(*setter);

	}

	void CommandList::print_debug()
	{
		/*if (!current_pipeline->debuggable) return;

		auto pso_name = current_pipeline->name;
		get_copy().read_buffer(debug_buffer.get(), 0, 3 * sizeof(Table::DebugStruct), [this, pso_name](const char* data, UINT64 size)
			{

				LogBlock block(Log::get(), log_level_internal::level_all);

				if (first_debug_log)
				{
					block << "-----------------------------------------\n";

					first_debug_log = false;
				}
				auto result = reinterpret_cast<const Table::DebugStruct*>(data);

				block << "DEBUG(" << name << "): " << pso_name << "\n";
				for (int i = 0; i < 3; i++)
				{
					block << "debug(" << i << "): " << result[i].v.x << " " << result[i].v.y << " " << result[i].v.z << " " << result[i].v.w << " " << "\n";
				}
				Log::get() << block;
			});

		StructureBuffer<Table::DebugStruct>* structured = static_cast<StructureBuffer<Table::DebugStruct>*>(debug_buffer.get());
		clear_uav(structured->rwByteAddressBuffer);*/
	}


	void CommandList::begin(std::string name, Timer* t)
	{
		if (name.empty())
		{
			compiler.SetName(L"EmptyName");
		}
		else
			compiler.SetName(convert(name).c_str());
		compiled = HAL::Private::CommandListCompiled();
#ifdef DEV
		begin_stack = Exceptions::get_stack_trace();
#endif

		HAL::Device::get().context_generator.generate(this);
		first_debug_log = true;


		//       Log::get() << "begin" << Log::endl;
		compiler.reset();
		//resource_index = 0;

		first_pipeline = nullptr;

		if (graphics) graphics->begin();
		if (compute) compute->begin();


		Transitions::begin();
		Eventer::begin(name, t);

		if (type != CommandListType::COPY) {
			compiler.set_descriptor_heaps(HAL::Device::get().get_descriptor_heap_manager().get_gpu_heap(DescriptorHeapType::CBV_SRV_UAV).get(), HAL::Device::get().get_descriptor_heap_manager().get_gpu_heap(DescriptorHeapType::SAMPLER).get());
		}
	}


	void CommandList::end()
	{
		//	create_transition_point(false);
			//	id = -1;
		Transitions::make_split_barriers();
		current_pipeline = nullptr;

		if (graphics) graphics->end();
		if (compute) compute->end();

		Eventer::reset();

	}

	void GraphicsContext::begin()
	{
		reset();
		reset_tables();
		index = HAL::Views::IndexBuffer();
	}
	void GraphicsContext::end()
	{
		//current_root_signature = nullptr;
	}

	void ComputeContext::begin()
	{
		reset();
		reset_tables();
	}
	void ComputeContext::end()
	{
		current_compute_root_signature = nullptr;
	}

	void GraphicsContext::on_execute()
	{

	}

	void ComputeContext::on_execute()
	{

	}

	void Sendable::on_done(std::function<void()> f)
	{
		on_execute_funcs.emplace_back(f);

	}
	void Sendable::compile()
	{
		compiled = compiler.compile();
	}
	std::shared_future<FenceWaiter> Sendable::execute(std::function<void()> f)
	{

		//TEST(compiler.Close());
		if (!compiled)
			compile();

		execute_fence = std::promise<FenceWaiter>();
		execute_fence_result = execute_fence.get_future();
		if (f)
			on_execute_funcs.emplace_back(f);

		Device::get().get_queue(type)->execute(static_cast<CommandList*>(this));

		return execute_fence_result;
	}

	void Sendable::execute_and_wait(std::function<void()> f)
	{
		execute(f).get().wait();
	}


	void Eventer::insert_time(QueryHeap& pQueryHeap, uint32_t QueryIdx)
	{
		compiler.insert_time(pQueryHeap, QueryIdx);
	}

	void Eventer::resolve_times(QueryHeap& pQueryHeap, uint32_t NumQueries, std::function<void(std::span<UINT64>)> f)
	{
		CommandList* list = static_cast<CommandList*>(this); // :(
		auto info = list->read_data(NumQueries * sizeof(UINT64));

		compiler.resolve_times(pQueryHeap, NumQueries, info);
		on_execute_funcs.emplace_back([info, f, NumQueries]() {

			UINT64* data = reinterpret_cast<UINT64*>(info.get_cpu_data());
			f(std::span(data, NumQueries));

			});
	}

	void GraphicsContext::on_set_signature(const RootSignature::ptr& s)
	{
		list->set_graphics_signature(s);
	}

	void GraphicsContext::set_heaps(DescriptorHeap::ptr& a, DescriptorHeap::ptr& b)
	{
		list->set_descriptor_heaps(a.get(), b.get());
	}


	void  GraphicsContext::set(UINT i, const HandleTableLight& table)
	{
		assert(false);//list->SetGraphicsRootDescriptorTable(i, table.get_gpu());
	}


	void  GraphicsContext::set_const_buffer(UINT i, ResourceAddress address)
	{
		list->graphics_set_const_buffer(i, address);
	}

	void GraphicsContext::set_rtv(const HandleTable& table, Handle h)
	{
		set_rtv(table.get_count(), table[0], h);
	}
	void GraphicsContext::set_rtv(const HandleTableLight& table, Handle h)
	{
		set_rtv(table.get_count(), table, h);
	}

	void GraphicsContext::set_rtv(int c, Handle rt, Handle h)
	{
		base.create_transition_point();
		for (int i = 0; i < c; i++)
		{
			get_base().transition(&rt.get_resource_info()[i]);
		}

		if (h.is_valid())
			get_base().transition(h.get_resource_info());
		list->set_rtv(c,rt,h);

		base.create_transition_point(false);
	}

	void GraphicsContext::set_rtv(std::initializer_list<Handle> rt, Handle h)
	{
		set_rtv(place_rtv(rt), h);
	}

	void GraphicsContext::draw(UINT vertex_count, UINT vertex_offset, UINT instance_count, UINT instance_offset)
	{
		PROFILE_GPU(L"draw");
		base.create_transition_point();
		base.setup_debug(this);

		commit_tables();
		list->draw(vertex_count, vertex_offset, instance_count, instance_offset);
		base.create_transition_point(false);

		get_base().print_debug();

	}
	void GraphicsContext::draw_indexed(UINT index_count, UINT index_offset, UINT vertex_offset, UINT instance_count, UINT instance_offset)
	{
		if (instance_count == 0) return;
		PROFILE_GPU(L"draw_indexed");
		base.create_transition_point();
		base.setup_debug(this);

		commit_tables();
		get_base().transition(index.Resource, ResourceState::INDEX_BUFFER);
		list->set_index_buffer(index);
		list->draw_indexed(index_count, index_offset, vertex_offset, instance_count, instance_offset);
		base.create_transition_point(false);
		get_base().print_debug();
	}
	void GraphicsContext::dispatch_mesh(D3D12_DISPATCH_MESH_ARGUMENTS args)
	{
		dispatch_mesh(ivec3{ args.ThreadGroupCountX, args.ThreadGroupCountY, args.ThreadGroupCountZ });
	}

	void GraphicsContext::dispatch_mesh(ivec3 v)
	{

		PROFILE_GPU(L"dispatch_mesh");
		base.create_transition_point();
		base.setup_debug(this);
		commit_tables();
		list->dispatch_mesh(v);
		base.create_transition_point(false);
		get_base().print_debug();
	}


	void GraphicsContext::set_scissors(sizer_long rect)
	{
		list->set_scissors(rect);
	}

	void GraphicsContext::set_viewports(std::vector<Viewport> viewports)
	{
		this->viewports = viewports;
		list->set_viewports(viewports);
	}

	void GraphicsContext::set_scissor(sizer_long rect)
	{
		list->set_scissors(rect);
	}

	void GraphicsContext::set_viewport(Viewport v)
	{
		this->viewports.resize(1);
		this->viewports[0] = v;

		set_viewports(viewports);
	}

	void GraphicsContext::set_viewport(vec4 v)
	{
		this->viewports.resize(1);
		this->viewports[0].pos.x = v.x;
		this->viewports[0].pos.y = v.y;
		this->viewports[0].size.x = v.z;
		this->viewports[0].size.y = v.w;
		this->viewports[0].depths = { 0, 1 };
		set_viewports(viewports);
	}

	void  CopyContext::update_buffer(Resource::ptr resource, UINT offset, const  char* data, UINT size)
	{
		update_buffer(resource.get(), offset, data, size);
	}
	void  CopyContext::update_buffer(Resource* resource, UINT offset, const  char* data, UINT size)
	{
		base.create_transition_point();
		//	if (base.type != CommandListType::COPY)
		base.transition(resource, ResourceState::COPY_DEST);

		auto info = base.place_data(size);
		memcpy(info.get_cpu_data(), data, size);
		list->copy_buffer(resource, offset, info.resource, info.resource_offset, size);
		base.create_transition_point(false);
	}


	
	void CopyContext::update_texture(Resource::ptr resource, ivec3 offset, ivec3 box, UINT sub_resource, const char* data, UINT row_stride, UINT slice_stride)
	{
		update_texture(resource.get(), offset, box, sub_resource, data, row_stride, slice_stride);
	}

	void CopyContext::update_texture(Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, const char* data, UINT row_stride, UINT slice_stride)
	{
		base.create_transition_point();

		base.transition(resource, ResourceState::COPY_DEST);
		auto layout = list->get_texture_layout(resource, sub_resource, box);
		auto info = base.place_data(layout.size, layout.alignment);

		if (layout.row_stride == row_stride)
		{
			if (slice_stride == 0 || slice_stride == row_stride * layout.rows_count)
			{
				auto pDestSlice = info.get_cpu_data();
				const std::byte* pSrcSlice = reinterpret_cast<const std::byte*>(data);
				memcpy(pDestSlice,
					pSrcSlice,
					layout.size);
			}

			else
			{
				for (UINT z = 0; z < static_cast<UINT>(box.z); ++z)
				{
					auto pDestSlice = info.get_cpu_data() + layout.slice_stride * z;
					const std::byte* pSrcSlice = reinterpret_cast<const std::byte*>(data) + slice_stride * z;
					memcpy(pDestSlice,
						pSrcSlice,
						slice_stride);
				}
			}
		}

		else
			for (UINT z = 0; z < static_cast<UINT>(box.z); ++z)
			{
				std::byte* pDestSlice = info.get_cpu_data() + layout.slice_stride * z;
				const std::byte* pSrcSlice = reinterpret_cast<const std::byte*>(data) + slice_stride * z;

				for (UINT y = 0; y < layout.rows_count; ++y)
				{
					memcpy(pDestSlice + layout.row_stride * y,
						pSrcSlice + row_stride * y,
						row_stride);
				}
			}



		list->update_texture(resource, offset, box, sub_resource,  info, layout);
		if constexpr (Debug::CheckErrors)	TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->GetDeviceRemovedReason());
		base.create_transition_point(false);
	}
	/*
	void  GraphicsContext::set_pipeline(PipelineState::ptr state)
	{
		set_signature(state->desc.root_signature);
		base.set_pipeline_internal(state.get());
	}
	*/
	//void SignatureDataSetter::set_layout(Layouts layout)
	//{
	//	set_signature(HAL::get_Signature(layout));
	//}
	std::future<bool> CopyContext::read_texture(Resource::ptr resource, ivec3 offset, ivec3 box, UINT sub_resource, std::function<void(const char*, UINT64, UINT64, UINT64)> f)
	{
		return read_texture(resource.get(), offset, box, sub_resource, f);
	}
	std::future<bool> CopyContext::read_texture(const Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, std::function<void(const char*, UINT64, UINT64, UINT64)> f)
	{

		base.create_transition_point();
		//	if (base.type != CommandListType::COPY)
		base.transition(resource, ResourceState::COPY_SOURCE);
		//else
	//		base.transition(resource, ResourceState::COMMON);

		auto layout = list->get_texture_layout(resource, sub_resource, box);
		auto info = base.read_data(layout.size, layout.alignment);
		list->read_texture(resource, offset, box, sub_resource, info, layout);

		if constexpr (Debug::CheckErrors)	TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->GetDeviceRemovedReason());
		auto result = std::make_shared<std::promise<bool>>();
		base.on_execute_funcs.push_back([result, info, f, layout]()
			{
				f(reinterpret_cast<char*>(info.get_cpu_data()), layout.row_stride, layout.slice_stride, info.resource->get_size());
				result->set_value(true);
			});

		base.create_transition_point(false);
		return result->get_future();
	}

	std::future<bool> CopyContext::read_buffer(Resource* resource, unsigned int offset, UINT64 size, std::function<void(const char*, UINT64)> f)
	{

		auto result = std::make_shared<std::promise<bool>>();

		if (size == 0)
		{
			f(nullptr, 0);
			result->set_value(true);
			return result->get_future();
		}

		base.create_transition_point();

		base.transition(resource, ResourceState::COPY_SOURCE);

		auto info = base.read_data(size);
		list->copy_buffer(info.resource, info.resource_offset, resource, offset, size);
		if constexpr (Debug::CheckErrors)	TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->GetDeviceRemovedReason());
		base.on_execute_funcs.push_back([result, info, f, size]()
			{
				f(reinterpret_cast<char*>(info.get_cpu_data()), size);
				result->set_value(true);
			});

		base.create_transition_point(false);
		return result->get_future();
	}

	std::future<bool> CopyContext::read_query(std::shared_ptr<QueryHeap>& query_heap, unsigned int offset, unsigned int size, std::function<void(const char*, UINT64)> f)
	{
		if (size == 0)
		{
			auto result = std::make_shared<std::promise<bool>>();
			f(nullptr, 0);
			result->set_value(true);
			return result->get_future();
		}

		//  auto size = resource->get_size();
		auto info = base.read_data(size);
		//  compiler.CopyResource(info.resource->get_resource()->get_native().Get(), resource->get_native().Get());
		//list->ResolveQueryData(query_heap->get_native().Get(), D3D12_QUERY_TYPE_PIPELINE_STATISTICS, 0, 1, info.resource->get_dx(), info.resource_offset);
		assert(false);
		if constexpr (Debug::CheckErrors)	TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->GetDeviceRemovedReason());
		auto result = std::make_shared<std::promise<bool>>();
		base.on_execute_funcs.push_back([result, info, f, size]()
			{
				f(reinterpret_cast<char*>(info.get_cpu_data()), size);
				result->set_value(true);
			});
		return result->get_future();
	}

	void CommandList::on_execute()
	{

		// Log::get() << "on_execute" << Log::endl;
		for (auto&& t : on_execute_funcs)
			t();

		//	srv.reset();
		//	smp.reset();

		if (graphics) graphics->on_execute();
		if (compute) compute->on_execute();



		on_execute_funcs.clear();
		Uploader::reset();
		Readbacker::reset();

		GPUCompiledManager::reset();
		Transitions::on_execute();
		frame_resources = nullptr;

		free_tracked_objects();

		HAL::Device::get().context_generator.free(this);

	}
	//void Transitions::create_transition_point(bool end)
	//{
	//	auto prev_point = transition_points.empty() ? nullptr : &transition_points.back();
	//	auto point = &transition_points.emplace_back(type);

	//	if (prev_point) prev_point->next_point = point;
	//	point->prev_point = prev_point;

	//	point->start = !end;

	//	if (end)
	//	{
	//		assert(point->prev_point->start);
	//	}
	//	compiler.func([point, this](API::Private::CommandListTranslator<API::Private::CommandListCompiler>& list)
	//		{
	//			HAL::Barriers  transitions(type);

	//			for (auto uav : point->uav_transitions)
	//			{
	//				transitions.uav(uav);
	//			}

	//			for (auto& uav : point->aliasing)
	//			{
	//				transitions.alias(nullptr, uav);
	//			}

	//			for (auto& transition : point->transitions)
	//			{
	//				auto prev_transition = transition.prev_transition;

	//				if (!prev_transition) continue;

	//				if (prev_transition->wanted_state == transition.wanted_state) continue;

	//				//					assert(!point->start);
	//				transitions.transition(transition.resource,
	//					prev_transition->wanted_state,
	//					transition.wanted_state,
	//					transition.subres, transition.flags);
	//			}

	//			auto& native_transitions = transitions.get_native();
	//			if (!native_transitions.empty())
	//			{

	//				assert(false);
	//				//list->ResourceBarrier((UINT)native_transitions.size(), native_transitions.data());
	//			}

	///*			{

	//				auto& native_transitions = point->compiled_transitions.get_native();
	//				if (!native_transitions.empty())
	//				{
	//					list->ResourceBarrier((UINT)native_transitions.size(), native_transitions.data());
	//				}
	//			}*/

	//		});
	//}


	void Transitions::make_split_barriers()
	{
		return;
		//for (auto& point : transition_points)
		//{
		//	for (auto& transition : point.transitions)
		//	{
		//		auto prev_transition = transition.prev_transition;

		//		if (!prev_transition) continue;

		//		if (prev_transition->wanted_state == transition.wanted_state) continue;

		//		auto prev_point = prev_transition->point->next_point;

		//		assert(prev_point->start);
		//		prev_point->compiled_transitions.transition(transition.resource,
		//			prev_transition->wanted_state,
		//			transition.wanted_state,
		//			transition.subres, HAL::BarrierFlags::BEGIN);

		//		transition.flags = HAL::BarrierFlags::END;
		//		/*
		//		transitions.transition(transition.resource,
		//			prev_transition->wanted_state,
		//			transition.wanted_state,
		//			transition.subres);*/
		//	}

		//}
	}

	std::shared_ptr<TransitionCommandList> Transitions::fix_pretransitions()
	{
		PROFILE(L"fix_pretransitions");

		HAL::Barriers result(CommandListType::DIRECT);
		std::vector<HAL::Resource*> discards;


		ResourceState states = ResourceState::COMMON;
		for (auto& r : used_resources)
		{
			states = states | r->get_state_manager().process_transitions(result, discards, this);
		}

		auto transition_type = GetBestType(states, type);

		if (result)
		{
			transition_list = (HAL::Device::get().get_queue(transition_type)->get_transition_list());
			transition_list->create_transition_list(result, discards);
			return transition_list;
		}
		return nullptr;
	}
	void Transitions::merge_transition(Transitions* to, Resource* resource)
	{


		if (resource->get_state_manager().transition(this, to))
		{
			track_object(*resource);
			use_resource(resource);
		}
	}
	void Transitions::prepare_transitions(Transitions* to, bool all)
	{
		for (auto& resource : to->used_resources)
		{
			bool is_new = !resource->get_state_manager().is_used(this);
			if ((all && is_new) || (!all && !is_new))
				if (resource->get_state_manager().transition(this, to))
				{
					track_object(*resource);
					use_resource(resource);
				}
		}
	}


	void Transitions::transition(const Resource::ptr& resource, ResourceState to, UINT subres)
	{
		transition(resource.get(), to, subres);
	}

	void Transitions::use_resource(const Resource* resource)
	{

		auto& state = const_cast<Resource*>(resource)->get_state_manager().get_cpu_state(this);
		if (!state.used)
		{
			state.used = true;
			used_resources.emplace_back(const_cast<Resource*>(resource));
		}
	}

	void Transitions::free_resources()
	{
#ifdef DEV
		CommandList* list = static_cast<CommandList*>(this); // :(

		for (auto r : used_resources)
		{
			r->not_used(list);
		}
#endif
	}

	void Transitions::transition(const Resource* resource, ResourceState to, UINT subres)
	{
		if (!resource) return;

		if (type == CommandListType::COPY && (to == ResourceState::COPY_DEST || to == ResourceState::COPY_SOURCE))
			to = ResourceState::COMMON;


		track_object(*const_cast<Resource*>(resource));

		CommandList* list = static_cast<CommandList*>(this); // :(

		if (resource->get_heap_type() == HeapType::DEFAULT || resource->get_heap_type() == HeapType::RESERVED)
		{
			use_resource(resource);
			const_cast<Resource*>(resource)->get_state_manager().transition(this, to, subres);
		}
	}

	void Transitions::transition(Resource* from, Resource* to)
	{

		track_object(*to);
		create_aliasing_transition(to);
		/*	transitions.emplace_back(CD3DX12_RESOURCE_BARRIER::Aliasing(from ? from->get_native().Get() : nullptr, to->get_native().Get()));

			CommandList* list = static_cast<CommandList*>(this); // :(


			if (to->is_new(id, global_id))
			{

				to->aliasing(id, global_id);

				bool good = std::find(used_resources.begin(), used_resources.end(), to) == used_resources.end();


				assert(good);

				used_resources.emplace_back(const_cast<Resource*>(to));
	#ifdef DEV
				const_cast<Resource*>(to)->used(list);
	#endif
				tracked_resources.emplace_back(const_cast<Resource*>(to)->tracked_info);
				assert(!to->is_new(id, global_id));
			}
			else
			{
				auto state = to->get_cpu_state(id, global_id);

				if (check(state & ResourceState::RENDER_TARGET) || check(state & ResourceState::DEPTH_WRITE))
				{
			//		flush_transitions();
				//	get_native_list()->DiscardResource(to->get_native().Get(), nullptr);
				}
			}

			//	get_native_list()->DiscardResource(to->get_native().Get(),nullptr);
			//	if(to->gpu_state.subres[0].state)
			//	if (transition_count == transitions.size())
			//		flush_transitions();*/
	}

	void Transitions::transition_uav(Resource* resource)
	{
		track_object(*resource);
		create_uav_transition(resource);
	}

	void CopyContext::copy_buffer(Resource* dest, int s_dest, Resource* source, int s_source, int size)
	{
		base.create_transition_point();
		//if (base.type != CommandListType::COPY)
		{
			base.transition(source, ResourceState::COPY_SOURCE);
			base.transition(dest, ResourceState::COPY_DEST);
		}

		list->copy_buffer(dest, s_dest, source, s_source, size);
		base.create_transition_point(false);
	}
	void CopyContext::copy_resource(Resource* dest, Resource* source)
	{
		base.create_transition_point();
		//	if (base.type != CommandListType::COPY)
		{
			base.transition(source, ResourceState::COPY_SOURCE);
			base.transition(dest, ResourceState::COPY_DEST);
		}
		list->copy_resource(dest, source);
		base.create_transition_point(false);
	}
	void CopyContext::copy_resource(const Resource::ptr& dest, const Resource::ptr& source)
	{
		copy_resource(dest.get(),source.get());
	}
	void CopyContext::copy_texture(const Resource::ptr& dest, int dest_subres, const Resource::ptr& source, int source_subres)
	{
		base.create_transition_point();
		//if (base.type != CommandListType::COPY) 
		{
			base.transition(source, ResourceState::COPY_SOURCE, source_subres);
			base.transition(dest, ResourceState::COPY_DEST, dest_subres);
		}

		list->copy_texture(dest, dest_subres, source, source_subres);
		if constexpr (Debug::CheckErrors)	TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->GetDeviceRemovedReason());
		base.create_transition_point(false);
	}

	void CopyContext::copy_texture(const Resource::ptr& to, ivec3 to_pos, const Resource::ptr& from, ivec3 from_pos, ivec3 size)
	{
		base.create_transition_point();
		//if (base.type != CommandListType::COPY) 
		{
			base.transition(from, ResourceState::COPY_SOURCE);
			base.transition(to, ResourceState::COPY_DEST);
		}
		list->copy_texture(to, to_pos, from, from_pos, size);
		if constexpr (Debug::CheckErrors)	TEST(Device::get(), Device::get().get_native_device()->GetDeviceRemovedReason());
		base.create_transition_point(false);
	}



	GraphicsContext& CommandList::get_graphics()
	{
		return *graphics.get();// reinterpret_cast<GraphicsContext&>(*this);
	}

	ComputeContext& CommandList::get_compute()
	{
		return *compute.get();//return reinterpret_cast<ComputeContext&>(*this);
	}

	CopyContext& CommandList::get_copy()
	{
		return *copy.get();//return reinterpret_cast<ComputeContext&>(*this);
	}


	void ComputeContext::on_set_signature(const RootSignature::ptr& s)
	{
		list->set_compute_signature(s);
	}

	void ComputeContext::dispach(int x, int y, int z)
	{
		PROFILE_GPU(L"Dispatch");
		base.create_transition_point();
		base.setup_debug(this);
		commit_tables();

		list->dispatch({ x, y, z });
		base.create_transition_point(false);
		get_base().print_debug();

	}


	void ComputeContext::dispach(ivec2 a, ivec2 b)
	{
		dispach(ivec3(a, 1), ivec3(b, 1));
	}

	void ComputeContext::dispach(ivec3 a, ivec3 b)
	{

		ivec3 res;
		res.x = Math::DivideByMultiple(a.x, b.x);
		res.y = Math::DivideByMultiple(a.y, b.y);
		res.z = Math::DivideByMultiple(a.z, b.z);
		dispach(res.x, res.y, res.z);
	}


	void  ComputeContext::set(UINT i, const HandleTableLight& table)
	{
		assert(false);//list->SetComputeRootDescriptorTable(i, table.get_gpu());
	}

	void CommandList::set_pipeline_internal(PipelineStateBase* pipeline)
	{
		if (current_pipeline != pipeline)
		{
			if (pipeline)
			{
				compiler.set_pipeline(pipeline);
				track_object(*pipeline);

				if (!first_pipeline) first_pipeline = pipeline;
			}
			current_pipeline = pipeline;
		}
	}
	void Eventer::on_start(Timer* timer)
	{
		names.push_back(timer->get_block().get_name());
		start_event(names.back().c_str());

		for (auto& c : timer->get_block().get_childs())
		{
			GPUBlock* b = dynamic_cast<GPUBlock*>(c.get());
			if (b)
			{
				//	b->gpu_counter.enabled = false;
			}
		}
		//		static_cast<GPUBlock*>(c.get())

		GPUBlock* b = dynamic_cast<GPUBlock*>(&timer->get_block());
		if (b)
		{
			b->gpu_counter.timer.start(this);
		}


		//	static_cast<GPUBlock&>(timer->get_block()).gpu_counter.timer.start(this);
		//	timer->block.begin_timings(executed ? nullptr : this);
		current = &timer->get_block();
	}
	void  Eventer::on_end(Timer* timer)
	{
		//timer->get_block().end_timings(executed ? nullptr : this);
	//	static_cast<GPUBlock&>(timer->get_block()).gpu_counter.timer.end(this);

		GPUBlock* b = dynamic_cast<GPUBlock*>(&timer->get_block());
		if (b)
		{
			b->gpu_counter.timer.end(this);
		}

		current = timer->get_block().get_parent().get();
		end_event();
	}
	thread_local Eventer* Eventer::thread_current = nullptr;
	void Eventer::reset()
	{
		thread_current = nullptr;
		timer.reset();
	}

	void Eventer::begin(std::string name, Timer* t)
	{
		this->name = name;
		thread_current = this;

		current = t ? &t->get_block() : &Profiler::get();
		TimedRoot::parent = t ? t->get_root() : &Profiler::get();
		if (type != CommandListType::COPY && name.size())
		{
			timer.reset(new Timer(start(convert(name).c_str())));
		}

		else
			current = nullptr;


		names.clear();
	}

	Timer  Eventer::start(std::wstring_view name)
	{
		if (Profiler::get().enabled)
			return Timer(&current->get_child<GPUBlock>(name), this);
		else return Timer(nullptr, nullptr);
		//return std::move(Timer(nullptr,nullptr));

	}


	void Eventer::start_event(std::wstring str)
	{
		compiler.start_event(str);
	}

	void Eventer::end_event()
	{
		compiler.end_event();
	}

	void Eventer::set_marker(const wchar_t* label)
	{

		//::PIXSetMarker(m_commandList.Get(), 0, label);
	}



	void GraphicsContext::execute_indirect(IndirectCommand& command_types, UINT max_commands, Resource* command_buffer, UINT64 command_offset, Resource* counter_buffer, UINT64 counter_offset)
	{
		PROFILE_GPU(L"execute_indirect");
		base.create_transition_point();

		base.setup_debug(this);

		if (command_buffer) get_base().transition(command_buffer, ResourceState::INDIRECT_ARGUMENT);
		if (counter_buffer) get_base().transition(counter_buffer, ResourceState::INDIRECT_ARGUMENT);

		get_base().transition(static_cast<HAL::Resource*>(index.Resource), ResourceState::INDEX_BUFFER);

		list->set_index_buffer(index);
		commit_tables();

		list->execute_indirect(
			command_types,
			max_commands,
			command_buffer,
			command_offset,
			counter_buffer,
			counter_offset);
		base.create_transition_point(false);
		get_base().print_debug();
	}
	void ComputeContext::execute_indirect(IndirectCommand& command_types, UINT max_commands, Resource* command_buffer, UINT64 command_offset, Resource* counter_buffer, UINT64 counter_offset)
	{
		PROFILE_GPU(L"execute_indirect");
		base.create_transition_point();
		base.setup_debug(this);

		assert(command_buffer);

		if (command_buffer) get_base().transition(command_buffer, ResourceState::INDIRECT_ARGUMENT);
		if (counter_buffer) get_base().transition(counter_buffer, ResourceState::INDIRECT_ARGUMENT);

		commit_tables();

		list->execute_indirect(
			command_types,
			max_commands,
			command_buffer,
			command_offset,
			counter_buffer,
			counter_offset);
		base.create_transition_point(false);
		get_base().print_debug();
	}

	void ComputeContext::build_ras(const HAL::RaytracingBuildDescStructure& build_desc, const HAL::RaytracingBuildDescBottomInputs& bottom)
	{
		base.create_transition_point();

		for (auto g : bottom.geometry)
		{
			base.transition(g.IndexBuffer.resource, ResourceState::NON_PIXEL_SHADER_RESOURCE);
			base.transition(g.VertexBuffer.resource, ResourceState::NON_PIXEL_SHADER_RESOURCE);
			base.transition(g.Transform3x4.resource, ResourceState::NON_PIXEL_SHADER_RESOURCE);
		}

		base.transition(build_desc.DestAccelerationStructureData.resource, ResourceState::RAYTRACING_STRUCTURE);
		base.transition(build_desc.SourceAccelerationStructureData.resource, ResourceState::RAYTRACING_STRUCTURE);

		base.transition(build_desc.ScratchAccelerationStructureData.resource, ResourceState::UNORDERED_ACCESS);
		commit_tables();

		list->build_ras(build_desc, bottom);

		base.create_transition_point(false);
	}


	void ComputeContext::build_ras(const HAL::RaytracingBuildDescStructure& build_desc, const HAL::RaytracingBuildDescTopInputs& top)
	{
		base.create_transition_point();

		base.transition(build_desc.DestAccelerationStructureData.resource, ResourceState::RAYTRACING_STRUCTURE);
		base.transition(build_desc.SourceAccelerationStructureData.resource, ResourceState::RAYTRACING_STRUCTURE);
		base.transition(build_desc.ScratchAccelerationStructureData.resource, ResourceState::UNORDERED_ACCESS);

		base.transition(top.instances.resource, ResourceState::NON_PIXEL_SHADER_RESOURCE);

		commit_tables();
		list->build_ras(build_desc, top);

		base.create_transition_point(false);
	}
	void ComputeContext::set_const_buffer(UINT i, ResourceAddress address)
	{
		list->compute_set_const_buffer(i, address);
	}
	void  Transitions::begin()
	{
		transition_count = 0;
		//	create_zero_transition();
		create_transition_point(false);
	}

	void Transitions::on_execute()
	{

		transition_points.clear();
		used_resources.clear();


		transition_list = nullptr;


	}


	void SignatureDataSetter::set_pipeline(std::shared_ptr<PipelineStateBase> pipeline)
	{
		/*
				{
					for (auto& s : used_slots.slots_usage)
					{
						auto slot_id = get_slot_id(s);
						auto& slot = slots[slot_id];

						auto& used_tables = *slot.tables;

						for (auto& id : used_tables)
						{
							auto& table = tables[id].table;

							for (UINT i = 0; i < (UINT)table.get_count(); ++i)
							{
								const auto& h = table[i];

								base.stop_using(h.get_resource_info());

							}
						}
					}
				}
				*/
		if (pipeline->root_signature)
			set_signature(pipeline->root_signature);

		base.set_pipeline_internal(pipeline.get());


		//	used_slots = pipeline->slots;
	}
}