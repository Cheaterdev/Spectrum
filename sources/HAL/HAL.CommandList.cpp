module HAL:CommandList;
import <HAL.h>;

import :Autogen;

import :Device;
import Core;

import HAL;

//using namespace HAL;
namespace HAL

{

	void GPUBlock::start(Eventer* list)
	{
		//auto l = static_cast<CommandList*>(list);

		auto& timers = get_state(list);
		timers.timers.emplace_back();

		timers.timers.back().start(list);
		//		list->gpu_timers.emplace_back(std::make_pair<TimedBlock*, GPUTimer>(this, timer));

				// todo: make block independent
				//list->gpu_timers.back().second.start(list);
	}

	void GPUBlock::end(Eventer* list)
	{
		//auto l = static_cast<CommandList*>(list);

		auto& timers = get_state(list);

		timers.timers.back().end(list);
	}

	GPUTimer::GPUTimer()
	{
		//id = device.get_time_manager().factory.alloc(2, 1, QueryType::Timestamp);
	}

	GPUTimer::~GPUTimer()
	{
		//id.Free();
		//HAL::Device::get().get_time_manager().put_id(id);
	}

	void GPUTimer::start(Eventer* list)
	{
		queue_type = list->get_type();

		querys = list->alloc_query(2, QueryType::Timestamp);

		if (querys)
			list->insert_time(querys, 0);
	}

	void GPUTimer::end(Eventer* list)
	{
		if (querys)
			list->insert_time(querys, 1);
	}


	uint64 GPUTimer::get_start()
	{
		if (!querys) return 0;

		return querys.get_heap()->read_back_data[querys.get_offset() + 0];
	}

	uint64 GPUTimer::get_end()
	{
		if (!querys) return 0;
		return  querys.get_heap()->read_back_data[querys.get_offset() + 1];
	}

	CommandList::CommandList(CommandListType type) : Eventer(Device::get())

	{
		this->type = type;

		compiler.create(type);

		if (type == CommandListType::DIRECT || type == CommandListType::COMPUTE)
			compute.reset(new ComputeContext(*this));

		//	if (type == CommandListType::DIRECT || type == CommandListType::COPY)
		copy.reset(new CopyContext(*this));

		if (type == CommandListType::DIRECT)
			graphics.reset(new GraphicsContext(*this));

		compiler.set_name(L"SpectrumCommandList");


		debug_buffer = StructuredBufferView<Table::DebugStruct>(64, HAL::counterType::NONE, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);
	}

	void CommandList::setup_debug(SignatureDataSetter* setter)
	{
		if (!current_pipeline->debuggable) return;
		Slots::DebugInfo info;
		info.GetDebug() = debug_buffer.rwStructuredBuffer;
		setter->set(info);

	}

	void CommandList::print_debug()
	{
		if (!current_pipeline->debuggable) return;

		auto pso_name = current_pipeline->name;
		get_copy().read_buffer(debug_buffer.resource.get(), 0, 3 * sizeof(Table::DebugStruct), [this, pso_name](std::span<std::byte> memory)
			{

				LogBlock block(Log::get(), log_level_internal::level_all);

				if (first_debug_log)
				{
					block << "-----------------------------------------\n";

					first_debug_log = false;
				}
				auto result = reinterpret_cast<const Table::DebugStruct*>(memory.data());

				block << "DEBUG(" << name << "): " << pso_name << "\n";
				for (int i = 0; i < 3; i++)
				{
					block << "debug(" << i << "): " << result[i].v.x << " " << result[i].v.y << " " << result[i].v.z << " " << result[i].v.w << " " << "\n";
				}
				Log::get() << block;
			});

		clear_uav(debug_buffer.rwRAW);
	}


	void CommandList::begin(std::string name, Timer* t)
	{
		active = true;
		if (name.empty())
		{
			compiler.set_name(L"EmptyName");
		}
		else
			compiler.set_name(convert(name));
		
		compiler.reset();
#ifdef DEV
		begin_stack = Exceptions::get_stack_trace();
#endif

		HAL::Device::get().context_generator.generate(this);
		first_debug_log = true;

		if(!frame_resources) frame_resources = Device::get().get_frame_manager().begin_frame();
		set_proxy(frame_resources->get_storage());

		assert(proxy);
		//       Log::get() << "begin" << Log::endl;


		first_pipeline = nullptr;

		if (graphics) graphics->begin();
		if (compute) compute->begin();


		Transitions::begin();
		Eventer::begin(name, t);

		if (type != CommandListType::COPY) {
			compiler.set_descriptor_heaps(
				HAL::Device::get().get_descriptor_heap_factory().get_cbv_srv_uav_heap().get(),
				HAL::Device::get().get_descriptor_heap_factory().get_sampler_heap().get()
			);

			if (graphics) {
				
				graphics->set_signature(Layouts::DefaultLayout);
				graphics->set_proxy(frame_resources->get_storage());

			}
			if (compute) 
			{
				
				compute->set_signature(Layouts::DefaultLayout);
				compute->set_proxy(frame_resources->get_storage());


			}
		}
	}


	void CommandList::end()
	{
		//	create_transition_point(false);
			//	id = -1;
		current_pipeline = nullptr;

		if (graphics) graphics->end();
		if (compute) compute->end();

		Eventer::end();
		proxy->resolve_timers([&, this](const QueryType& type, uint64 from, uint64 to, QueryHeap::ptr heap)
					{
						assert(from == 0);
						resolve_times(heap.get(), static_cast<uint>(to), [heap](std::span<UINT64> data) {
							std::copy(data.begin(), data.end(), heap->read_back_data.begin());
							});
					});
		active = false;

		frame_resources->free_storage(proxy);
		proxy = nullptr;

	}

	void GraphicsContext::begin()
	{
		reset();
	

	}
	void GraphicsContext::end()
	{
		index = HAL::Views::IndexBuffer();
		compiled_rt = CompiledRT();
			reset_tables();
	}

	void ComputeContext::begin()
	{
		reset();
	}

	void ComputeContext::end()
	{
		current_compute_root_signature = nullptr;
			reset_tables();
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

	//	auto cmd = static_cast<CommandList*>(this);

		if (active)
			end();

			dynamic_cast<CommandList*>(this)->compile_transitions();
	
		auto ca = frame_resources->get_ca(type);
	 compiler.compile(*ca);
		 frame_resources->free_ca(ca);
	}

	FenceWaiter Sendable::execute(std::function<void()> f)
	{
		if(!compiler.is_compiled()) 
			Sendable::compile();

		//execute_fence = std::promise<FenceWaiter>();
		//execute_fence_result = execute_fence.get_future();
		if (f)
			on_execute_funcs.emplace_back(f);

		auto fence = Device::get().get_queue(type)->execute(dynamic_cast<CommandList*>(this));

		return fence;
	}

	void Sendable::execute_and_wait(std::function<void()> f)
	{
		execute(f).wait();
	}


	void Eventer::insert_time(QueryHandle& handle, uint offset)
	{
		track_object(*handle.get_heap());
		compiler.insert_time(handle, offset);

	}

	void Eventer::resolve_times(QueryHeap* pQueryHeap, uint32_t NumQueries, std::function<void(std::span<UINT64>)> f)
	{

		//CommandList* list = static_cast<CommandList*>(this); // :(
		auto info = read_data(NumQueries * sizeof(UINT64),GPUEntityStorageInterface::DEFAULT_ALIGN, static_cast<uint>(type) );

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


	void  GraphicsContext::set_const_buffer(UINT i, UINT offset, UINT v)
	{
		list->graphics_set_constant(i, offset, v);
	}

	void GraphicsContext::validate()
	{
		if constexpr (Debug::CheckErrors)	{
			PipelineState* graphics = dynamic_cast<PipelineState*>(get_base().current_pipeline);
			if (graphics)
			{

				{
					auto &pipeline_formats = graphics->desc.rtv.rtv_formats;
					std::vector<HAL::Format> rtv_formats = compiled_rt.get_formats();

					if(pipeline_formats!=rtv_formats)
					{
						Log::get() << "Wrong rendertarget format for pipeline " << graphics->name << Log::endl;
					}
				}


				{
					auto &pipeline_format = graphics->desc.rtv.ds_format;
					HAL::Format ds_format = compiled_rt.get_depth_format();

					if(pipeline_format!=ds_format)
					{
						Log::get() << "Wrong depthstencil format for pipeline " << graphics->name << Log::endl;
					}
				}


			}
		}
		
	}

	void GraphicsContext::set_rtv(const CompiledRT & rt,RTOptions options, float depth , uint stencil)
			{

		compiled_rt = rt;
		const RTVHandle& table_rtv = rt.table_rtv;
		const DSVHandle& table_dsv= rt.table_dsv;
	

			base.pre_command<false, false>(*this,BarrierSync::DRAW);

		for (uint i = 0; i < table_rtv.get_count(); i++)
		{
			get_base().transition(table_rtv[i].get_resource_info());
		}
		if (table_dsv)
		{
			get_base().transition(table_dsv.get_resource_info());
		}

		if (check(options & RTOptions::SetHandles))
			list->set_rtv(table_rtv.get_count(), table_rtv, table_dsv);

		if (check(options & RTOptions::ClearColor))
		{
			for (uint i = 0; i < table_rtv.get_count(); i++)
			{
				list->clear_rtv(table_rtv[i], float4(0, 0, 0, 0));
			}
		}

		if (table_dsv && check(options & (RTOptions::ClearStencil | RTOptions::ClearDepth)))
			list->clear_depth_stencil(table_dsv[0], check(options & RTOptions::ClearDepth), check(options & RTOptions::ClearStencil), depth, stencil);

		base.post_command<false, false>(*this);
	uint2 size;


			if (table_rtv)
			{
				auto& view = std::get<HAL::Views::RenderTarget>(table_rtv.get_resource_info().view);

				std::visit(overloaded{
					[&](const HAL::Views::RenderTarget::Buffer& Buffer) {
						assert(false);
					},
					[&](const HAL::Views::RenderTarget::Texture1D& Texture1D) {
						size = view.Resource->get_desc().as_texture().get_size(Texture1D.MipSlice).xy;

					},
					[&](const HAL::Views::RenderTarget::Texture1DArray& Texture1DArray) {
						size = view.Resource->get_desc().as_texture().get_size(Texture1DArray.MipSlice).xy;
					},
					[&](const HAL::Views::RenderTarget::Texture2D& Texture2D) {
						size = view.Resource->get_desc().as_texture().get_size(Texture2D.MipSlice).xy;
					},
					[&](const HAL::Views::RenderTarget::Texture2DArray& Texture2DArray) {
						size = view.Resource->get_desc().as_texture().get_size(Texture2DArray.MipSlice).xy;
					},
					[&](const HAL::Views::RenderTarget::Texture3D& Texture3D) {
						size = view.Resource->get_desc().as_texture().get_size(Texture3D.MipSlice).xy;
					},
					[&](const HAL::Views::RenderTarget::Texture2DMS& Texture2DMS) {
						assert(false);
					},
					[&](const HAL::Views::RenderTarget::Texture2DMSArray& Texture2DMSArray) {
						assert(false);
					},
					[&](auto other) {
						assert(false);
					}
					}, view.View);

			}
			else
				if (table_dsv)
				{
					auto& view = std::get<HAL::Views::DepthStencil>(table_dsv.get_resource_info().view);

					std::visit(overloaded{
						[&](const HAL::Views::DepthStencil::Texture1D& Texture1D) {
							size = view.Resource->get_desc().as_texture().get_size(Texture1D.MipSlice).xy;
						},
						[&](const HAL::Views::DepthStencil::Texture1DArray& Texture1DArray) {
							size = view.Resource->get_desc().as_texture().get_size(Texture1DArray.MipSlice).xy;
						},
						[&](const HAL::Views::DepthStencil::Texture2D& Texture2D) {
							size = view.Resource->get_desc().as_texture().get_size(Texture2D.MipSlice).xy;
						},
						[&](const HAL::Views::DepthStencil::Texture2DArray& Texture2DArray) {
							size = view.Resource->get_desc().as_texture().get_size(Texture2DArray.MipSlice).xy;
						},
						[&](const HAL::Views::DepthStencil::Texture2DMS& Texture2DMS) {
							assert(false);
						},
						[&](const HAL::Views::DepthStencil::Texture2DMSArray& Texture2DMSArray) {
							assert(false);
						},
						[&](auto other) {
							assert(false);
						}
						}, view.View);
				}

			if (size.x)
			{
				if (check(options & RTOptions::SetViewport))
				{
					std::vector<HAL::Viewport> vps(1);
					vps[0].size = size;
					vps[0].pos = { 0,0 };
					vps[0].depths = { 0,1 };

					set_viewports(vps);
				}

				if (check(options & RTOptions::SetScissors))
				{
					sizer_long scissors = { 0, 0, size.x, size.y };
					set_scissors(scissors);
				}
			}

			}

	//void GraphicsContext::set_rtv(int c, RTVHandle rt, DSVHandle h)
	//{



	//	base.create_transition_point();
	//	for (int i = 0; i < c; i++)
	//	{
	//		get_base().transition(rt[i].get_resource_info());
	//	}

	//	if (h.is_valid())
	//	{
	//		if (rt.is_valid()) {
	//			auto& i = rt.get_resource_info();
	//			auto rtv = std::get<HAL::Views::RenderTarget>(i.view);
	//			auto dsv = std::get<HAL::Views::DepthStencil>(h.get_resource_info().view);
	//			assert(rtv.Resource->get_desc().as_texture().Dimensions == dsv.Resource->get_desc().as_texture().Dimensions);
	//		}
	//		get_base().transition(h.get_resource_info());
	//	}
	//	list->set_rtv(c, rt, h);

	//	base.create_transition_point(false);
	//}



	void GraphicsContext::draw(UINT vertex_count, UINT vertex_offset, UINT instance_count, UINT instance_offset)
	{
		PROFILE_GPU(L"draw");

		base.pre_command<false, true>(*this,BarrierSync::DRAW);
		list->draw(vertex_count, vertex_offset, instance_count, instance_offset);
		base.post_command<false, true>(*this);
	}

	void GraphicsContext::draw_indexed(UINT index_count, UINT index_offset, UINT vertex_offset, UINT instance_count, UINT instance_offset)
	{
		if (instance_count == 0) return;
		PROFILE_GPU(L"draw_indexed");
		base.pre_command<false, true>(*this,BarrierSync::DRAW);

		get_base().transition(index.Resource, ResourceStates::INDEX_BUFFER);
		list->set_index_buffer(index);

		list->draw_indexed(index_count, index_offset, vertex_offset, instance_count, instance_offset);
		base.post_command<false, true>(*this);
	}
	void GraphicsContext::dispatch_mesh(D3D12_DISPATCH_MESH_ARGUMENTS args)
	{
		dispatch_mesh(ivec3{ args.ThreadGroupCountX, args.ThreadGroupCountY, args.ThreadGroupCountZ });
	}

	void GraphicsContext::dispatch_mesh(ivec3 v)
	{
		PROFILE_GPU(L"dispatch_mesh");
		base.pre_command<false, true>(*this,BarrierSync::DRAW);
		list->dispatch_mesh(v);
		base.post_command<false, true>(*this);
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

	void  CopyContext::update_buffer(Resource::ptr resource, uint64 offset, const  char* data, uint64 size)
	{
		update_buffer(resource.get(), offset, data, size);
	}
	void  CopyContext::update_buffer(Resource* resource, uint64 offset, const  char* data, uint64 size)
	{
		base.pre_command<false, false>(*this,BarrierSync::COPY);

		//	if (base.type != CommandListType::COPY)
		base.transition(resource, ResourceStates::COPY_DEST);

		auto info = base.place_data(size);
		memcpy(info.get_cpu_data(), data, size);
		list->copy_buffer(resource, offset, info.resource, info.resource_offset, size);
		base.post_command<false, false>(*this);

	}



	void CopyContext::update_texture(Resource::ptr resource, ivec3 offset, ivec3 box, UINT sub_resource, const char* data, UINT row_stride, UINT slice_stride)
	{
		update_texture(resource.get(), offset, box, sub_resource, data, row_stride, slice_stride);
	}

	void CopyContext::update_texture(Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, const char* data, UINT row_stride, UINT slice_stride)
	{
		base.pre_command<false, false>(*this,BarrierSync::COPY);

		base.transition(resource, ResourceStates::COPY_DEST);
		auto layout = Device::get().get_texture_layout(resource->get_desc(), sub_resource, box);
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



		list->update_texture(resource, offset, box, sub_resource, info, layout);
		if constexpr (Debug::CheckErrors)	TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->GetDeviceRemovedReason());
		base.post_command<false, false>(*this);

	}

	std::future<bool> CopyContext::read_texture(Resource::ptr resource, ivec3 offset, ivec3 box, UINT sub_resource, std::function<void(std::span<std::byte>, texture_layout)> f)
	{
		return read_texture(resource.get(), offset, box, sub_resource, f);
	}
	std::future<bool> CopyContext::read_texture(const Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, std::function<void(std::span<std::byte>, texture_layout)> f)
	{

		base.pre_command<false, false>(*this,BarrierSync::COPY);

		//	if (base.type != CommandListType::COPY)
		base.transition(resource, ResourceStates::COPY_SOURCE);
		//else
	//		base.transition(resource, ResourceState::COMMON);

		auto layout = Device::get().get_texture_layout(resource->get_desc(), sub_resource, box);
		auto info = base.read_data(layout.size, layout.alignment, static_cast<uint>(base.get_type()) );
		list->read_texture(resource, offset, box, sub_resource, info, layout);

		if constexpr (Debug::CheckErrors)	TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->GetDeviceRemovedReason());
		auto result = std::make_shared<std::promise<bool>>();
		base.on_execute_funcs.push_back([result, info, f, layout]()
			{
				f({ info.get_cpu_data(),info.size }, layout);
				result->set_value(true);
			});

		base.post_command<false, false>(*this);

		return result->get_future();
	}
	std::future<bool> CopyContext::read_texture(const HAL::Resource* resource, UINT sub_resource, std::function<void(std::span<std::byte>, texture_layout)> f)
	{
		base.pre_command<false, false>(*this,BarrierSync::COPY);

		//	if (base.type != CommandListType::COPY)
		base.transition(resource, ResourceStates::COPY_SOURCE);
		//else
	//		base.transition(resource, ResourceState::COMMON);

		auto layout = Device::get().get_texture_layout(resource->get_desc(), sub_resource);
		auto info = base.read_data(layout.size, layout.alignment,static_cast<uint>(base.get_type()) );
		list->read_texture(resource, 0, resource->get_desc().as_texture().get_size(sub_resource), sub_resource, info, layout);

		if constexpr (Debug::CheckErrors)	TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->GetDeviceRemovedReason());
		auto result = std::make_shared<std::promise<bool>>();
		base.on_execute_funcs.push_back([result, info, f, layout]()
			{
				f({ info.get_cpu_data(),info.size }, layout);
				result->set_value(true);
			});

		base.post_command<false, false>(*this);

		return result->get_future();
	}

	std::future<bool> CopyContext::read_buffer(Resource* resource, unsigned int offset, UINT64 size, std::function<void(std::span<std::byte>)> f)
	{

		auto result = std::make_shared<std::promise<bool>>();

		if (size == 0)
		{
			f({});
			result->set_value(true);
			return result->get_future();
		}

		base.pre_command<false, false>(*this,BarrierSync::COPY);

		base.transition(resource, ResourceStates::COPY_SOURCE);

		auto info = base.read_data(size,GPUEntityStorageInterface::DEFAULT_ALIGN, static_cast<uint>(base.get_type()) );
		list->copy_buffer(info.resource, info.resource_offset, resource, offset, size);
		if constexpr (Debug::CheckErrors)	TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->GetDeviceRemovedReason());
		base.on_execute_funcs.push_back([result, info, f, size]()
			{
				f({ reinterpret_cast<std::byte*>(info.get_cpu_data()), size });
				result->set_value(true);
			});

		base.post_command<false, false>(*this);

		return result->get_future();
	}

	std::future<bool> CopyContext::read_query(std::shared_ptr<QueryHeap>& query_heap, unsigned int offset, unsigned int size, std::function<void(std::span<std::byte>)> f)
	{
		if (size == 0)
		{
			auto result = std::make_shared<std::promise<bool>>();
			f({});
			result->set_value(true);
			return result->get_future();
		}

		//  auto size = resource->get_size();
		auto info = base.read_data(size,GPUEntityStorageInterface::DEFAULT_ALIGN, static_cast<uint>(base.get_type()) );
		//  compiler.CopyResource(info.resource->get_resource()->get_native().Get(), resource->get_native().Get());
		//list->ResolveQueryData(query_heap->get_native().Get(), D3D12_QUERY_TYPE_PIPELINE_STATISTICS, 0, 1, info.resource->get_dx(), info.resource_offset);
		assert(false);
		if constexpr (Debug::CheckErrors)	TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->GetDeviceRemovedReason());
		auto result = std::make_shared<std::promise<bool>>();
		base.on_execute_funcs.push_back([result, info, f, size]()
			{
				f({ reinterpret_cast<std::byte*>(info.get_cpu_data()), size });
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

		Eventer::reset();

	


		//GPUEntityStorage<LocalAllocationPolicy>::reset();

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



	std::shared_ptr<TransitionCommandList> Transitions::fix_pretransitions()
	{
		PROFILE(L"fix_pretransitions");

		HAL::Barriers result(CommandListType::DIRECT);
		std::vector<HAL::Resource*> discards;


		CommandListType transition_type = type;
		for (auto& r : used_resources)
		{
			auto res_type =  r->get_state_manager().process_transitions(result, discards, this);

			transition_type= Merge(transition_type,res_type);
		}

		if (result)
		{
			auto cmd = static_cast<CommandList*>(this);


	auto		transition_list = (HAL::Device::get().get_queue(transition_type)->get_transition_list());
	transition_list->compiler.set_name(convert(name + std::string(":Transitions")));
			transition_list->create_transition_list(*cmd->frame_resources, result, discards);
			return transition_list;
		}
		return nullptr;
	}
	//void Transitions::merge_transition(Transitions* to, Resource* resource)
	//{


	//	if (resource->get_state_manager().transition(this, to))
	//	{
	//		track_object(*resource);
	//		use_resource(resource);
	//	}
	//}
	/*void Transitions::prepare_transitions(Transitions* to, bool all)
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
	}*/


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
	}

	void Transitions::transition(const Resource* resource, ResourceState to, UINT subres)
	{
		if (!resource) return;

		//if (type == CommandListType::COPY && (to == ResourceState::COPY_DEST || to == ResourceState::COPY_SOURCE))
		//	to = ResourceState::COMMON;


		track_object(*const_cast<Resource*>(resource));

	
		if (resource->get_heap_type() == HeapType::DEFAULT || resource->get_heap_type() == HeapType::RESERVED)
		{
			use_resource(resource);
			const_cast<Resource*>(resource)->get_state_manager().transition(this, to, subres);
		}
	}


	void Transitions::alias_begin(HAL::Resource* resource)
	{
			track_object(*resource);
			const_cast<Resource*>(resource)->get_state_manager().alias_begin(this);
	}
	void Transitions::alias_end(HAL::Resource* resource)
	{
			track_object(*resource);

				const_cast<Resource*>(resource)->get_state_manager().alias_end(this);
			//transition(resource, ResourceStates::NO_ACCESS);
	}



	void CopyContext::copy_buffer(Resource* dest, uint64 s_dest, Resource* source, uint64 s_source, uint64 size)
	{
		base.pre_command<false, false>(*this,BarrierSync::COPY);

		//if (base.type != CommandListType::COPY)
		{
			base.transition(source, ResourceStates::COPY_SOURCE);
			base.transition(dest, ResourceStates::COPY_DEST);
		}


		list->copy_buffer(dest, s_dest, source, s_source, size);
		base.post_command<false, false>(*this);

	}
	void CopyContext::copy_resource(Resource* dest, Resource* source)
	{
		base.pre_command<false, false>(*this,BarrierSync::COPY);

		//	if (base.type != CommandListType::COPY)
		{
			base.transition(source, ResourceStates::COPY_SOURCE);
			base.transition(dest, ResourceStates::COPY_DEST);
		}
		list->copy_resource(dest, source);
		base.post_command<false, false>(*this);

	}
	void CopyContext::copy_resource(const Resource::ptr& dest, const Resource::ptr& source)
	{
		copy_resource(dest.get(), source.get());
	}
	void CopyContext::copy_texture(const Resource::ptr& dest, int dest_subres, const Resource::ptr& source, int source_subres)
	{
		base.pre_command<false, false>(*this,BarrierSync::COPY);

		//if (base.type != CommandListType::COPY) 
		{
			base.transition(source, ResourceStates::COPY_SOURCE, source_subres);
			base.transition(dest, ResourceStates::COPY_DEST, dest_subres);
		}

		list->copy_texture(dest, dest_subres, source, source_subres);
		if constexpr (Debug::CheckErrors)	TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->GetDeviceRemovedReason());
		base.post_command<false, false>(*this);

	}

	void CopyContext::copy_texture(const Resource::ptr& to, ivec3 to_pos, const Resource::ptr& from, ivec3 from_pos, ivec3 size)
	{
		base.pre_command<false, false>(*this,BarrierSync::COPY);

		//if (base.type != CommandListType::COPY) 
		{
			base.transition(from, ResourceStates::COPY_SOURCE);
			base.transition(to, ResourceStates::COPY_DEST);
		}
		list->copy_texture(to, to_pos, from, from_pos, size);
		if constexpr (Debug::CheckErrors)	TEST(Device::get(), Device::get().get_native_device()->GetDeviceRemovedReason());
		base.post_command<false, false>(*this);
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

	void ComputeContext::dispatch(int x, int y, int z)
	{
		PROFILE_GPU(L"Dispatch");

		base.pre_command<true, false>(*this,BarrierSync::COMPUTE_SHADING);
		list->dispatch({ x, y, z });
		base.post_command<true, false>(*this);
	}


	void ComputeContext::dispatch(ivec2 a, ivec2 b)
	{
		dispatch(ivec3(a, 1), ivec3(b, 1));
	}

	void ComputeContext::dispatch(ivec3 a, ivec3 b)
	{

		ivec3 res;
		res.x = Math::DivideByMultiple(a.x, b.x);
		res.y = Math::DivideByMultiple(a.y, b.y);
		res.z = Math::DivideByMultiple(a.z, b.z);
		dispatch(res.x, res.y, res.z);
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
			gpu_timers.emplace_back(b);
			b->start(this);
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

			b->end(this);
		}

		current = timer->get_block().get_parent().get();
		end_event();
	}
	thread_local Eventer* Eventer::thread_current = nullptr;
	void Eventer::reset()
	{
			for (auto& e : gpu_timers)
		{
			auto block = e;
			auto& timers = block->get_state(this);
			for (auto& t : timers.timers)
				Profiler::get().on_gpu_timer(std::make_pair<TimedBlock*, GPUTimerInterface*>(block, &t));
		}
		gpu_timers.clear();
	}

	void Eventer::end()
	{
		started = false;
		thread_current = nullptr;
		timer.reset();

	}
	void Eventer::begin(std::string name, Timer* t)
	{
		assert(!started);
		started = true;
		this->name = name;
		thread_current = this;

		current = t ? &t->get_block() : &Profiler::get();
		TimedRoot::parent = t ? t->get_root() : &Profiler::get();
		if (type != CommandListType::COPY && name.size())
		{
			assert(!timer);
			timer.reset(new Timer(start(convert(name).c_str())));
		}

		else
			current = nullptr;


		names.clear();
	}

	Timer  Eventer::start(std::wstring_view name)
	{
		if (Profiler::get().enabled)
			return Timer(&current->get_child<GPUBlock>(name, HAL::Device::get()), this);
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
			assert(command_buffer);
		//	assert(dynamic_cast<PipelineState*>(get_base().current_pipeline));
		PROFILE_GPU(L"execute_indirect");

		bool graphics = dynamic_cast<PipelineState*>(get_base().current_pipeline);

		if(graphics)
			base.pre_command<false, true>(get_base().get_graphics(),BarrierSync::ALL_SHADING,&command_types.slots);
		else
			base.pre_command<true, false>(get_base().get_compute(),BarrierSync::COMPUTE_SHADING,&command_types.slots);
	
		if (command_buffer) get_base().transition(command_buffer, ResourceStates::INDIRECT_ARGUMENT);
		if (counter_buffer) get_base().transition(counter_buffer, ResourceStates::INDIRECT_ARGUMENT);

		get_base().transition(static_cast<HAL::Resource*>(index.Resource.get()), ResourceStates::INDEX_BUFFER);

		list->set_index_buffer(index);

		list->execute_indirect(
			command_types,
			max_commands,
			command_buffer,
			command_offset,
			counter_buffer,
			counter_offset);
	
		if(graphics)
			base.post_command<false, true>(get_base().get_graphics());
		else
			base.post_command<true, false>(get_base().get_compute());
	
	}

	void ComputeContext::execute_indirect(IndirectCommand& command_types, UINT max_commands, Resource* command_buffer, UINT64 command_offset, Resource* counter_buffer, UINT64 counter_offset)
	{
		assert(command_buffer);
		PROFILE_GPU(L"execute_indirect");
		base.pre_command<true, false>(*this,BarrierSync::COMPUTE_SHADING);

		assert(command_buffer);

		if (command_buffer) get_base().transition(command_buffer, ResourceStates::INDIRECT_ARGUMENT);
		if (counter_buffer) get_base().transition(counter_buffer, ResourceStates::INDIRECT_ARGUMENT);

		list->execute_indirect(
			command_types,
			max_commands,
			command_buffer,
			command_offset,
			counter_buffer,
			counter_offset);

		base.post_command<true, false>(*this);
	}

	void ComputeContext::build_ras(const HAL::RaytracingBuildDescStructure& build_desc, const HAL::RaytracingBuildDescBottomInputs& bottom)
	{
		base.pre_command<false, false>(*this,BarrierSync::BUILD_RAYTRACING_ACCELERATION_STRUCTURE);

		for (auto g : bottom.geometry)
		{
			base.transition(g.IndexBuffer.resource, HAL::ResourceState( BarrierSync::COMPUTE_SHADING, BarrierAccess::SHADER_RESOURCE, TextureLayout::UNDEFINED));
			base.transition(g.VertexBuffer.resource,  { BarrierSync::COMPUTE_SHADING, BarrierAccess::SHADER_RESOURCE, TextureLayout::UNDEFINED});
			base.transition(g.Transform3x4.resource,  { BarrierSync::COMPUTE_SHADING, BarrierAccess::SHADER_RESOURCE, TextureLayout::UNDEFINED});
		}

		base.transition(build_desc.DestAccelerationStructureData.resource, ResourceStates::RAYTRACING_STRUCTURE_WRITE);
		base.transition(build_desc.SourceAccelerationStructureData.resource, ResourceStates::RAYTRACING_STRUCTURE_WRITE);

		base.transition(build_desc.ScratchAccelerationStructureData.resource, { BarrierSync::COMPUTE_SHADING, BarrierAccess::UNORDERED_ACCESS, TextureLayout::UNDEFINED});
		//	commit_tables();

		list->build_ras(build_desc, bottom);

		base.post_command<false, false>(*this);
	}


	void ComputeContext::build_ras(const HAL::RaytracingBuildDescStructure& build_desc, const HAL::RaytracingBuildDescTopInputs& top)
	{
		base.pre_command<false, false>(*this,BarrierSync::BUILD_RAYTRACING_ACCELERATION_STRUCTURE);

		base.transition(build_desc.DestAccelerationStructureData.resource, ResourceStates::RAYTRACING_STRUCTURE_WRITE);
		base.transition(build_desc.SourceAccelerationStructureData.resource, ResourceStates::RAYTRACING_STRUCTURE_WRITE);
		base.transition(build_desc.ScratchAccelerationStructureData.resource, { BarrierSync::COMPUTE_SHADING, BarrierAccess::UNORDERED_ACCESS, TextureLayout::UNDEFINED});

		base.transition(top.instances.resource,{ BarrierSync::COMPUTE_SHADING, BarrierAccess::SHADER_RESOURCE, TextureLayout::UNDEFINED});

		//commit_tables();
		list->build_ras(build_desc, top);

		base.post_command<false, false>(*this);
	}
	void ComputeContext::set_const_buffer(UINT i, UINT offset, UINT v)
	{
		list->compute_set_constant(i, offset, v);
	}
	void  Transitions::begin()
	{
		transition_count = 0;
		//	create_zero_transition();
		create_usage_point(false);
	}

	void Transitions::on_execute()
	{

		usage_points.clear();
		used_resources.clear();


		//transition_list = nullptr;


	}

	void SignatureDataSetter::commit_tables( BarrierSync operation, UsedSlots* slots)
	{
		uint id = 0;
		for (auto& table : tables)
		{

			if (table.dirty) {
				for (auto& resource_info : table.resources)
					get_base().transition(*resource_info,operation);

				for (auto& d : table.descriptors)
					get_base().track_object(*d);

				set_cb(id, table.const_buffer, operation);

				table.dirty = false;
			}
			id++;
		}

		if constexpr (Debug::CheckErrors)
		{
			auto pipeline = get_base().current_pipeline;
			for (auto& slot : pipeline->slots.slots_usage)
			{
				auto id = get_table_index(slot);

				if (tables[id].slot_id != slot)
				{
					bool found = false;
					if (slots)
					{
						found = slots->uses(slot);
					}

					if (!found)
						Log::get() << "Possible null slot " << get_slot_name(slot) << " for pipeline " << pipeline->name << Log::endl;
				}
			}
		}
	}

	void SignatureDataSetter::stop_using(uint id)
	{
		auto& table = tables[id];

		for (auto & res:table.resources)
		{
			base.stop_using(*res);
		}
	}


	void SignatureDataSetter::set_pipeline(std::shared_ptr<PipelineStateBase> pipeline)
	{

		{

			std::set<SlotID> diff;

			std::set_difference(used_slots.slots_usage.begin(), used_slots.slots_usage.end(), pipeline->slots.slots_usage.begin(), pipeline->slots.slots_usage.end(),
				std::inserter(diff, diff.begin()));


			for (auto& s : diff)
			{
				auto i = get_table_index(s);

				if(tables[i].slot_id == s);
				stop_using(i);
				//auto& slot = slots[slot_id];

				//for (auto& id : used_tables)
				//{

				//	stop_using(id);
				//	/*auto& table = tables[id].table;

				//	for (UINT i = 0; i < (UINT)table.get_count(); ++i)
				//	{
				//		const auto& h = table[i];

				//		base.stop_using(h.get_resource_info());

				//	}*/
				//}
			}
		}
				
		if (pipeline->root_signature)
			set_signature(pipeline->root_signature);

		base.set_pipeline_internal(pipeline.get());


		used_slots = pipeline->slots;
	}


		std::vector<HAL::Format> CompiledRT::get_formats() const
		{
			std::vector<HAL::Format> result;

			if (table_rtv)
			{

				for (uint i = 0; i < table_rtv.get_count(); i++)
				{
					auto& view = std::get<HAL::Views::RenderTarget>(table_rtv[i].get_resource_info().view);
					result.emplace_back(view.Format);
				}

			}
			return result;
		}
		HAL::Format CompiledRT::get_depth_format() const
		{
			if (!table_dsv) return HAL::Format::UNKNOWN;
			auto& view = std::get<HAL::Views::DepthStencil>((table_dsv).get_resource_info().view);

			return view.Format;
		}



		//const CompiledRT& CompiledRT::set(HAL::GraphicsContext& context, HAL::RTOptions options, float depth, uint stencil) const
		//{
		//	context.set_rtv(*this,options,depth, stencil);
		//	return *this;
		//}

		void TransitionCommandList::create_transition_list(FrameResources& frame, const HAL::Barriers& transitions, std::vector<HAL::Resource*>& discards)
		{
				HAL::Device::get().context_generator.generate(this);
	
			set_proxy(frame.get_storage());
		

			compiler.reset();
			Eventer::begin("Transitions");
			compiler.func([&](auto list){
				list.transitions(transitions);
				});

			Eventer::end();	
			
			proxy->resolve_timers([&, this](const QueryType& type, uint64 from, uint64 to, QueryHeap::ptr heap)
					{
						assert(from == 0);
						resolve_times(heap.get(), static_cast<uint>(to), [heap](std::span<UINT64> data) {
							std::copy(data.begin(), data.end(), heap->read_back_data.begin());
							});
					});
				HAL::Device::get().context_generator.free(this);
			frame.free_storage(proxy);


				auto ca = frame.get_ca(type);
			compiler.compile(*ca);
		end();

			frame.free_ca(ca);


		}


		TransitionCommandList::TransitionCommandList(CommandListType type) :Eventer(HAL::Device::get())
		{
			this->type=type;
			compiler.create(type);

		}
}