module HAL:CommandList;


import :Autogen;

import :Device;
import Core;

import HAL;

//using namespace HAL;
namespace HAL

{
	void GPUBlock::start(Eventer* list)
	{
		gpu_timer.start(list);
	}

	void GPUBlock::end(Eventer* list)
	{
		gpu_timer.end(list);
	}

	GPUTimer::GPUTimer()
	{
	}

	GPUTimer::~GPUTimer()
	{
	}

	void GPUTimer::start(Eventer* list)
	{
		queue_type = list->get_type();

		querys.Free();

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
		return querys.get_heap()->read_back_data[querys.get_offset() + 1];
	}

	CommandList::CommandList(CommandListType type, Device& device) : Eventer(device)
	{
		this->type = type;

		compiler.create(type, device);

		if (type == CommandListType::DIRECT || type == CommandListType::COMPUTE)
			compute.reset(new ComputeContext(*this));

		copy.reset(new CopyContext(*this));

		if (type == CommandListType::DIRECT)
			graphics.reset(new GraphicsContext(*this));

		compiler.set_name(L"SpectrumCommandList");


		debug_buffer = StructuredBufferView<Table::DebugStruct>(device, 64, HAL::counterType::NONE,
			HAL::ResFlags::ShaderResource |
			HAL::ResFlags::UnorderedAccess);
	}

	void CommandList::setup_debug(SignatureDataSetter* setter)
	{
		if (!current_pipeline || !current_pipeline->debuggable) return;
		Slots::DebugInfo info;
		info.debug = debug_buffer;
		setter->set(info);
	}

	void CommandList::print_debug()
	{
		if (!current_pipeline->debuggable) return;

		auto pso_name = current_pipeline->name;
		get_copy().read<Table::DebugStruct>(debug_buffer, 0, 3, [this, pso_name](std::span<Table::DebugStruct> result)
			{
				LogBlock block(Log::get(), log_level_internal::level_all);

				if (first_debug_log)
				{
					block << "-----------------------------------------\n";

					first_debug_log = false;
				}

				block << "DEBUG(" << name << "): " << pso_name << "\n";
				for (int i = 0; i < 3; i++)
				{
					block << "debug(" << i << "): " << result[i].v.x << " " << result[i].v.y << " " << result[i].v.z << " "
						<< result[i].v.w << " " << "\n";
				}
				Log::get() << block;
			});

		get_compute().clear(debug_buffer);
	}


	void CommandList::begin(std::wstring_view name)
	{
		active = true;
		if (name.empty())
		{
			compiler.set_name(L"EmptyName");
		}
		else
			compiler.set_name(name);

		compiler.reset();
#ifdef DEV
		begin_stack = Exceptions::get_stack_trace();
#endif

		device.context_generator.generate(this);
		first_debug_log = true;

		if (!frame_resources) frame_resources = device.get_frame_manager().begin_frame();
		auto proxy = frame_resources->get_storage();
		set_proxy(proxy);


		first_pipeline = nullptr;

		if (graphics) graphics->begin();
		if (compute) compute->begin();


		Transitions::begin();
		Eventer::begin(name);

		if (type != CommandListType::COPY)
		{
			compiler.set_descriptor_heaps(
				device.get_descriptor_heap_factory().get_cbv_srv_uav_heap().get(),
				device.get_descriptor_heap_factory().get_sampler_heap().get()
			);

			if (graphics)
			{
				graphics->set_signature(Layouts::DefaultLayout);
				graphics->set_proxy(proxy);
			}
			if (compute)
			{
				compute->set_signature(Layouts::DefaultLayout);
				compute->set_proxy(proxy);
			}
		}
	}


	void CommandList::end()
	{
		current_pipeline = nullptr;

		if (graphics) graphics->end();
		if (compute) compute->end();

		Eventer::end();
		proxy->resolve_timers([&, this](const QueryType& type, uint64 from, uint64 to, QueryHeap::ptr heap)
			{
				ASSERT(from == 0);
				resolve_times(heap.get(), static_cast<uint>(to), [heap](std::span<UINT64> data)
					{
						std::copy(data.begin(), data.end(), heap->read_back_data.begin());
					});
			});
		active = false;
		frame_resources->free_storage(proxy);

		set_proxy(nullptr);
		if (graphics) graphics->set_proxy(nullptr);
		if (compute) compute->set_proxy(nullptr);
	}

	void CommandList::discard(HAL::Resource* resource)
	{
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
		if (active)
			end();

		
		dynamic_cast<CommandList*>(this)->compile_transitions();

		auto ca = frame_resources->get_ca(type);
		compiler.compile(*ca);
		frame_resources->free_ca(ca);
	}

	FenceWaiter Sendable::execute(std::function<void()> f)
	{
		if (!compiler.is_compiled())
			Sendable::compile();

		if (f)
			on_execute_funcs.emplace_back(f);


		std::list<CommandList::ptr>	lists;
		  lists.emplace_back(dynamic_cast<CommandList*>(this)->get_ptr());
		auto fence = dynamic_cast<CommandList*>(this)->get_device().get_queue(type)->execute(lists);

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
		auto info = read_data(NumQueries * sizeof(UINT64), GPUEntityStorageInterface::DEFAULT_ALIGN,
		                      static_cast<uint>(type));

		compiler.resolve_times(pQueryHeap, NumQueries, info);
		on_execute_funcs.emplace_back([info, f, NumQueries]()
		{
			auto data = reinterpret_cast<UINT64*>(info.get_cpu_data());
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


	void GraphicsContext::set_const_buffer(UINT i, UINT offset, UINT v)
	{
		list->graphics_set_constant(i, offset, v);
	}

	void GraphicsContext::validate()
	{
		if constexpr (Debug::CheckErrors)
		{
			auto graphics = dynamic_cast<PipelineState*>(get_base().current_pipeline);
			if (graphics)
			{
				{
					auto& pipeline_formats = graphics->desc.rtv.rtv_formats;
					std::vector<HAL::Format> rtv_formats = compiled_rt.get_formats();

					if (pipeline_formats != rtv_formats)
					{
						Log::get() << "Wrong rendertarget format for pipeline " << graphics->name << Log::endl;
					}
				}


				{
					auto& pipeline_format = graphics->desc.rtv.ds_format;
					HAL::Format ds_format = compiled_rt.get_depth_format();

					if (pipeline_format != ds_format)
					{
						Log::get() << "Wrong depthstencil format for pipeline " << graphics->name << Log::endl;
					}
				}
			}
		}
	}

	void GraphicsContext::set_rtv(const CompiledRT& rt, RTOptions options, float depth, uint stencil, vec4 clear_color)
	{
		compiled_rt = rt;
		const RTVHandle& table_rtv = rt.table_rtv;
		const DSVHandle& table_dsv = rt.table_dsv;


		base.pre_command<false, false>(*this, BarrierSync::DRAW);

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
				list->clear_rtv(table_rtv[i], clear_color);
			}
		}

		if (table_dsv && check(options & (RTOptions::ClearStencil | RTOptions::ClearDepth)))
			list->clear_depth_stencil(table_dsv[0], check(options & RTOptions::ClearDepth),
			                          check(options & RTOptions::ClearStencil), depth, stencil);

		base.post_command<false, false>(*this, BarrierSync::DRAW);
		uint2 size;


		if (table_rtv)
		{
			auto& view = std::get<HAL::Views::RenderTarget>(table_rtv.get_resource_info().view);

			std::visit(overloaded{
				           [&](const HAL::Views::RenderTarget::Buffer& Buffer)
				           {
					           ASSERT(false);
				           },
				           [&](const HAL::Views::RenderTarget::Texture1D& Texture1D)
				           {
					           size = view.Resource->get_desc().as_texture().get_size(Texture1D.MipSlice).xy;
				           },
				           [&](const HAL::Views::RenderTarget::Texture1DArray& Texture1DArray)
				           {
					           size = view.Resource->get_desc().as_texture().get_size(Texture1DArray.MipSlice).xy;
				           },
				           [&](const HAL::Views::RenderTarget::Texture2D& Texture2D)
				           {
					           size = view.Resource->get_desc().as_texture().get_size(Texture2D.MipSlice).xy;
				           },
				           [&](const HAL::Views::RenderTarget::Texture2DArray& Texture2DArray)
				           {
					           size = view.Resource->get_desc().as_texture().get_size(Texture2DArray.MipSlice).xy;
				           },
				           [&](const HAL::Views::RenderTarget::Texture3D& Texture3D)
				           {
					           size = view.Resource->get_desc().as_texture().get_size(Texture3D.MipSlice).xy;
				           },
				           [&](const HAL::Views::RenderTarget::Texture2DMS& Texture2DMS)
				           {
					           ASSERT(false);
				           },
				           [&](const HAL::Views::RenderTarget::Texture2DMSArray& Texture2DMSArray)
				           {
					           ASSERT(false);
				           },
				           [&](auto other)
				           {
					           ASSERT(false);
				           }
			           }, view.View);
		}
		else if (table_dsv)
		{
			auto& view = std::get<HAL::Views::DepthStencil>(table_dsv.get_resource_info().view);

			std::visit(overloaded{
				           [&](const HAL::Views::DepthStencil::Texture1D& Texture1D)
				           {
					           size = view.Resource->get_desc().as_texture().get_size(Texture1D.MipSlice).xy;
				           },
				           [&](const HAL::Views::DepthStencil::Texture1DArray& Texture1DArray)
				           {
					           size = view.Resource->get_desc().as_texture().get_size(Texture1DArray.MipSlice).xy;
				           },
				           [&](const HAL::Views::DepthStencil::Texture2D& Texture2D)
				           {
					           size = view.Resource->get_desc().as_texture().get_size(Texture2D.MipSlice).xy;
				           },
				           [&](const HAL::Views::DepthStencil::Texture2DArray& Texture2DArray)
				           {
					           size = view.Resource->get_desc().as_texture().get_size(Texture2DArray.MipSlice).xy;
				           },
				           [&](const HAL::Views::DepthStencil::Texture2DMS& Texture2DMS)
				           {
					           ASSERT(false);
				           },
				           [&](const HAL::Views::DepthStencil::Texture2DMSArray& Texture2DMSArray)
				           {
					           ASSERT(false);
				           },
				           [&](auto other)
				           {
					           ASSERT(false);
				           }
			           }, view.View);
		}

		if (size.x)
		{
			if (check(options & RTOptions::SetViewport))
			{
				std::vector<HAL::Viewport> vps(1);
				vps[0].size = size;
				vps[0].pos = {0, 0};
				vps[0].depths = {0, 1};

				set_viewports(vps);
			}

			if (check(options & RTOptions::SetScissors))
			{
				sizer_long scissors = {0, 0, size.x, size.y};
				set_scissors(scissors);
			}
		}
	}

	void GraphicsContext::draw(UINT vertex_count, UINT vertex_offset, UINT instance_count, UINT instance_offset)
	{
		PROFILE_GPU(L"draw");

		base.pre_command<false, true>(*this, BarrierSync::DRAW);
		list->draw(vertex_count, vertex_offset, instance_count, instance_offset);
		base.post_command<false, true>(*this, BarrierSync::DRAW);
	}

	void GraphicsContext::draw_indexed(UINT index_count, UINT index_offset, UINT vertex_offset, UINT instance_count,
	                                   UINT instance_offset)
	{
		if (instance_count == 0) return;
		PROFILE_GPU(L"draw_indexed");
		base.pre_command<false, true>(*this, BarrierSync::DRAW);

		get_base().transition(index.Resource, ResourceStates::INDEX_BUFFER);
		list->set_index_buffer(index);

		list->draw_indexed(index_count, index_offset, vertex_offset, instance_count, instance_offset);
		base.post_command<false, true>(*this, BarrierSync::DRAW);
	}

	void GraphicsContext::dispatch_mesh(DispatchMeshArguments args)
	{
		dispatch_mesh(ivec3{args.ThreadGroupCountX, args.ThreadGroupCountY, args.ThreadGroupCountZ});
	}

	void GraphicsContext::dispatch_mesh(ivec3 v)
	{
		PROFILE_GPU(L"dispatch_mesh");
		base.pre_command<false, true>(*this, BarrierSync::DRAW);
		list->dispatch_mesh(v);
		base.post_command<false, true>(*this, BarrierSync::DRAW);
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
		this->viewports[0].depths = {0, 1};
		set_viewports(viewports);
	}

	void CopyContext::update_buffer(Resource::ptr resource, uint64 offset, const char* data, uint64 size)
	{
		update_buffer(resource.get(), offset, data, size);
	}

	void CopyContext::update_buffer(Resource* resource, uint64 offset, const char* data, uint64 size)
	{
		base.pre_command<false, false>(*this, BarrierSync::COPY);

		base.transition(resource, ResourceStates::COPY_DEST);

		auto info = base.place_data(size);
		std::memcpy(info.get_cpu_data(), data, size);
		list->copy_buffer(resource, offset, info.resource, info.resource_offset, size);
		base.post_command<false, false>(*this, BarrierSync::COPY);;
	}


	void CopyContext::update_texture(Resource::ptr resource, ivec3 offset, ivec3 box, UINT sub_resource,
	                                 const char* data, UINT row_stride, UINT slice_stride)
	{
		update_texture(resource.get(), offset, box, sub_resource, data, row_stride, slice_stride);
	}

	void CopyContext::update_texture(Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, const char* data,
	                                 UINT row_stride, UINT slice_stride)
	{
		base.pre_command<false, false>(*this, BarrierSync::COPY);

		base.transition(resource, ResourceStates::COPY_DEST);
		auto layout = base.get_device().get_texture_layout(resource->get_desc(), sub_resource, box);
		auto info = base.place_data(layout.size, layout.alignment);

		if (layout.row_stride == row_stride)
		{
			if (slice_stride == 0 || slice_stride == row_stride * layout.rows_count)
			{
				auto pDestSlice = info.get_cpu_data();
				auto pSrcSlice = reinterpret_cast<const std::byte*>(data);
				std::memcpy(pDestSlice,
				       pSrcSlice,
				       layout.size);
			}

			else
			{
				for (UINT z = 0; z < static_cast<UINT>(box.z); ++z)
				{
					auto pDestSlice = info.get_cpu_data() + layout.slice_stride * z;
					const std::byte* pSrcSlice = reinterpret_cast<const std::byte*>(data) + slice_stride * z;
					std::memcpy(pDestSlice,
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
					std::memcpy(pDestSlice + layout.row_stride * y,
					       pSrcSlice + row_stride * y,
					       row_stride);
				}
			}


		list->update_texture(resource, offset, box, sub_resource, info, layout);
		if constexpr (Debug::CheckErrors)
			TEST(base.get_device(), base.get_device().get_device_removed_reason());
		base.post_command<false, false>(*this, BarrierSync::COPY);
	}

	std::future<bool> CopyContext::read_texture(Resource::ptr resource, ivec3 offset, ivec3 box, UINT sub_resource,
	                                            std::function<void(std::span<std::byte>, texture_layout)> f)
	{
		return read_texture(resource.get(), offset, box, sub_resource, f);
	}

	std::future<bool> CopyContext::read_texture(const Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource,
	                                            std::function<void(std::span<std::byte>, texture_layout)> f)
	{
		base.pre_command<false, false>(*this, BarrierSync::COPY);

		base.transition(resource, ResourceStates::COPY_SOURCE);

		auto layout = base.get_device().get_texture_layout(resource->get_desc(), sub_resource, box);
		auto info = base.read_data(layout.size, layout.alignment, static_cast<uint>(base.get_type()));
		list->read_texture(resource, offset, box, sub_resource, info, layout);

		if constexpr (Debug::CheckErrors)
			TEST(base.get_device(), base.get_device().get_device_removed_reason());
		auto result = std::make_shared<std::promise<bool>>();
		base.on_execute_funcs.push_back([result, info, f, layout]()
		{
			f({info.get_cpu_data(), info.size}, layout);
			result->set_value(true);
		});

		base.post_command<false, false>(*this, BarrierSync::COPY);

		return result->get_future();
	}

	std::future<bool> CopyContext::read_texture(const HAL::Resource* resource, UINT sub_resource,
	                                            std::function<void(std::span<std::byte>, texture_layout)> f)
	{
		base.pre_command<false, false>(*this, BarrierSync::COPY);
		base.transition(resource, ResourceStates::COPY_SOURCE);

		auto layout = base.get_device().get_texture_layout(resource->get_desc(), sub_resource);
		auto info = base.read_data(layout.size, layout.alignment, static_cast<uint>(base.get_type()));
		list->read_texture(resource, 0, resource->get_desc().as_texture().get_size(sub_resource), sub_resource, info,
		                   layout);

		if constexpr (Debug::CheckErrors)
			TEST(base.get_device(), base.get_device().get_device_removed_reason());
		auto result = std::make_shared<std::promise<bool>>();
		base.on_execute_funcs.push_back([result, info, f, layout]()
		{
			f({info.get_cpu_data(), info.size}, layout);
			result->set_value(true);
		});

		base.post_command<false, false>(*this, BarrierSync::COPY);

		return result->get_future();
	}

	std::future<bool> CopyContext::read_buffer(Resource* resource, uint64 offset, uint64 size,
	                                           std::function<void(std::span<std::byte>)> f)
	{
		auto result = std::make_shared<std::promise<bool>>();

		if (size == 0)
		{
			f({});
			result->set_value(true);
			return result->get_future();
		}

		base.pre_command<false, false>(*this, BarrierSync::COPY);

		base.transition(resource, ResourceStates::COPY_SOURCE);

		auto info = base.read_data(size, GPUEntityStorageInterface::DEFAULT_ALIGN, static_cast<uint>(base.get_type()));
		list->copy_buffer(info.resource, info.resource_offset, resource, offset, size);
		if constexpr (Debug::CheckErrors)
			TEST(base.get_device(), base.get_device().get_device_removed_reason());
		base.on_execute_funcs.push_back([result, info, f, size]()
		{
			f({reinterpret_cast<std::byte*>(info.get_cpu_data()), size});
			result->set_value(true);
		});

		base.post_command<false, false>(*this, BarrierSync::COPY);

		return result->get_future();
	}

	std::future<bool> CopyContext::read_query(std::shared_ptr<QueryHeap>& query_heap, unsigned int offset,
	                                          unsigned int size, std::function<void(std::span<std::byte>)> f)
	{
		if (size == 0)
		{
			auto result = std::make_shared<std::promise<bool>>();
			f({});
			result->set_value(true);
			return result->get_future();
		}

		auto info = base.read_data(size, GPUEntityStorageInterface::DEFAULT_ALIGN, static_cast<uint>(base.get_type()));
		ASSERT(false);
		if constexpr (Debug::CheckErrors)
			TEST(base.get_device(), base.get_device().get_device_removed_reason());
		auto result = std::make_shared<std::promise<bool>>();
		base.on_execute_funcs.push_back([result, info, f, size]()
		{
			f({reinterpret_cast<std::byte*>(info.get_cpu_data()), size});
			result->set_value(true);
		});
		return result->get_future();
	}

	void CommandList::on_execute()
	{
		for (auto&& t : on_execute_funcs)
			t();

		if (graphics) graphics->on_execute();
		if (compute) compute->on_execute();

		on_execute_funcs.clear();

		Eventer::reset();

		Transitions::on_execute();
		frame_resources = nullptr;

		free_tracked_objects();

		device.context_generator.free(this);
	}


		void Transitions::create_usage_point(BarrierSync operation, bool end)

			{
				auto prev_point = usage_points.empty() ? nullptr : &usage_points.back();
				auto point = &usage_points.emplace_back(type);


				if (prev_point) prev_point->next_point = point;
				point->prev_point = prev_point;
				point->cmd_list = this;
				point->start = !end;
				point->index = static_cast<uint>(usage_points.size());
				point->operation = 	  operation;
				compiler.func_barrier(point);

			///	poin.transition_type = prev_point?
			}

			void Transitions::compile_transitions()
			{
			 if(transitions_compiled) 
				 return;
						std::set<Resource*> non_tracked_resources;


				for (auto& point : usage_points)
				{
					for (auto& usage : point.usages)
					{
						auto prev_usage = usage.prev_usage;

					//	ASSERT(!usage.debug);
						if(!prev_usage) 
						{
							bool is_automatic =  check(usage.resource->get_desc().Flags &HAL::ResFlags::DisableStateTracking);
				//			ASSERT(!);

							if(!is_automatic&usage.resource->get_desc().is_texture())							
							non_tracked_resources.insert(usage.resource);
							continue; //  needs to be synchronized between cmdlists. Can track here what resources needs to be checked
						}
							

						auto prev_state =  prev_usage->wanted_state;

					


						if (prev_state == usage.wanted_state) continue;


						if (prev_state!=ResourceStates::UNKNOWN && usage.prev_usage->prev_usage)
						{
							  ASSERT(prev_state.operation!=BarrierSync::NONE);

						}
						ASSERT(prev_state.is_valid(usage.resource->get_type()));
						ASSERT(usage.wanted_state.is_valid(usage.resource->get_type()));


						auto a = prev_state.get_best_cmd_type();
						auto b = usage.wanted_state.get_best_cmd_type();

						ASSERT(IsCompatible(type, a));
						ASSERT(IsCompatible(type, b));
						BarrierFlags flags = BarrierFlags::NONE;

						if (prev_state==ResourceStates::UNKNOWN)
							flags |= BarrierFlags::DISCARD;

						auto prev_point = prev_usage ? prev_usage->point : nullptr;
						 if(prev_point)prev_point=prev_point->next_point;

						bool can_split = false;// usage.point->cmd_list!=prev_usage->point->cmd_list;//usage.debug;

						if(!point.start /*&& !usage.debug*/) can_split = false; // can split only between work i.e. only from start
						if(prev_point==&point)	   can_split = false; // can't split if it's needed right now
						if(usage.wanted_state==ResourceStates::UNKNOWN)	   can_split = false; // can't split if it's deactivated. probably some new resources will be activated shortly after. we dont know. we do not track it. we dont want to know. thats it. final.						
						if(usage.wanted_state.access==BarrierAccess::NO_ACCESS)	   can_split = false; // resource or it's memory will not be used anymore. There is no end point for that(or is it? Last one in the list?)


					//	if(usage.next_usage&& usage.point->cmd_list==usage.next_usage->point->cmd_list)
						 ASSERT(!(usage.next_usage&&usage.wanted_state.operation==BarrierSync::NONE));

						if (usage.resource->debug_transitions)
							Log::get() << "TRANSITION" << prev_state << " " << usage.wanted_state << "subres: " << usage.subres << Log::endl;
					
						  				HAL::Debug::BarrierBreakpoints::check_usage(usage.resource->name, usage.subres, usage.wanted_state);

						if (can_split)
						{
							ASSERT(prev_point->start);
							auto sync_state = usage.wanted_state;

							sync_state.operation = BarrierSync::SPLIT;

							prev_point->transitions.transition(usage.resource,
								prev_state,
								sync_state,
								usage.subres, flags);


							point.transitions.transition(usage.resource,
								sync_state,
								usage.wanted_state,
								usage.subres, flags);
	
						}
						else
						{
							point.transitions.transition(usage.resource,
								prev_state,
								usage.wanted_state,
								usage.subres, flags);

						}


					}
				}

				transitions_compiled = true;


				for (auto& resource : non_tracked_resources)
				{
					auto& cpu_state = resource->get_state_manager().get_state(this);

					auto transition_one = [&](UINT subres) {

						auto& subres_cpu = cpu_state.get_subres_state(subres);
						ASSERT(subres_cpu.used);



						auto target = ResourceStates::NO_ACCESS;
						target.layout = resource->get_state_manager().initial_layout;

						auto end_point = subres_cpu.last_usage->point->next_point;


						if (resource->debug_transitions)
							Log::get() << "TRANSITION" << subres_cpu.last_usage->wanted_state << " " << target << "subres: " << subres << Log::endl;


						end_point->transitions.transition(resource,
							subres_cpu.last_usage->wanted_state,
							target,
							subres, BarrierFlags::NONE);
					};

					for (int i = 0; i < cpu_state.subres.size(); i++)
						transition_one(i);
				}

			
			}

			HAL::ResourceUsage* Transitions::add_usage(const HAL::Resource* resource, UINT subres, ResourceState state, HAL::TransitionType type)
			{
				HAL::UsagePoint* point = nullptr;

				if (type == HAL::TransitionType::LAST) point = &usage_points.back();
				if (type == HAL::TransitionType::ZERO) point = &usage_points.front();

				HAL::ResourceUsage& usage = point->usages.emplace_back();

				usage.resource = const_cast<HAL::Resource*>(resource);
				usage.subres = subres;
				usage.wanted_state = state;

				HAL::Debug::BarrierBreakpoints::check_usage(resource->name, subres, state);

				if(usage.prev_usage && usage.prev_usage->wanted_state!=ResourceStates::UNKNOWN)
			ASSERT( usage.prev_usage->wanted_state.operation!=BarrierSync::NONE);

				usage.point = point;
				return &usage;
			}

	
		
			void Transitions::transition_present(const HAL::Resource* resource_ptr)
			{

				create_usage_point(BarrierSync::NONE);

				transition(resource_ptr, { BarrierSync::NONE, BarrierAccess::NO_ACCESS, TextureLayout::PRESENT }, ALL_SUBRESOURCES);

				create_usage_point(BarrierSync::NONE,false);
			}


			void Transitions::transition(const ResourceInfo& info, BarrierSync operation )
			{
				if (!info.is_valid()) return;

				ResourceState target_state;//= ResourceState::COMMON;


				if (std::holds_alternative<HAL::Views::ShaderResource>(info.view))
				{
					if (type == CommandListType::DIRECT)
					{
						operation = BarrierSync::ALL_SHADING;// | BarrierSync::DRAW ;
					}

					if (type == CommandListType::COMPUTE)
					{
						operation = BarrierSync::COMPUTE_SHADING;//  ResourceStates::NON_PIXEL_SHADER_RESOURCE;
					}

					target_state = { operation, BarrierAccess::SHADER_RESOURCE, TextureLayout::SHADER_RESOURCE };  //TODO BarrierSync::ALL

				}
				else 	if (std::holds_alternative<HAL::Views::UnorderedAccess>(info.view))
				{
					//ASSERT( operation != BarrierSync::NONE);

					if (type == CommandListType::DIRECT)
					{
						operation = BarrierSync::ALL_SHADING;// | BarrierSync::DRAW ;
					}

					if (type == CommandListType::COMPUTE)
					{
						operation = BarrierSync::COMPUTE_SHADING;//  ResourceStates::NON_PIXEL_SHADER_RESOURCE;
					}

					target_state = { operation, BarrierAccess::UNORDERED_ACCESS, TextureLayout::UNORDERED_ACCESS };  //TODO BarrierSync::ALL
				}
				else 	if (std::holds_alternative<HAL::Views::RenderTarget>(info.view))
				{
					target_state = ResourceStates::RENDER_TARGET;
				}
				else 	if (std::holds_alternative<HAL::Views::DepthStencil>(info.view))
				{
					target_state = ResourceStates::DEPTH_STENCIL;

				}
				else if (std::holds_alternative<HAL::Views::ConstantBuffer>(info.view))
				{
					if (type == CommandListType::DIRECT)
					{
						operation = BarrierSync::ALL_SHADING;// | BarrierSync::DRAW ;
					}

					if (type == CommandListType::COMPUTE)
					{
						operation = BarrierSync::COMPUTE_SHADING;//  ResourceStates::NON_PIXEL_SHADER_RESOURCE;
					}

					target_state = { operation, BarrierAccess::CONSTANT_BUFFER, TextureLayout::UNDEFINED };  //TODO BarrierSync::ALL

				}
				else ASSERT(false);


				info.for_each_subres([&](const HAL::Resource::ptr& resource, UINT subres)
					{
						transition(resource.get(), target_state, subres);
					});
			}

			void Transitions::stop_using(const ResourceInfo& info)
			{
				if (!info.is_valid()) return;


				info.for_each_subres([&](const HAL::Resource::ptr& resource, UINT subres)
					{
						const_cast<HAL::Resource*>(resource.get())->get_state_manager().stop_using(this, subres);
					});
			}


#ifdef PRETRANSITIONS_FIX
	std::shared_ptr<TransitionCommandList> Transitions::fix_pretransitions()
	{
		PROFILE(L"fix_pretransitions");

		HAL::Barriers result(CommandListType::DIRECT);
		
		CommandListType transition_type = type;

		{
			PROFILE(L"processing resources");

		
			for (auto& r : need_check_transitions)
			{
				auto res_type = r->get_state_manager().process_transitions(result, this);

				transition_type = Merge(transition_type, res_type);
			}

		}
		if (result)
		{
				PROFILE(L"creating list");
			auto cmd = static_cast<CommandList*>(this);


			auto transition_list = (static_cast<CommandList*>(this)->get_device().get_queue(transition_type)->get_transition_list());
			transition_list->compiler.set_name(L":Transitions");
			transition_list->create_transition_list(*cmd->frame_resources, result);
			return transition_list;
		}
		return nullptr;
	}
#endif



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

		track_object(*const_cast<Resource*>(resource));


		if (resource->get_heap_type() == HeapType::DEFAULT || resource->get_heap_type() == HeapType::RESERVED)
		{
			use_resource(resource);
			const_cast<Resource*>(resource)->get_state_manager().transition(this, to, subres);
		}
	}


	void Transitions::alias_begin(HAL::Resource* resource)
	{
		resource->get_state(this).alias_ended = false;
		track_object(*resource);
	
		const_cast<Resource*>(resource)->get_state_manager().alias_begin(this);
	}

	void Transitions::alias_end(HAL::Resource* resource)
	{
		track_object(*resource);


		const_cast<Resource*>(resource)->get_state_manager().alias_end(this);
												resource->get_state(this).alias_ended = true;

	}


	void CopyContext::copy_buffer(Resource* dest, uint64 s_dest, Resource* source, uint64 s_source, uint64 size)
	{
		base.pre_command<false, false>(*this, BarrierSync::COPY);

		base.transition(source, ResourceStates::COPY_SOURCE);
		base.transition(dest, ResourceStates::COPY_DEST);


		list->copy_buffer(dest, s_dest, source, s_source, size);
		base.post_command<false, false>(*this, BarrierSync::COPY);
	}

	void CopyContext::copy_resource(Resource* dest, Resource* source)
	{
		base.pre_command<false, false>(*this, BarrierSync::COPY);

		base.transition(source, ResourceStates::COPY_SOURCE);
		base.transition(dest, ResourceStates::COPY_DEST);
		list->copy_resource(dest, source);
		base.post_command<false, false>(*this, BarrierSync::COPY);
	}

	void CopyContext::copy_resource(const Resource::ptr& dest, const Resource::ptr& source)
	{
		copy_resource(dest.get(), source.get());
	}

	void CopyContext::copy_texture(const Resource::ptr& dest, int dest_subres, const Resource::ptr& source,
	                               int source_subres)
	{
		base.pre_command<false, false>(*this, BarrierSync::COPY);

		base.transition(source, ResourceStates::COPY_SOURCE, source_subres);
		base.transition(dest, ResourceStates::COPY_DEST, dest_subres);

		list->copy_texture(dest, dest_subres, source, source_subres);
		if constexpr (Debug::CheckErrors)
			TEST(base.get_device(), base.get_device().get_device_removed_reason());
		base.post_command<false, false>(*this, BarrierSync::COPY);
	}

	void CopyContext::copy_texture(const Resource::ptr& to, ivec3 to_pos, const Resource::ptr& from, ivec3 from_pos,
	                               ivec3 size)
	{
		base.pre_command<false, false>(*this, BarrierSync::COPY);

		base.transition(from, ResourceStates::COPY_SOURCE);
		base.transition(to, ResourceStates::COPY_DEST);
		list->copy_texture(to, to_pos, from, from_pos, size);
		if constexpr (Debug::CheckErrors)
			TEST(base.get_device(), base.get_device().get_device_removed_reason());
		base.post_command<false, false>(*this, BarrierSync::COPY);
	}


	GraphicsContext& CommandList::get_graphics()
	{
		return *graphics.get(); // reinterpret_cast<GraphicsContext&>(*this);
	}

	ComputeContext& CommandList::get_compute()
	{
		return *compute.get(); //return reinterpret_cast<ComputeContext&>(*this);
	}

	CopyContext& CommandList::get_copy()
	{
		return *copy.get(); //return reinterpret_cast<ComputeContext&>(*this);
	}

	void ComputeContext::dispatch_graph(ResourceAddress addr)
	{
		PROFILE_GPU(L"dispatch_graph");
		base.pre_command<true, false>(*this, BarrierSync::COMPUTE_SHADING);
		list->dispatch_graph(addr);
		base.post_command<true, false>(*this, BarrierSync::COMPUTE_SHADING);
	}

	void ComputeContext::set_program(StateObject* id, ResourceAddress buffer, uint size, bool init)
	{
		base.current_pipeline = id;
		if (id->root_signature)
			set_signature(id->root_signature);
		list->set_program(id, buffer, size, init);
	}

	void ComputeContext::on_set_signature(const RootSignature::ptr& s)
	{
		list->set_compute_signature(s);
	}

	void ComputeContext::dispatch(int x, int y, int z)
	{
		PROFILE_GPU(L"Dispatch");

		base.pre_command<true, false>(*this, BarrierSync::COMPUTE_SHADING);
		list->dispatch({x, y, z});
		base.post_command<true, false>(*this, BarrierSync::COMPUTE_SHADING);
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

	
	void ComputeContext::dispatch2(ivec2 a, ivec2 b)
	{
		PROFILE_GPU(L"Dispatch");
		ASSERT(false);
		base.pre_command<true, false>(*this, BarrierSync::COMPUTE_SHADING);
		base.post_command<true, false>(*this, BarrierSync::COMPUTE_SHADING);
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
		start_event(timer->get_block().get_name());

		auto b = static_cast<GPUBlock*>(&timer->get_block());
		if (b)
		{
			gpu_timers.emplace_back(b->get_ptr<GPUBlock>());
			b->start(this);
		}

		current = &timer->get_block();
	}

	void Eventer::on_end(Timer* timer)
	{
		auto b = static_cast<GPUBlock*>(&timer->get_block());
		if (b)
		{
			b->end(this);
		}

		current = timer->get_block().parent;
		end_event();
	}

	thread_local Eventer* Eventer::thread_current = nullptr;

	void Eventer::reset()
	{
		for (auto& block : gpu_timers)
		{
			Profiler::get().on_gpu_timer(std::make_pair<TimedBlock*, GPUTimerInterface*>(block.get(), &block->gpu_timer));
		}
		gpu_timers.clear();
	}

	

	void Eventer::begin(std::wstring_view name)
	{
		ASSERT(!started);
		started = true;
		this->name = name;
		thread_current = this;
		TimedRoot::parent = &Profiler::get();   

		current = Profiler::get().get_current();

		if (type != CommandListType::COPY && name.size())
		{
			ASSERT(!timer);
			timer.reset(new Timer(start(name)));
		}
		else		  
		{
			current = nullptr;
			timer.reset();
	}


	}

	Timer Eventer::start(std::wstring_view name)
	{
		if (Profiler::get().enabled)
			return Timer(std::make_shared<GPUBlock>(name, current, device), this);
		else
			return Timer(nullptr, nullptr);
		//return std::move(Timer(nullptr,nullptr));
	}
	
	void Eventer::end()
	{
		started = false;
		thread_current = nullptr;
		timer.reset();
	}

	void Eventer::start_event(std::wstring_view str)
	{
		compiler.start_event(str);
	}

	void Eventer::end_event()
	{
		compiler.end_event();
	}

	void Eventer::set_marker(const wchar_t* label)
	{
	}


	void GraphicsContext::execute_indirect(IndirectCommand& command_types, UINT max_commands, Resource* command_buffer,
	                                       UINT64 command_offset, Resource* counter_buffer, UINT64 counter_offset)
	{
		ASSERT(command_buffer);
		//	ASSERT(dynamic_cast<PipelineState*>(get_base().current_pipeline));
		PROFILE_GPU(L"execute_indirect");

		bool graphics = dynamic_cast<PipelineState*>(get_base().current_pipeline);

		if (graphics)
			base.pre_command<false, true>(get_base().get_graphics(), BarrierSync::ALL_SHADING, &command_types.slots);
		else
			base.pre_command<true, false>(get_base().get_compute(), BarrierSync::COMPUTE_SHADING, &command_types.slots);

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

		if (graphics)
			base.post_command<false, true>(get_base().get_graphics(), BarrierSync::ALL_SHADING);
		else
			base.post_command<true, false>(get_base().get_compute(), BarrierSync::COMPUTE_SHADING);
	}

	void ComputeContext::execute_indirect(IndirectCommand& command_types, UINT max_commands, Resource* command_buffer,
	                                      UINT64 command_offset, Resource* counter_buffer, UINT64 counter_offset)
	{
		ASSERT(command_buffer);
		PROFILE_GPU(L"execute_indirect");
		base.pre_command<true, false>(*this, BarrierSync::COMPUTE_SHADING);

		ASSERT(command_buffer);

		if (command_buffer) get_base().transition(command_buffer, ResourceStates::INDIRECT_ARGUMENT);
		if (counter_buffer) get_base().transition(counter_buffer, ResourceStates::INDIRECT_ARGUMENT);

		list->execute_indirect(
			command_types,
			max_commands,
			command_buffer,
			command_offset,
			counter_buffer,
			counter_offset);

		base.post_command<true, false>(*this, BarrierSync::COMPUTE_SHADING);
	}

	void ComputeContext::build_ras(const HAL::RaytracingBuildDescStructure& build_desc,
	                               const HAL::RaytracingBuildDescBottomInputs& bottom)
	{
		base.pre_command<false, false>(*this, BarrierSync::BUILD_RAYTRACING_ACCELERATION_STRUCTURE);

		for (auto g : bottom.geometry)
		{
			base.transition(g.IndexBuffer.resource,
			                HAL::ResourceState(BarrierSync::COMPUTE_SHADING, BarrierAccess::SHADER_RESOURCE,
			                                   TextureLayout::UNDEFINED));
			base.transition(g.VertexBuffer.resource, {
				                BarrierSync::COMPUTE_SHADING, BarrierAccess::SHADER_RESOURCE, TextureLayout::UNDEFINED
			                });
			base.transition(g.Transform3x4.resource, {
				                BarrierSync::COMPUTE_SHADING, BarrierAccess::SHADER_RESOURCE, TextureLayout::UNDEFINED
			                });
		}

		base.transition(build_desc.DestAccelerationStructureData.resource, ResourceStates::RAYTRACING_STRUCTURE_WRITE);
		base.transition(build_desc.SourceAccelerationStructureData.resource,
		                ResourceStates::RAYTRACING_STRUCTURE_WRITE);

		base.transition(build_desc.ScratchAccelerationStructureData.resource, {
			                BarrierSync::COMPUTE_SHADING, BarrierAccess::UNORDERED_ACCESS, TextureLayout::UNDEFINED
		                });
		list->build_ras(build_desc, bottom);

		base.post_command<false, false>(*this, BarrierSync::BUILD_RAYTRACING_ACCELERATION_STRUCTURE);
	}


	void ComputeContext::build_ras(const HAL::RaytracingBuildDescStructure& build_desc,
	                               const HAL::RaytracingBuildDescTopInputs& top)
	{
		base.pre_command<false, false>(*this, BarrierSync::BUILD_RAYTRACING_ACCELERATION_STRUCTURE);

		base.transition(build_desc.DestAccelerationStructureData.resource, ResourceStates::RAYTRACING_STRUCTURE_WRITE);
		base.transition(build_desc.SourceAccelerationStructureData.resource,
		                ResourceStates::RAYTRACING_STRUCTURE_WRITE);
		base.transition(build_desc.ScratchAccelerationStructureData.resource, {
			                BarrierSync::COMPUTE_SHADING, BarrierAccess::UNORDERED_ACCESS, TextureLayout::UNDEFINED
		                });

		base.transition(top.instances.resource, {
			                BarrierSync::COMPUTE_SHADING, BarrierAccess::SHADER_RESOURCE, TextureLayout::UNDEFINED
		                });

		list->build_ras(build_desc, top);

		base.post_command<false, false>(*this, BarrierSync::BUILD_RAYTRACING_ACCELERATION_STRUCTURE);
	}

	void ComputeContext::set_const_buffer(UINT i, UINT offset, UINT v)
	{
		list->compute_set_constant(i, offset, v);
	}

	void Transitions::begin()
	{
		transition_count = 0;
		create_usage_point(BarrierSync::NONE, false);
	}

	void Transitions::on_execute()
	{
		usage_points.clear();
		used_resources.clear();
		need_check_transitions.clear();
		transitions_compiled = false;
	}

	void SignatureDataSetter::commit_tables(BarrierSync operation, UsedSlots* slots)
	{
		uint id = 0;
		for (auto& table : tables)
		{
			if (table.dirty)
			{
				for (auto& resource_info : table.resources)
					get_base().transition(*resource_info, operation);

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
						Log::get() << "Possible null slot " << get_slot_name(slot) << " for pipeline " << pipeline->name
							<< Log::endl;
				}
			}
		}
	}

	void SignatureDataSetter::stop_using(uint id)
	{
		auto& table = tables[id];

		for (auto& res : table.resources)
		{
			base.stop_using(*res);
		}
	}


	void SignatureDataSetter::set_pipeline(std::shared_ptr<PipelineStateBase> pipeline)
	{
		{
			std::set<SlotID> diff;

			std::set_difference(used_slots.slots_usage.begin(), used_slots.slots_usage.end(),
			                    pipeline->slots.slots_usage.begin(), pipeline->slots.slots_usage.end(),
			                    std::inserter(diff, diff.begin()));


			for (auto& s : diff)
			{
				auto i = get_table_index(s);
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


	void TransitionCommandList::create_transition_list(FrameResources& frame, const HAL::Barriers& transitions)
	{

		ASSERT(false);
		device.context_generator.generate(this);

		set_proxy(frame.get_storage());


		compiler.reset();
		Eventer::begin(L"Transitions");
		compiler.func([&](auto list)
		{
			list.transitions(transitions);
		});

		Eventer::end();

		proxy->resolve_timers([&, this](const QueryType& type, uint64 from, uint64 to, QueryHeap::ptr heap)
		{
			ASSERT(from == 0);
			resolve_times(heap.get(), static_cast<uint>(to), [heap](std::span<UINT64> data)
			{
				std::copy(data.begin(), data.end(), heap->read_back_data.begin());
			});

			
		});
		
		frame.free_storage(proxy);


		auto ca = frame.get_ca(type);
		compiler.compile(*ca);
		end();

		frame.free_ca(ca);

	}


	TransitionCommandList::TransitionCommandList(CommandListType type, Device& device) : Eventer(device)
	{
		this->type = type;
		compiler.create(type, device);
	}


	// Implementations moved from HAL.CommandList.ixx

	void CommandListBase::free_tracked_objects()
	{
		tracked_resources.clear();
		dstorage_fence = FenceWaiter();
	}

	CommandListType CommandListBase::get_type()
	{
		return type;
	}

	GPUBlock::GPUBlock(std::wstring_view name, TimedBlock* parent, Device& device)
		: TimedBlock(name, parent), device(device)
	{
	}

	Eventer::Eventer(Device& device) : device(device)
	{
	}

	SignatureDataSetter::SignatureDataSetter(CommandList& base) : base(base)
	{
		tables.resize(32); // !!!!!!!!!!!
	}

	void SignatureDataSetter::reset_tables()
	{
		root_sig = nullptr;
		for (auto& table : tables)
		{
			table.const_buffer = CBVHandle();
			table.resources.clear();
			table.descriptors.clear();
			table.dirty = false;
		}
	}

	void SignatureDataSetter::reset()
	{
		used_slots.clear();
		tables.clear();
		tables.resize(32);
	}

	CommandList& SignatureDataSetter::get_base()
	{
		return base;
	}

	void SignatureDataSetter::set_signature(const RootSignature::ptr& signature)
	{
		if (root_sig == signature) return;

		root_sig = signature;

		auto& desc = signature->get_desc();

		on_set_signature(signature);
	}

	void SignatureDataSetter::set_signature(Layouts layout)
	{
		set_signature(base.get_device().get_engine_root_layout_holder().GetSignature(layout));
	}

	void SignatureDataSetter::set_cb(UINT index, const CBVHandle& cb, BarrierSync operation)
	{
		get_base().transition(cb.get_resource_info(), operation);
		set_const_buffer(index, 0, cb.get_offset());
	}

	GraphicsContext::GraphicsContext(CommandList& base)
		: SignatureDataSetter(base), list(base.get_native_list())
	{
	}

	std::vector<HAL::Format> GraphicsContext::get_formats() const
	{
		return compiled_rt.get_formats();
	}

	CommandList& GraphicsContext::get_base()
	{
		return base;
	}

	void GraphicsContext::set_topology(HAL::PrimitiveTopologyType topology, HAL::PrimitiveTopologyFeed feedType, bool adjusted, uint controlpoints)
	{
		list->set_topology(topology, feedType, adjusted, controlpoints);
	}

	void GraphicsContext::set_stencil_ref(UINT ref)
	{
		list->set_stencil_ref(ref);
	}

	std::vector<Viewport> GraphicsContext::get_viewports()
	{
		return viewports;
	}

	void GraphicsContext::set_index_buffer(HAL::Views::IndexBuffer view)
	{
		index = view;
	}

	ComputeContext::ComputeContext(CommandList& base)
		: SignatureDataSetter(base), list(base.get_native_list())
	{
	}

	CommandList& ComputeContext::get_base()
	{
		return base;
	}

	CommandListType TransitionCommandList::get_type()
	{
		return type;
	}

	void TransitionCommandList::end()
	{
	}

	void TransitionCommandList::on_execute()
	{
		for (auto&& t : on_execute_funcs)
			t();

		on_execute_funcs.clear();

		Eventer::reset();

		device.context_generator.free(this);
	}

	void CommandList::update_tilings(HAL::update_tiling_info&& info)
	{
		tile_updates.emplace_back(std::move(info));
		track_object(*(info.resource));
	}

	void CommandList::clear_uav(const UAVHandle& h, vec4 ClearColor)
	{
		create_usage_point(BarrierSync::CLEAR_UNORDERED_ACCESS_VIEW);
		transition(h.get_resource_info(), BarrierSync::CLEAR_UNORDERED_ACCESS_VIEW);
		compiler.clear_uav(h, ClearColor);
		create_usage_point(BarrierSync::CLEAR_UNORDERED_ACCESS_VIEW, false);
	}

	const std::vector<CommandRecord>& CommandList::get_debug_records() const
	{
		return compiler.get_debug_records();
	}

	DelayedCommandList* CommandListBase::get_native_list()
	{
		return &compiler;
	}

	HAL::UsagePoint* Transitions::get_last_usage_point()
	{
		return &usage_points.back();
	}

	CopyContext::CopyContext(CommandList& base) : base(base), list(base.get_native_list()) {}

	const API::CommandList& TransitionCommandList::get_compiled() const { return compiler.get_list(); }
}
