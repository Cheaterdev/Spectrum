module HAL:CommandList;


import :Autogen;

import :Device;
import Core;

import HAL;

// File-local template — full HAL::Resource available here, no std::function overhead.
//
// Reports each resource a view touches ONCE, with the SubresRange it covers,
// rather than calling back per subresource. A view already IS a range -- the
// mip/array/plane fields below are the same six numbers -- so shredding it here
// only to have the caller (and formerly the backend) put it back together was
// wasted work: a 256-slice view meant 256 callbacks, 256 shared_ptr passes and
// 256 identical resource comparisons in the caller's filter.
//
// SubresRange::all() is still preferred wherever the view covers the whole
// resource: D3D12 has a dedicated sentinel for that and it stays cheaper than an
// equivalent range all the way down.
template<typename F>
static void visit_subres(const HAL::ResourceInfo& info, F&& f)
{
	using HAL::SubresRange;

	std::visit(overloaded{
		[&](const HAL::Views::ShaderResource& v) {
			std::visit(overloaded{
				[&](const HAL::Views::ShaderResource::Buffer&) { f(v.Resource, SubresRange::all()); },
				[&](const HAL::Views::ShaderResource::Texture2D& t) {
					auto& desc = v.Resource->get_desc().as_texture();
					if (t.MipLevels == desc.MipLevels && t.MostDetailedMip == 0 && desc.is2D())
						f(v.Resource, SubresRange::all());
					else
						f(v.Resource, SubresRange{ t.MostDetailedMip, t.MipLevels, 0, 1, t.PlaneSlice, 1 });
				},
				[&](const HAL::Views::ShaderResource::Texture2DArray& t) {
					auto& desc = v.Resource->get_desc().as_texture();
					if (t.MipLevels == desc.MipLevels && t.MostDetailedMip == 0 && t.FirstArraySlice == 0 && t.ArraySize == desc.ArraySize && desc.is2D())
						f(v.Resource, SubresRange::all());
					else
						f(v.Resource, SubresRange{ t.MostDetailedMip, t.MipLevels, t.FirstArraySlice, t.ArraySize, t.PlaneSlice, 1 });
				},
				[&](const HAL::Views::ShaderResource::Texture3D& t) {
					auto& desc = v.Resource->get_desc().as_texture();
					if (t.MipLevels == desc.MipLevels && t.MostDetailedMip == 0)
						f(v.Resource, SubresRange::all());
					else
						f(v.Resource, SubresRange{ t.MostDetailedMip, t.MipLevels, 0, 1, 0, 1 });
				},
				[&](const HAL::Views::ShaderResource::Cube& t) {
					auto& desc = v.Resource->get_desc().as_texture();
					if (t.MipLevels == desc.MipLevels && t.MostDetailedMip == 0)
						f(v.Resource, SubresRange::all());
					else
						f(v.Resource, SubresRange{ t.MostDetailedMip, t.MipLevels, 0, 6, 0, 1 });
				},
				[&](const HAL::Views::ShaderResource::Raytracing&) { f(v.Resource, SubresRange::all()); },
				[&](auto) { ASSERT(false); }
			}, v.View);
		},
		[&](const HAL::Views::UnorderedAccess& v) {
			std::visit(overloaded{
				[&](const HAL::Views::UnorderedAccess::Buffer& b) {
					f(v.Resource, SubresRange::all());
					if (b.CounterResource) f(b.CounterResource, SubresRange::all());
				},
				[&](const HAL::Views::UnorderedAccess::Texture2D& t) {
					auto& desc = v.Resource->get_desc().as_texture();
					if (desc.MipLevels == 1 && desc.is2D())
						f(v.Resource, SubresRange::all());
					else
						f(v.Resource, SubresRange{ t.MipSlice, 1, 0, 1, t.PlaneSlice, 1 });
				},
				[&](const HAL::Views::UnorderedAccess::Texture2DArray& t) {
					auto& desc = v.Resource->get_desc().as_texture();
					if (desc.MipLevels == 1 && desc.is2D() && desc.ArraySize == 1)
						f(v.Resource, SubresRange::all());
					else
						f(v.Resource, SubresRange{ t.MipSlice, 1, t.FirstArraySlice, t.ArraySize, t.PlaneSlice, 1 });
				},
				[&](const HAL::Views::UnorderedAccess::Texture3D& t) {
					auto& desc = v.Resource->get_desc().as_texture();
					if (t.FirstWSlice == 0 && t.WSize == desc.Dimensions.z && desc.MipLevels == 1)
						f(v.Resource, SubresRange::all());
					else
						f(v.Resource, SubresRange{ t.MipSlice, 1, 0, 1, 0, 1 });
				},
				[&](auto) { ASSERT(false); }
			}, v.View);
		},
		[&](const HAL::Views::RenderTarget& v) {
			std::visit(overloaded{
				[&](const HAL::Views::RenderTarget::Texture2D& t) {
					auto& desc = v.Resource->get_desc().as_texture();
					if (desc.MipLevels == 1 && desc.is2D() == 1)
						f(v.Resource, SubresRange::all());
					else
						f(v.Resource, SubresRange{ t.MipSlice, 1, 0, 1, t.PlaneSlice, 1 });
				},
				[&](const HAL::Views::RenderTarget::Texture2DArray& t) {
					f(v.Resource, SubresRange{ t.MipSlice, 1, t.FirstArraySlice, t.ArraySize, t.PlaneSlice, 1 });
				},
				[&](auto) { ASSERT(false); }
			}, v.View);
		},
		[&](const HAL::Views::DepthStencil& v) {
			std::visit(overloaded{
				[&](const HAL::Views::DepthStencil::Texture2D& t) {
					f(v.Resource, SubresRange{ t.MipSlice, 1, 0, 1, 0, 1 });
				},
				[&](const HAL::Views::DepthStencil::Texture2DArray& t) {
					f(v.Resource, SubresRange{ t.MipSlice, 1, t.FirstArraySlice, t.ArraySize, 0, 1 });
				},
				[&](auto) { ASSERT(false); }
			}, v.View);
		},
		[&](const HAL::Views::ConstantBuffer& v) { f(v.Resource, SubresRange::all()); },
		[&](auto) { ASSERT(false); }
	}, info.view);
}

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

		if (type == CommandListType::DIRECT || is_compute_queue(type))
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

				block << "DEBUG(" << name.ptr << "): " << pso_name << "\n";
				for (int i = 0; i < 3; i++)
				{
					block << "debug(" << i << "): " << result[i].v.x << " " << result[i].v.y << " " << result[i].v.z << " "
						<< result[i].v.w << " " << "\n";
				}
				Log::get() << block;
			});

		get_compute().clear(debug_buffer);
	}


	void CommandList::begin(LiteralWStr name)
	{
		active = true;
		compiler.set_name(name.ptr);

		compiler.reset();
#ifdef DEV
		begin_stack = Exceptions::get_stack_trace();
#endif

		device.context_generator.generate(this);
		first_debug_log = true;

		if (!frame_resources)
		{
			frame_resources = device.get_frame_manager().begin_frame();
			owns_frame_resources = true;
		}
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

	void CommandList::invalidate_state()
	{
		current_pipeline = nullptr;
		if (graphics) { graphics->invalidate_signature(); graphics->invalidate_tables(); }
		if (compute)  { compute->invalidate_signature();  compute->invalidate_tables();  }

		// Record time, so this pushes a real Cmd (replayed in sequence right
		// after the func() call that prompted this) — not a live D3D12 call.
		compiler.set_descriptor_heaps(
			device.get_descriptor_heap_factory().get_cbv_srv_uav_heap().get(),
			device.get_descriptor_heap_factory().get_sampler_heap().get()
		);
	}


	void CommandList::end()
	{
		current_pipeline = nullptr;

		// Reserve the last operation's barriers_after point -- nothing follows
		// it to trigger the close in begin_op.
		end_op();

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

		// If this command list created its own frame_resources (nobody else shares it,
		// e.g. FrameGraph::start_list()), it's the sole owner and commits now. Shared
		// frame_resources (FrameGraph's current_frame) are committed once by the owner
		// after all passes for the frame have been compiled.
		if (owns_frame_resources)
			frame_resources->commit_descriptors_to_gpu();

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

	// Compile a standalone list -- one that is not part of a FrameGraph batch.
	// It still goes through a group (of one), because that is what computes the
	// barriers and then replays the list; running either half directly here
	// would be a second, independent computation over the same command stream.
	void Sendable::compile()
	{
		if (active)
			end();

		CommandListGroup group;
		group.add(dynamic_cast<CommandList*>(this)->get_ptr());
		group.plan_resources();
		group.compile_transitions();
		group.compile();
	}

	FenceWaiter Sendable::execute(std::function<void()> f)
	{
		if (!compiler.is_compiled())
			Sendable::compile();

		if (f)
			on_execute_funcs.emplace_back(f);


		CommandListGroup group;
		group.add(dynamic_cast<CommandList*>(this)->get_ptr());
		auto fence = dynamic_cast<CommandList*>(this)->get_device().get_queue(type)->execute(group);

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
		const Handles::RTV& table_rtv = rt.table_rtv;
		const Handles::DSV& table_dsv = rt.table_dsv;

		// Changing the render target ends the batch: see Transitions::break_op.
		// Before pre_command, so the begin_op inside it is the call that opens
		// the new operation and stamps it with the right class -- the RTV
		// usages and clears recorded below then land in that new operation
		// rather than in the run being closed.
		get_base().break_op();

		{ PROFILE(L"pre_command"); base.pre_command<false, false>(*this, BarrierSync::DRAW); }

		{ PROFILE(L"rt_transitions");
		for (uint i = 0; i < table_rtv.get_count(); i++)
			get_base().add_resource_usage(table_rtv[i].get_resource_info());
		if (table_dsv)
			get_base().add_resource_usage(table_dsv.get_resource_info());
		}

		if (check(options & RTOptions::SetHandles))
			list->set_rtv(table_rtv.get_count(), table_rtv, table_dsv);

		if (check(options & RTOptions::ClearColor))
		{
			for (uint i = 0; i < table_rtv.get_count(); i++)
				list->clear_rtv(table_rtv[i], clear_color);
		}

		if (table_dsv && check(options & (RTOptions::ClearStencil | RTOptions::ClearDepth)))
			list->clear_depth_stencil(table_dsv[0], check(options & RTOptions::ClearDepth),
			                          check(options & RTOptions::ClearStencil), depth, stencil);

		{ PROFILE(L"post_command"); base.post_command<false, false>(*this, BarrierSync::DRAW); }
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

		get_base().add_resource_usage(index.Resource, ResourceStates::INDEX_BUFFER);
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


	void GraphicsContext::clear_depth_rects(std::vector<sizer_long> rects, float depth)
	{
		if (compiled_rt.table_dsv)
			list->clear_depth_rects(compiled_rt.table_dsv[0], depth, rects);
	}

	void GraphicsContext::set_scissors(sizer_long rect)
	{
		list->set_scissors(rect);
	}

	void GraphicsContext::set_scissors(std::vector<sizer_long> rects)
	{
		list->set_scissors(rects);
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

		base.add_resource_usage(resource, ResourceStates::COPY_DEST);

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

		base.add_resource_usage(resource, ResourceStates::COPY_DEST, sub_resource);
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

		base.add_resource_usage(resource, ResourceStates::COPY_SOURCE, sub_resource);

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
		base.add_resource_usage(resource, ResourceStates::COPY_SOURCE, sub_resource);

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

		base.add_resource_usage(resource, ResourceStates::COPY_SOURCE);

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


			// Grow this list's operation sequence: a run of same-class work stays
			// ONE CmdListOperation, so only a change of class appends a new one.
			//
			// Reserves the barrier points as it goes, which is what fixes their
			// POSITION in the command stream (the groups themselves are filled
			// in later): the outgoing operation's barriers_after closes right
			// after its last command, then the new operation's barriers_before
			// opens immediately ahead of its first. Recording a list therefore
			// produces
			//     [op0.before] op0 cmds [op0.after] [op1.before] op1 cmds ...
			void Transitions::begin_op(BarrierSync op)
			{
				// Counts commands, not operations -- a run of same-class work
				// merges below but each call is still a distinct dispatch/draw.
				op_step++;

				if (!force_new_op && !operations.empty() && operations.back().type == op)
					return;                                      // same class -> keep growing

				force_new_op = false;

				end_op();

				auto& operation = operations.emplace_back(type, op, static_cast<uint>(operations.size()));
				compiler.func_barrier(&operation.barriers_before, operation.index, false, operation.type);
				op_first_step = op_step;
			}

			void Transitions::split_op()
			{
				if (operations.empty()) return;

				const BarrierSync op = operations.back().type;

				end_op();

				auto& operation = operations.emplace_back(type, op, static_cast<uint>(operations.size()));
				compiler.func_barrier(&operation.barriers_before, operation.index, false, operation.type);
				op_first_step = op_step;
			}

			void Transitions::break_op()
			{
				// Nothing recorded into the current operation yet -- it already
				// IS the fresh one the caller wants, and breaking would strand
				// an empty operation with two reserved barrier points.
				if (operations.empty() || op_step == op_first_step) return;

				// DEFERRED, not an immediate split: the new operation's class
				// must come from the next command, not from the one being
				// closed. set_pipeline runs BEFORE the following draw's
				// pre_command, so splitting here would create an operation of
				// the OUTGOING class -- switching a compute run to a graphics
				// one would leave an empty COMPUTE operation behind. Letting
				// begin_op honour the flag makes it pick the right class.
				force_new_op = true;
			}

			// Close the operation currently at the back, reserving the point its
			// barriers_after will be emitted at. Idempotent-by-position: only the
			// transition from one operation to the next (or to end of list) calls
			// it, so a group is never reserved twice.
			void Transitions::end_op()
			{
				if (operations.empty()) return;

				// Idempotent: reserving the same barriers_after twice emits that
				// group twice, and the second run's LayoutBefore is stale by
				// definition. CommandList::end() closes the final operation, and
				// anything appending afterwards (process_transitions) would
				// otherwise close it a second time.
				auto& operation = operations.back();
				if (operation.closed) return;
				operation.closed = true;

				compiler.func_barrier(&operation.barriers_after, operation.index, true, operation.type);
			}

			void Transitions::transition_present(const HAL::Resource* resource_ptr)
			{
				begin_op(BarrierSync::NONE);
				add_resource_usage(resource_ptr, { BarrierSync::NONE, BarrierAccess::NO_ACCESS, TextureLayout::PRESENT }, ALL_SUBRESOURCES);
			}


			void Transitions::begin_external_op(BarrierSync op)
			{
				// begin_op without its merge check: a run of same-class work
				// merges by design, but external work must be its own operation
				// or its barriers would be hoisted in front of unrelated commands
				// that happen to share the class.
				end_op();

				auto& operation = operations.emplace_back(type, op, static_cast<uint>(operations.size()));
				compiler.func_barrier(&operation.barriers_before, operation.index, false, operation.type);
			}

			void Transitions::transition_to(const HAL::Resource* resource, ResourceState state)
			{
				// split_op, not begin_op: begin_op would merge into the last
				// operation whenever the classes happen to match, and the target
				// state would then be merged with that operation's real use
				// instead of following it.
				if (operations.empty())
					begin_op(BarrierSync::NONE);
				else
					split_op();

				add_resource_usage(resource, state, ALL_SUBRESOURCES);

				// Close it here: this runs after CommandList::end() has already
				// closed the list, so nothing else will reserve this operation's
				// barriers_after, and any resource whose last use lands in it
				// would lose its trailing barriers. end_op() is idempotent, so a
				// later split_op() on the same list stays correct.
				end_op();
			}

			void Transitions::transition_to_rest(const HAL::Resource* resource)
			{
				transition_to(resource, state_at_rest(resting_layout(resource)));
			}


			void Transitions::add_resource_usage(const ResourceInfo& info, BarrierSync operation, bool whole_resource )
			{
				if (!info.is_valid()) return;
				ResourceState target_state;//= ResourceState::COMMON;


				if (std::holds_alternative<HAL::Views::ShaderResource>(info.view))
				{
					if (type == CommandListType::DIRECT)
					{
						operation = BarrierSync::ALL_SHADING;// | BarrierSync::DRAW ;
					}

					if (is_compute_queue(type))
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

					if (is_compute_queue(type))
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

					if (is_compute_queue(type))
					{
						operation = BarrierSync::COMPUTE_SHADING;//  ResourceStates::NON_PIXEL_SHADER_RESOURCE;
					}

					target_state = { operation, BarrierAccess::CONSTANT_BUFFER, TextureLayout::UNDEFINED };  //TODO BarrierSync::ALL

				}
				else ASSERT(false);

				auto resource= 		 info.get_resource();

		if (!resource->frame_graph_managed)
			track_object(*const_cast<Resource*>(resource));

		if (resource->get_heap_type() == HeapType::DEFAULT || resource->get_heap_type() == HeapType::RESERVED)
		{
			use_resource(resource);

			// Record the VIEW, not its expanded subresource list. The view
			// already names its mip/array/plane range, so expanding it here
			// would pay a per-subresource cost on every bind for information
			// that can be recovered later, once, only for resources that
			// actually need a barrier.
			//
			// [Barrier = ALL] drops the view entirely and records a bare
			// whole-resource use: there is then no range to expand at all, so
			// the resource stays on compile_transitions' single
			// current[ALL_SUBRESOURCES] entry instead of diverging into one
			// per slice.
			record_usage(resource, whole_resource
				? HAL::OperationUsage(ALL_SUBRESOURCES, target_state)
				: HAL::OperationUsage(&info, target_state));
		}
			}

	void Transitions::add_resource_usage(const Resource::ptr& resource, ResourceState to, UINT subres)
	{
		add_resource_usage(resource.get(), to, subres);
	}

	void Transitions::use_resource(const Resource* resource)
	{
		auto& state = const_cast<Resource*>(resource)->get_state(this);
		if (!state.listed)
		{
			state.listed = true;
			used_resources.emplace_back(const_cast<Resource*>(resource));
		}
	}

	// Append one use of `resource` to the operation currently being recorded.
	// Cheap by construction: a lookup of this resource's per-list state and a
	// push_back. No state comparison, no barrier decision, no subresource
	// expansion -- all of that happens later, once, over the recorded
	// operations.
	void Transitions::record_usage(const Resource* resource, const HAL::OperationUsage& usage)
	{
		// A usage always belongs to an operation. Paths that transition a
		// resource without issuing GPU work through pre_command -- the upload
		// flush, and other direct add_resource_usage callers -- never open one,
		// so open it here. Without this the usage is recorded against operation
		// 0 while `operations` stays empty, and the barrier pass indexes off the
		// end of it.
		if (operations.empty())
			begin_op(BarrierSync::NONE);

		uint op_index = operations.back().index;

		auto& state = const_cast<Resource*>(resource)->get_state(this);

		// Unordered access is the one case where re-touching a resource inside
		// a single operation is a real hazard: two dispatches writing the same
		// UAV (or one writing and the next reading it) must be ordered, and the
		// state does not change to signal it. Render-target and depth writes are
		// ordered by the pipeline, so they are deliberately not included.
		//
		// `step` is what separates "bound twice for the same dispatch" -- which
		// is fine, those binds merge -- from "touched again by a later dispatch
		// in the same run", which needs the operation split so a barrier can sit
		// between the two halves.
		const bool uav_involved = check(usage.state.access & BarrierAccess::UNORDERED_ACCESS);

		for (const auto& prev : state.operations[op_index])
		{
			if (prev.step == op_step) continue;   // same dispatch, just another bind
			if (!uav_involved && !check(prev.state.access & BarrierAccess::UNORDERED_ACCESS)) continue;

			split_op();
			op_index = operations.back().index;
			break;
		}

		auto& usages = state.operations[op_index];
		usages.push_back(usage);
		usages.back().step = op_step;
	}

	void Transitions::add_resource_usage(const Resource* resource, ResourceState to, UINT subres)
	{
		if (!resource) return;

		if (!resource->frame_graph_managed)
			track_object(*const_cast<Resource*>(resource));

		if (resource->get_heap_type() == HeapType::DEFAULT || resource->get_heap_type() == HeapType::RESERVED)
		{
			use_resource(resource);

			// No view here -- the subresource is named directly (copies, and
			// anything else that transitions a resource without binding it).
			record_usage(resource, HAL::OperationUsage(subres, to));
		}
	}


	void Transitions::set_entry_state(const HAL::Resource* resource, ResourceState state)
	{
		auto& tracked = const_cast<Resource*>(resource)->get_state(this);
		tracked.has_entry_state = true;
		tracked.entry_state     = state;
	}

	std::optional<ResourceState> Transitions::get_exit_state(const HAL::Resource* resource) const
	{
		auto& tracked = const_cast<Resource*>(resource)->get_state(const_cast<Transitions*>(this));
		if (tracked.operations.empty()) return std::nullopt;

		// operations is ordered by index, so the last entry is the last
		// operation, and its last usage is the one that leaves the resource
		// where it ends up.
		const auto& usages = tracked.operations.rbegin()->second;
		if (usages.empty()) return std::nullopt;

		return usages.back().state;
	}

	std::optional<ResourceState> Transitions::get_first_use_state(const HAL::Resource* resource) const
	{
		auto& tracked = const_cast<Resource*>(resource)->get_state(const_cast<Transitions*>(this));
		if (tracked.operations.empty()) return std::nullopt;

		// Mirror of get_exit_state: ordered map, so the FIRST entry is the
		// earliest operation, and its first usage is the state this list needs
		// the resource to already be in when it starts.
		const auto& usages = tracked.operations.begin()->second;
		if (usages.empty()) return std::nullopt;

		return usages.front().state;
	}

	void Transitions::alias_begin(HAL::Resource* resource)
	{
		resource->get_state(this).alias_ended = false;
		track_object(*resource);

		// The aliased-into resource has no meaningful contents -- its first use
		// is entered from UNKNOWN with a discard. compile_transitions already
		// does that for a virgin resource, so re-arm the flag here.
		//
		// BOTH flags, because they gate different halves of that and only
		// re-arming `virgin` produced a barrier from UNDEFINED with no DISCARD:
		// `virgin` decides the before-state (from_undefined), while
		// `initialized` decides whether the first write carries the discard.
		// Left initialized, the resource claims UNDEFINED and then preserves
		// contents that are another resource's memory.
		resource->virgin = true;
		resource->initialized = false;
	}

	void Transitions::alias_end(HAL::Resource* resource)
	{
		track_object(*resource);


		// End-of-aliasing barriers belong in the operation's barriers_after --
		// that group is reserved and waiting, not wired up yet.
		resource->get_state(this).alias_ended = true;

	}


	void CopyContext::copy_buffer(Resource* dest, uint64 s_dest, Resource* source, uint64 s_source, uint64 size)
	{
		base.pre_command<false, false>(*this, BarrierSync::COPY);

		base.add_resource_usage(source, ResourceStates::COPY_SOURCE);
		base.add_resource_usage(dest, ResourceStates::COPY_DEST);


		list->copy_buffer(dest, s_dest, source, s_source, size);
		base.post_command<false, false>(*this, BarrierSync::COPY);
	}

	void CopyContext::copy_resource(Resource* dest, Resource* source)
	{
		base.pre_command<false, false>(*this, BarrierSync::COPY);

		base.add_resource_usage(source, ResourceStates::COPY_SOURCE);
		base.add_resource_usage(dest, ResourceStates::COPY_DEST);
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

		base.add_resource_usage(source, ResourceStates::COPY_SOURCE, source_subres);
		base.add_resource_usage(dest, ResourceStates::COPY_DEST, dest_subres);

		list->copy_texture(dest, dest_subres, source, source_subres);
		if constexpr (Debug::CheckErrors)
			TEST(base.get_device(), base.get_device().get_device_removed_reason());
		base.post_command<false, false>(*this, BarrierSync::COPY);
	}

	void CopyContext::copy_texture(const Resource::ptr& to, ivec3 to_pos, const Resource::ptr& from, ivec3 from_pos,
	                               ivec3 size)
	{
		base.pre_command<false, false>(*this, BarrierSync::COPY);

		base.add_resource_usage(from, ResourceStates::COPY_SOURCE);
		base.add_resource_usage(to, ResourceStates::COPY_DEST);
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
		PROFILE_GPU(L"dispatch");

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
			// A PSO change ends the batch, for the same reason set_rtv does: one
			// operation gets ONE set of entry barriers, so every resource it
			// touches must hold a single state across the whole run. Two draws
			// with different pipelines bind different resources for different
			// purposes, and merging them means a resource written by the first
			// and read by the second collapses to whichever use came last.
			//
			// Only fires on a real change (this branch) and only when the
			// current operation has recorded something (break_op's own guard),
			// so redundant set_pipeline calls and a pass's opening bind are
			// free.
			break_op();

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

	

	void Eventer::begin(LiteralWStr name)
	{
		ASSERT(!started);
		started = true;
		this->name = name;
		thread_current = this;
		TimedRoot::parent = &Profiler::get();   

		current = Profiler::get().get_current();

		if (type != CommandListType::COPY)
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

	Timer Eventer::start(LiteralWStr name)
	{
		if (Profiler::get().enabled)
			return Timer(std::make_shared<GPUBlock>(name, current, device), this);
		else
			return Timer();
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

		{ PROFILE(L"pre_command");
		  if (graphics)
			  base.pre_command<false, true>(get_base().get_graphics(), BarrierSync::ALL_SHADING, &command_types.slots);
		  else
			  base.pre_command<true, false>(get_base().get_compute(), BarrierSync::COMPUTE_SHADING, &command_types.slots); }

		{ PROFILE(L"transitions");
		  if (command_buffer) get_base().add_resource_usage(command_buffer, ResourceStates::INDIRECT_ARGUMENT);
		  if (counter_buffer) get_base().add_resource_usage(counter_buffer, ResourceStates::INDIRECT_ARGUMENT);
		  get_base().add_resource_usage(static_cast<HAL::Resource*>(index.Resource.get()), ResourceStates::INDEX_BUFFER); }

		list->set_index_buffer(index);

		list->execute_indirect(
			command_types,
			max_commands,
			command_buffer,
			command_offset,
			counter_buffer,
			counter_offset);

		{ PROFILE(L"post_command");
		  if (graphics)
			  base.post_command<false, true>(get_base().get_graphics(), BarrierSync::ALL_SHADING);
		  else
			  base.post_command<true, false>(get_base().get_compute(), BarrierSync::COMPUTE_SHADING); }
	}

	void ComputeContext::execute_indirect(IndirectCommand& command_types, UINT max_commands, Resource* command_buffer,
	                                      UINT64 command_offset, Resource* counter_buffer, UINT64 counter_offset)
	{
		ASSERT(command_buffer);
		PROFILE_GPU(L"execute_indirect");

		{ PROFILE(L"pre_command");
		  base.pre_command<true, false>(*this, BarrierSync::COMPUTE_SHADING); }

		{ PROFILE(L"transitions");
		  if (command_buffer) get_base().add_resource_usage(command_buffer, ResourceStates::INDIRECT_ARGUMENT);
		  if (counter_buffer) get_base().add_resource_usage(counter_buffer, ResourceStates::INDIRECT_ARGUMENT); }

		list->execute_indirect(
			command_types,
			max_commands,
			command_buffer,
			command_offset,
			counter_buffer,
			counter_offset);

		{ PROFILE(L"post_command");
		  base.post_command<true, false>(*this, BarrierSync::COMPUTE_SHADING); }
	}

	void ComputeContext::build_ras(const HAL::RaytracingBuildDescStructure& build_desc,
	                               const HAL::RaytracingBuildDescBottomInputs& bottom)
	{
		base.pre_command<false, false>(*this, BarrierSync::BUILD_RAYTRACING_ACCELERATION_STRUCTURE);

		for (auto g : bottom.geometry)
		{
			base.add_resource_usage(g.IndexBuffer.resource,
			                HAL::ResourceState(BarrierSync::COMPUTE_SHADING, BarrierAccess::SHADER_RESOURCE,
			                                   TextureLayout::UNDEFINED));
			base.add_resource_usage(g.VertexBuffer.resource, {
				                BarrierSync::COMPUTE_SHADING, BarrierAccess::SHADER_RESOURCE, TextureLayout::UNDEFINED
			                });
			base.add_resource_usage(g.Transform3x4.resource, {
				                BarrierSync::COMPUTE_SHADING, BarrierAccess::SHADER_RESOURCE, TextureLayout::UNDEFINED
			                });
		}

		base.add_resource_usage(build_desc.DestAccelerationStructureData.resource, ResourceStates::RAYTRACING_STRUCTURE_WRITE);
		base.add_resource_usage(build_desc.SourceAccelerationStructureData.resource,
		                ResourceStates::RAYTRACING_STRUCTURE_WRITE);

		base.add_resource_usage(build_desc.ScratchAccelerationStructureData.resource, {
			                BarrierSync::COMPUTE_SHADING, BarrierAccess::UNORDERED_ACCESS, TextureLayout::UNDEFINED
		                });
		list->build_ras(build_desc, bottom);

		base.post_command<false, false>(*this, BarrierSync::BUILD_RAYTRACING_ACCELERATION_STRUCTURE);
	}


	void ComputeContext::build_ras(const HAL::RaytracingBuildDescStructure& build_desc,
	                               const HAL::RaytracingBuildDescTopInputs& top)
	{
		base.pre_command<false, false>(*this, BarrierSync::BUILD_RAYTRACING_ACCELERATION_STRUCTURE);

		base.add_resource_usage(build_desc.DestAccelerationStructureData.resource, ResourceStates::RAYTRACING_STRUCTURE_WRITE);
		base.add_resource_usage(build_desc.SourceAccelerationStructureData.resource,
		                ResourceStates::RAYTRACING_STRUCTURE_WRITE);
		base.add_resource_usage(build_desc.ScratchAccelerationStructureData.resource, {
			                BarrierSync::COMPUTE_SHADING, BarrierAccess::UNORDERED_ACCESS, TextureLayout::UNDEFINED
		                });

		base.add_resource_usage(top.instances.resource, {
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
		operations.clear();
		op_step = 0;
		tracked_resources.reserve(512);
		used_resources.reserve(256);
	}

	void Transitions::on_execute()
	{
		operations.clear();
		op_step = 0;
		used_resources.clear();
	}

	void SignatureDataSetter::commit_tables(BarrierSync operation, UsedSlots* slots)
	{
		PROFILE(L"commit_tables");
		uint id = 0;
		for (auto& table : tables)
		{
			if (table.dirty)
			{
				{
					PROFILE(L"transitions");
					for (auto& bound : table.resources)
						get_base().add_resource_usage(*bound.info, operation, bound.whole_resource);
				}
				{
					PROFILE(L"set_cb");
					set_cb(id, table.const_buffer, operation);
				}

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
			   /*
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

		frame.free_ca(ca);*/

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

	GPUBlock::GPUBlock(LiteralWStr name, TimedBlock* parent, Device& device)
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
			table.const_buffer = Handles::CBV();
			table.resources.clear();
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

	void SignatureDataSetter::set_cb(UINT index, const Handles::CBV& cb, BarrierSync operation)
	{
		get_base().add_resource_usage(cb.get_resource_info(), operation);
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

	void CommandList::clear_uav(const Handles::UAV& h, vec4 ClearColor, bool whole_resource)
	{
		begin_op(BarrierSync::CLEAR_UNORDERED_ACCESS_VIEW);
		add_resource_usage(h.get_resource_info(), BarrierSync::CLEAR_UNORDERED_ACCESS_VIEW, whole_resource);
		compiler.clear_uav(h, ClearColor);
	}

	void CommandList::clear_dsv(const Handles::DSV& h, bool clear_depth, bool clear_stencil, float depth, UINT8 stencil,
	                            bool whole_resource)
	{
		// begin_op (not a bracketing barrier-point pair) so consecutive
		// clear_dsv calls against different subresources of the same
		// resource -- e.g. one per dirty VSM page slice -- automatically
		// merge into a single open batch instead of one barrier pair each.
		// The batch is closed lazily by the next differing-class op or at
		// list end, same as every draw/dispatch already does via
		// pre_command/post_command.
		begin_op(BarrierSync::DEPTH_STENCIL);
		add_resource_usage(h.get_resource_info(), BarrierSync::DEPTH_STENCIL, whole_resource);
		compiler.clear_depth_stencil(h, clear_depth, clear_stencil, depth, stencil);
	}

	void CommandList::add_heaviest_barrier()
	{
		// One break is enough. It closes the operation holding the work so far
		// and opens an empty one; the barrier is then pushed into the stream
		// after that operation's (still empty) barriers_before point, and the
		// commands that follow merge into the same operation. So the ordering
		// is [prev.after][next.before] GLOBAL [next's work] -- the global
		// barrier separates the two runs, which is the point.
		//
		// A second break here would be a no-op anyway: break_op declines to
		// split an operation nothing has recorded into yet.
		break_op();
		compiler.global_barrier();
	}

	const std::vector<CommandRecord>& CommandList::get_debug_records() const
	{
		return compiler.get_debug_records();
	}

	std::vector<CommandRecord> CommandList::take_debug_records()
	{
		return compiler.take_debug_records();
	}

	DelayedCommandList* CommandListBase::get_native_list()
	{
		return &compiler;
	}

	CopyContext::CopyContext(CommandList& base) : base(base), list(base.get_native_list()) {}

	const API::CommandList& TransitionCommandList::get_compiled() const { return compiler.get_list(); }

	// --- CommandListGroup ---

	// One linear pass per resource across the whole group. Cost is proportional
	// to real activity, not to a resource's declared subresource count: a
	// resource used whole stays a single ALL_SUBRESOURCES entry from start to
	// finish and never pays per-mip anything.
	void CommandListGroup::plan_resources()
	{
		PROFILE(L"plan_resources");

		planned.clear();
		planned_built = true;

		for (auto& list : lists)
		{
			for (auto* resource : list->get_used_resources())
			{
				// Dedup by linear scan rather than a hash set: a list names a
				// resource at most once (TrackedResourceState::listed), so this
				// only has to catch resources shared BETWEEN the group's lists,
				// and `planned` is a short contiguous array of pointers. A set
				// would trade that scan for an allocation per resource.
				bool seen = false;
				for (auto& p : planned)
					if (p.resource == resource) { seen = true; break; }
				if (seen) continue;

				PlannedResource p;
				p.resource       = resource;
				p.from_undefined = resource->virgin;

				// Does this group write the resource anywhere? Only a write can
				// establish contents, so only a write can own the discard.
				bool writes = false;
				if (!resource->initialized)
				{
					for (auto& l : lists)
					{
						auto& tracked = resource->get_state(l.get());
						for (auto& [op_index, usages] : tracked.operations)
						{
							for (auto& usage : usages)
								if (usage.state.has_write_bits()) { writes = true; break; }
							if (writes) break;
						}
						if (writes) break;
					}
				}

				if (writes)
				{
					p.owns_discard = true;
					resource->initialized = true;
				}

				resource->virgin = false;
				planned.push_back(p);
			}
		}
	}

	void CommandListGroup::compile_transitions()
	{
		PROFILE(L"compile_transitions");

		// `planned` is built by plan_resources() on the submitting thread and
		// is read-only here, which is what lets groups compile in parallel. It
		// already holds every resource the group touches, deduplicated and in
		// first-seen order, so there is no list to rebuild.
		//
		// Skipping plan_resources() would silently disable every first-use
		// discard and enter every resource from the wrong state, so catch it.
		ASSERT(planned_built);

		for (auto& res_plan : planned)
		{
			Resource* resource = res_plan.resource;

			// Both first-use facts are consumed ONCE within the group, so they
			// are tracked group-locally rather than read back off the resource.
			// The discard goes to the first write; SyncBefore=NONE goes to the
			// genuinely first barrier and nothing after it, since D3D12 rejects
			// NONE once the resource has been accessed (#1417).
			bool discard_pending    = res_plan.owns_discard;
			bool undefined_pending  = res_plan.from_undefined;

			// Where the group has left each subresource so far -- carried
			// ACROSS lists, which is the whole point: list N+1 diffs against
			// where list N actually left the resource, not against the resting
			// layout. The ALL_SUBRESOURCES key is the uniform value standing in
			// for every subresource with no entry of its own, so a
			// whole-resource lifetime costs exactly one entry.
			// Per-subresource state for this group, kept as RANGES.
			//
			// A view is a rectangle in (mip x slice x plane) space and so is a
			// D3D12 barrier. Keying this per flat subresource forced both ends to
			// be shredded and rebuilt -- one map entry and one barrier per
			// subresource, thousands of them for an atlas or a Hi-Z pyramid.
			// SubresRangeMap keeps the rectangle intact from the view all the way
			// to D3D12_BARRIER_SUBRESOURCE_RANGE.
			uint dim_mips = 1, dim_slices = 1, dim_planes = 1;
			if (resource->get_desc().is_texture())
			{
				const auto& tex = resource->get_desc().as_texture();
				dim_mips   = std::max(1u, (uint)tex.MipLevels);
				dim_slices = std::max(1u, (uint)tex.ArraySize);
				dim_planes = std::max(1u, resource->get_device().Subresources(resource->get_desc())
					/ (dim_mips * dim_slices));
			}

			// A bare subresource index (a use with no descriptor -- copies name
			// one directly) as the rectangle it stands for.
			auto index_to_range = [&](UINT subres) -> SubresRange
			{
				if (subres == ALL_SUBRESOURCES || !resource->get_desc().is_texture())
					return SubresRange::all();

				const auto& tex = resource->get_desc().as_texture();
				return SubresRange::single(tex.get_mip(subres), tex.get_array(subres), tex.get_plane(subres));
			};

			SubresRangeMap current;
			current.reset(dim_mips, dim_slices, dim_planes);

			// The last operation in the group that used this resource -- its
			// barriers_after is where the return to rest goes.
			CmdListOperation* last_use = nullptr;

			for (auto& list : lists)
			{
				auto& tracked = resource->get_state(list.get());
				if (tracked.operations.empty()) continue;

				// Whoever schedules this resource may know where the previous
				// group left it -- something this group cannot see for itself.
				// Only meaningful before the group has touched it; once it has,
				// its own tracking is authoritative.
				if (current.empty() && tracked.has_entry_state)
					current.assign(SubresRange::all(), tracked.entry_state);

				// tracked.operations is an ordered map, so this walks the
				// operations in the order they were recorded.
				for (auto& [op_index, usages] : tracked.operations)
				{
					// 1. Collapse everything this operation does to the resource
					//    into one wanted state per REGION. A resource bound
					//    several ways in one operation (sampled and written
					//    through different views) merges where the states allow.
					SubresRangeMap wanted;
					wanted.reset(dim_mips, dim_slices, dim_planes);

					std::vector<std::pair<SubresRange, ResourceState>> pending;

					auto want = [&](SubresRange range, ResourceState state)
					{
						// Read-only states combine; anything else cannot be
						// satisfied by one barrier, so the later use wins and the
						// operation runs in that state.
						//
						// Collected before assigning: assign() rewrites the very
						// entries visit() is walking.
						pending.clear();
						wanted.visit(range, [&](SubresRange piece, const ResourceState* cur)
						{
							ResourceState result = state;
							if (cur)
							{
								auto merged = merge_state(*cur, state);
								result = merged ? *merged : state;
							}
							pending.emplace_back(piece, result);
						});

						for (auto& pr : pending) wanted.assign(pr.first, pr.second);
					};

					for (auto& usage : usages)
					{
						if (usage.info)
						{
							// The view reports the rectangle it covers and it goes
							// straight in -- no expansion into subresources at any
							// point. visit_subres also reports side resources (a UAV
							// counter buffer), so this still has to filter.
							visit_subres(*usage.info, [&](const HAL::Resource::ptr& r, SubresRange range)
							{
								if (r.get() == resource) want(range, usage.state);
							});
						}
						else
						{
							want(index_to_range(usage.subres), usage.state);
						}
					}

					if (wanted.empty()) continue;

					auto& operation = list->operations[op_index];
					auto& barriers  = operation.barriers_before;

					// Entering from the creation (undefined) layout. SyncBefore
					// may only be NONE while D3D12 has genuinely never seen the
					// resource -- once anything has touched it, NONE is rejected
					// outright (#1417), so widen to ALL. AccessBefore stays
					// NO_ACCESS either way: that is the only access D3D12 permits
					// alongside LAYOUT_UNDEFINED.
					auto from_undefined = [&]() -> ResourceState
					{
						if (undefined_pending)
						{
							undefined_pending = false;
							return ResourceStates::UNKNOWN;   // never seen: NONE is correct, and required
						}
						return ResourceState{ BarrierSync::ALL, BarrierAccess::NO_ACCESS, TextureLayout::UNDEFINED };
					};

					auto emit = [&](SubresRange want_range, ResourceState after)
					{
						// The first WRITE establishes the resource's contents. Its
						// memory is fresh (or freshly aliased), so discard rather
						// than preserve -- and cover the WHOLE resource in one
						// barrier, because D3D12 tracks initialization per
						// subresource while this is a per-resource fact. A cubemap
						// otherwise reports 42 uninitialized subresources (6 faces
						// x 7 mips) after only the first one was discarded.
						//
						// Keyed on `initialized`, not `virgin`: a resource whose
						// first-ever touch is a READ gets a defined layout from
						// that point on but still holds nothing, so the discard has
						// to wait for the write that actually fills it (#1422).
						//
						// Do NOT relax this to "discard on the first barrier" so
						// that a read arriving before the first write gets one.
						// Tried 2026-08-19 twice: unconditionally it costs 54 x
						// #1422 (D3D12 counts a placed RT/DS resource as initialized
						// only when the DISCARD rides a barrier INTO an RT/DS
						// layout); restricted to non-RT/DS resources it "works", but
						// only by making a read of never-written memory look
						// legitimate. A read before the first write is a bug in the
						// pass, and the debugger flagging it is the point.
						if (discard_pending && after.has_write_bits())
						{
							// Declare where the resource ACTUALLY is, not UNDEFINED
							// unconditionally: a barrier must never disagree with
							// the state we believe it starts from. Only a uniform
							// whole-resource state can be used, since this barrier
							// covers everything; if regions have diverged there is
							// no single before-state to name and UNDEFINED (don't
							// care) is the honest answer.
							const ResourceState before = current.is_uniform()
								? current.uniform_state()
								: from_undefined();

							barriers.transition(resource, before, after,
								SubresRange::all(), BarrierFlags::SINGLE | BarrierFlags::DISCARD);

							discard_pending = false;

							current.clear();
							current.assign(SubresRange::all(), after);
							return;
						}

						// True when nothing in this GROUP has touched the resource
						// yet -- not merely this region. That is the exact condition
						// the D3D12 spec attaches to SyncBefore = NONE: "there MUST
						// have been no preceding barriers or accesses made to that
						// resource in the same ExecuteCommandLists scope", and a
						// group is submitted as exactly one such scope.
						const bool group_first_touch = current.empty();

						// Whether we actually KNOW where the resource is on entry,
						// as opposed to guessing the resting layout.
						//
						// The rest block below deliberately does NOT rest
						// frame_graph_managed resources -- the FrameGraph links
						// those across passes itself and rests them once, at the
						// last pass that uses them. So for those the resting layout
						// is not where they are; the only trustworthy entry state is
						// the one the FrameGraph declares via set_entry_state, and
						// that has already been consumed into `current` above.
						const bool entry_known = !resource->frame_graph_managed;

						// One pass over the region: `current` reports each part of
						// it with the state that part is in, splitting wherever they
						// differ. What used to need a per-subresource walk plus a
						// "have they diverged?" special case is just this.
						std::vector<std::pair<SubresRange, ResourceState>> updates;

						current.visit(want_range, [&](SubresRange piece, const ResourceState* tracked_before)
						{
							ResourceState before;
							const bool was_tracked = tracked_before != nullptr;

							if (was_tracked)
							{
								before = *tracked_before;
							}
							else
							{
								// Untouched by this group so far. A resource no
								// barrier has ever moved is still in its creation
								// (undefined) layout; anything else was handed back
								// at rest by whoever used it last.
								//
								// state_at_rest, NOT {SYNC_NONE, NO_ACCESS, rest}.
								// The spec does license SyncBefore = NONE for the
								// first touch in an ExecuteCommandLists scope, but
								// NO_ACCESS paired with a REAL layout is the same
								// combination D3D12 rejects on the after side
								// (#1331). Such a barrier does not take effect, so
								// the layout never moves and the NEXT barrier fails
								// validation instead (#1334). It also bought
								// nothing: the skip below keys on layout equality,
								// which is identical under either form.
								before = res_plan.from_undefined
									? from_undefined()
									: state_at_rest(resting_layout(resource));
							}

							updates.emplace_back(piece, after);

							// Entering the scope in the layout we already want.
							// Nothing to transition and nothing to synchronize
							// against -- the scope boundary already guarantees prior
							// work retired and caches flushed.
							//
							// Gated on entry_known. Skipping both emits no barrier
							// AND records `after` as the group's belief; on a
							// GUESSED entry that belief is unfounded and every later
							// barrier is computed from a wrong base. Emitting keeps
							// a wrong guess to one self-correcting transition.
							if (entry_known && !was_tracked && group_first_touch
								&& before.layout == after.layout && !res_plan.from_undefined)
								return;

							// Read-after-read, differing only in which shader stages
							// read it. Neither side writes, so there is no hazard to
							// order and nothing to make visible. Access must match
							// exactly -- a different access needs the data made
							// visible to it even when both are reads. No entry_known
							// gate: this only fires on a state the group actually
							// tracked, never on a guess.
							if (was_tracked && before.layout == after.layout
								&& before.access == after.access
								&& !before.has_write_bits() && !after.has_write_bits())
								return;

							// Same state is not the same as no hazard. Two dispatches
							// writing the same UAV need ordering even though nothing
							// about the state changes -- the layouts match, so this
							// is purely an execution/memory barrier, which is what a
							// legacy UAV barrier was. Only unordered access needs it:
							// render-target and depth writes are pipeline-ordered,
							// and read-after-read needs nothing.
							const bool uav_ordering = (before == after)
								&& check(after.access & BarrierAccess::UNORDERED_ACCESS)
								&& was_tracked;

							if (before == after && !uav_ordering)
								return;

							barriers.transition(resource, before, after, piece, BarrierFlags::SINGLE);
						});

						for (auto& up : updates) current.assign(up.first, up.second);
					};

					// 2. Apply. Each region of the resource this operation wants a
					//    state for, with that state -- regions it does not touch
					//    report none and are skipped.
					wanted.visit(SubresRange::all(), [&](SubresRange piece, const ResourceState* st)
					{
						if (st) emit(piece, *st);
					});

					last_use = &operation;
				}
			}

			// Hand the resource back at rest, right after the group's last use
			// of it -- not at end of group, so it is released as early as it is
			// actually free. A group therefore both finds and leaves such a
			// resource in the same well-defined state, which is what makes
			// groups independent of the order they are submitted in.
			//
			// FrameGraph-managed resources opt out: the FrameGraph knows the
			// whole pass graph, so it links their usage across passes itself and
			// rests them once, at the last pass that uses them -- rather than
			// converging every group boundary.
			if (!last_use) continue;
			if (resource->frame_graph_managed) continue;

			const TextureLayout rest_layout = resting_layout(resource);
			const ResourceState rest = state_at_rest(rest_layout);
			auto& after_barriers = last_use->barriers_after;

			// Regions the group never touched report no state and are already at
			// rest by the same contract, so only tracked ones can need a barrier.
			// No subresource count anywhere in here.
			current.visit(SubresRange::all(), [&](SubresRange piece, const ResourceState* st)
			{
				if (!st) return;
				if (st->layout == rest_layout) return;
				after_barriers.transition(resource, *st, rest, piece);
			});

			current.clear();
			current.assign(SubresRange::all(), rest);

			// The contract this whole design rests on: a non-FrameGraph resource
			// enters a group at its resting layout and leaves at its resting
			// layout, so groups are independent of the order they are submitted
			// in and the next group can assume where it is without being told.
			if constexpr (BuildOptions::Dev)
			{
				ASSERT(current.check_disjoint());
				current.visit(SubresRange::all(), [&](SubresRange, const ResourceState* st)
				{
					ASSERT(!st || st->layout == rest_layout);
				});
			}
		}
	}

	// Replay every list into its API command list. Runs after
	// compile_transitions -- compile() is what consumes the barrier groups, so
	// doing it first would emit them empty.
	//
	// Lists compile independently of each other, so this fans out and joins
	// before returning: whoever called us is about to submit, and every list has
	// to be closed by then.
	void CommandListGroup::compile()
	{
		PROFILE(L"group_compile");

		if (lists.size() == 1)
		{
			// The common case (a standalone list, or a batch of one) -- not
			// worth a task dispatch.
			auto& list = lists.front();
			auto ca = list->frame_resources->get_ca(list->get_type());
			list->compiler.compile(*ca);
			list->frame_resources->free_ca(ca);
			return;
		}

		std::vector<std::future<void>> tasks;
		tasks.reserve(lists.size());

		for (auto& list : lists)
			tasks.emplace_back(thread_pool::get().enqueue([list]()
			{
					PROFILE(((CommandListBase*)list.get())->get_name());
				auto ca = list->frame_resources->get_ca(list->get_type());

				list->compiler.compile(*ca);
				list->frame_resources->free_ca(ca);
			}));

		for (auto& t : tasks)
			t.wait();
	}
}
