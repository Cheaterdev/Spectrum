#include "pch.h"
#include "CommandList.h"

namespace DX12
{


	CommandList::CommandList(CommandListType type)

	{
		this->type = type;
		D3D12_COMMAND_LIST_TYPE t = static_cast<D3D12_COMMAND_LIST_TYPE>(type);
		Device::get().get_native_device()->CreateCommandAllocator(t, IID_PPV_ARGS(&m_commandAllocator));
		Device::get().get_native_device()->CreateCommandList(0, t, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList));
		m_commandList->Close();

		if (type == CommandListType::DIRECT || type == CommandListType::COMPUTE)
			compute.reset(new ComputeContext(*this));

		if (type == CommandListType::DIRECT || type == CommandListType::COPY)
			copy.reset(new CopyContext(*this));

		if (type == CommandListType::DIRECT)
			graphics.reset(new GraphicsContext(*this));

		m_commandList->SetName(L"SpectrumCommandList");


		debug_buffer = std::make_shared<StructuredBuffer<Table::DebugStruct>>(64, counterType::NONE, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	}

	void CommandList::setup_debug(SignatureDataSetter* setter)
	{
		if (!current_pipeline->debuggable) return;
		Slots::DebugInfo info;
		StructuredBuffer<Table::DebugStruct>* structured = static_cast<StructuredBuffer<Table::DebugStruct>*>(debug_buffer.get());
		info.GetDebug() = structured->rwStructuredBuffer;
		info.set(*setter);

	}

	void CommandList::print_debug()
	{
		if (!current_pipeline->debuggable) return;

		auto pso_name = current_pipeline->name;
		get_copy().read_buffer(debug_buffer.get(), 0, 3 * sizeof(Table::DebugStruct::CB), [this, pso_name](const char* data, UINT64 size)
			{
				if (first_debug_log)
				{
					Log::get() << "-----------------------------------------" << Log::endl;

					first_debug_log = false;
				}
				auto result = reinterpret_cast<const Table::DebugStruct::CB*>(data);

				Log::get() << "DEBUG(" << name << "): " << pso_name<< Log::endl;
				for (int i = 0; i < 3; i++)
				{
					Log::get() << "debug(" << i << "): " << result[i].v.x << " " << result[i].v.y << " " << result[i].v.z << " " << result[i].v.w << " " << Log::endl;
				}

			});

		StructuredBuffer<Table::DebugStruct>* structured = static_cast<StructuredBuffer<Table::DebugStruct>*>(debug_buffer.get());
		clear_uav(structured->get_raw_uav());
	}


	void CommandList::begin(std::string name, Timer* t)
	{
#ifdef DEV
		begin_stack = Exceptions::get_stack_trace();
#endif

		first_debug_log = true;
		id = Device::get().id_generator.get();
		static atomic_uint64_t _global_id;
		global_id = _global_id++;

		//       Log::get() << "begin" << Log::endl;
		m_commandAllocator->Reset();
		TEST(m_commandList->Reset(m_commandAllocator.Get(), nullptr));
		//resource_index = 0;

		if (graphics) graphics->begin();
		if (compute) compute->begin();


		for (auto& e : heaps)
			e = nullptr;
	
		Transitions::reset();
		Eventer::begin(name, t);
	
		if (type != CommandListType::COPY) {
			set_heap(DescriptorHeapType::SAMPLER, DescriptorHeapManager::get().get_gpu_heap(DescriptorHeapType::SAMPLER));
			set_heap(DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapManager::get().get_gpu_heap(DescriptorHeapType::CBV_SRV_UAV));
		}
		//set_heap(DescriptorHeapType::SAMPLER, smp_descriptors.get_heap());
	}


	void CommandList::end()
	{

		//	id = -1;

		current_pipeline = nullptr;

		if (graphics) graphics->end();
		if (compute) compute->end();

		Eventer::reset();

	}

	void GraphicsContext::begin()
	{
		topology = D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

	}
	void GraphicsContext::end()
	{
		current_root_signature = nullptr;
	}

	void ComputeContext::begin()
	{

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

	std::shared_future<FenceWaiter> Sendable::execute(std::function<void()> f)
	{
		CommandList* list = static_cast<CommandList*>(this); // :(

		list->flush_transitions();
		TEST(m_commandList->Close());

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
		m_commandList->EndQuery(pQueryHeap.get_native(), D3D12_QUERY_TYPE_TIMESTAMP, QueryIdx);
	}

	void Eventer::resolve_times(QueryHeap& pQueryHeap, uint32_t NumQueries, std::function<void(std::span<UINT64>)> f)
	{	
		CommandList* list = static_cast<CommandList*>(this); // :(
		auto info = list->read_data(NumQueries* sizeof(UINT64));

		list->flush_transitions();
		m_commandList->ResolveQueryData(pQueryHeap.get_native(), D3D12_QUERY_TYPE_TIMESTAMP, 0, NumQueries, info.resource->get_native().Get(), info.offset);


		on_execute_funcs.emplace_back([info, f, NumQueries]() {

			UINT64* data = reinterpret_cast<UINT64*>(info.get_cpu_data());
			f(std::span(data, NumQueries));

			});
	}

	void GraphicsContext::set_signature(const RootSignature::ptr& s)
	{
		assert(s);
		if (current_root_signature != s)
		{
			base.get_native_list()->SetGraphicsRootSignature(s->get_native().Get());
			current_root_signature = s;
		}
	}

	void GraphicsContext::set_heaps(DescriptorHeap::ptr& a, DescriptorHeap::ptr& b)
	{
		ID3D12DescriptorHeap* heaps[2] = { 0 };

		if (a)
			heaps[0] = a->get_native().Get();

		if (b)
			heaps[1] = b->get_native().Get();

		base.get_native_list()->SetDescriptorHeaps(b ? 2 : 1, heaps);
	}


	void  GraphicsContext::set(UINT i, const HandleTableLight& table)
	{
		base.get_native_list()->SetGraphicsRootDescriptorTable(i, table.gpu);
	}


	void  GraphicsContext::set_const_buffer(UINT i, const D3D12_GPU_VIRTUAL_ADDRESS& table)
	{
		list->SetGraphicsRootConstantBufferView(i, table);
	}

	void GraphicsContext::set_rtv(const HandleTable& table, Handle h)
	{
		set_rtv(table.get_count(), table.get_base(), h);
	}
	void GraphicsContext::set_rtv(const HandleTableLight& table, Handle h)
	{
		set_rtv(table.get_count(), table, h);
	}

	void GraphicsContext::set_rtv(int c, Handle rt, Handle h)
	{

		for (int i = 0; i < c; i++)
		{
			get_base().transition_rtv(&rt.resource_info[i]);
		}

		if (h.is_valid())
			get_base().transition_dsv(h.resource_info);

		get_base().flush_transitions();
		list->OMSetRenderTargets(c, &rt.cpu, true, h.is_valid() ? &h.cpu : nullptr);
	}

void GraphicsContext::set_rtv(std::initializer_list<Handle> rt, Handle h)
	{
		set_rtv(place_rtv(rt), h);	
	}
	void GraphicsContext::flush_binds(bool force)
	{
		base.flush_heaps(force);
	}
	void GraphicsContext::draw(UINT vertex_count, UINT vertex_offset, UINT instance_count, UINT instance_offset)
	{

		base.setup_debug(this);

		base.flush_transitions();
		flush_binds();
		list->DrawInstanced(vertex_count, instance_count, vertex_offset, instance_offset);
		get_base().print_debug();
	}
	void GraphicsContext::draw_indexed(UINT index_count, UINT index_offset, UINT vertex_offset, UINT instance_count, UINT instance_offset)
	{
		if (instance_count == 0) return;

		base.setup_debug(this);

		base.flush_transitions();
		flush_binds();
		list->DrawIndexedInstanced(index_count, instance_count, index_offset, vertex_offset, instance_offset);
		get_base().print_debug();
	}



	void GraphicsContext::set_scissors(sizer_long rect)
	{
		if (rect.left >= rect.right || rect.top >= rect.bottom)
			Log::get() << Log::LEVEL_WARNING << "rect is invalid" << Log::endl;

		list->RSSetScissorRects(1, reinterpret_cast<D3D12_RECT*>(&rect));
	}

	void GraphicsContext::set_viewports(std::vector<Viewport> viewports)
	{
		this->viewports = viewports;
		list->RSSetViewports(static_cast<UINT>(viewports.size()), viewports.data());
	}

	void GraphicsContext::set_scissor(sizer_long rect)
	{
		if (rect.left >= rect.right || rect.top >= rect.bottom)
			Log::get() << Log::LEVEL_WARNING << "rect is invalid" << Log::endl;

		list->RSSetScissorRects(1, reinterpret_cast<D3D12_RECT*>(&rect));
	}

	void GraphicsContext::set_viewport(Viewport v)
	{
		this->viewports.resize(1);
		this->viewports[0] = v;
		list->RSSetViewports(static_cast<UINT>(viewports.size()), viewports.data());
	}

	void GraphicsContext::set_viewport(vec4 v)
	{
		this->viewports.resize(1);
		this->viewports[0].TopLeftX = v.x;
		this->viewports[0].TopLeftY = v.y;
		this->viewports[0].Width = v.z;
		this->viewports[0].Height = v.w;
		this->viewports[0].MinDepth = 0;
		this->viewports[0].MaxDepth = 1;
		list->RSSetViewports(static_cast<UINT>(viewports.size()), viewports.data());
	}

	void  CopyContext::update_buffer(Resource::ptr resource, UINT offset, const  char* data, UINT size)
	{
		update_buffer(resource.get(), offset, data, size);
	}
	void  CopyContext::update_buffer(Resource* resource, UINT offset, const  char* data, UINT size)
	{
		if (base.type != CommandListType::COPY)
			base.transition(resource, Render::ResourceState::COPY_DEST);

		base.flush_transitions();
		auto info = base.place_data(size);
		memcpy(info.get_cpu_data(), data, size);
		base.get_native_list()->CopyBufferRegion(
			resource->get_native().Get(), offset, info.resource->get_native().Get(), info.offset, size);
	}

	D3D12_GPU_VIRTUAL_ADDRESS UploadInfo::get_gpu_address()
	{
		return resource->get_gpu_address() + offset;
	}

	ResourceAddress UploadInfo::get_resource_address()
	{
		ResourceAddress address = resource->get_resource_address();
		address.address += offset;
		return address;
	}


	std::byte* UploadInfo::get_cpu_data() const
	{
		return resource->buffer_data + offset;
	}
	Handle UploadInfo::create_cbv(CommandList& list)
	{

		D3D12_CONSTANT_BUFFER_VIEW_DESC  desc = {};
		desc.BufferLocation = resource->get_gpu_address() + offset;
		desc.SizeInBytes = (UINT)Math::AlignUp(size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

		assert(desc.SizeInBytes < 65536);
		Handle h = list.frame_resources->get_cpu_heap(Render::DescriptorHeapType::CBV_SRV_UAV).place();

		Device::get().create_cbv(h, resource.get(), desc);

		return h;
	}
	std::byte* Readbacker::ReadBackInfo::get_cpu_data() const
	{
		return resource->buffer_data + offset;
	}
	Readbacker::ReadBackInfo Readbacker::read_data(UINT64 uploadBufferSize, unsigned int alignment)
	{
		const auto AlignedSize = static_cast<UINT>(Math::roundUp(uploadBufferSize, alignment));	
		auto handle = allocator.alloc(AlignedSize, alignment, D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS, HeapType::READBACK);

		handles.emplace_back(handle);
		ReadBackInfo info;
		info.resource = handle.get_heap()->cpu_buffer;
		info.offset = handle.get_offset();
		info.size = uploadBufferSize;
		return info;
	}
	void  CopyContext::update_resource(Resource::ptr resource, UINT first_subresource, UINT sub_count, D3D12_SUBRESOURCE_DATA* data)
	{
	//	if (base.type != CommandListType::COPY)
			base.transition(resource, Render::ResourceState::COPY_DEST);

		base.flush_transitions();
		UINT64 uploadBufferSize = GetRequiredIntermediateSize(resource->get_native().Get(), first_subresource, sub_count);
		auto info = base.place_data(uploadBufferSize, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
		UpdateSubresources(base.get_native_list().Get(), resource->get_native().Get(), info.resource->get_native().Get(), info.offset, first_subresource, sub_count, data);
	}
	void CopyContext::update_texture(Resource::ptr resource, ivec3 offset, ivec3 box, UINT sub_resource, const char* data, UINT row_stride, UINT slice_stride)
	{
		update_texture(resource.get(), offset, box, sub_resource, data, row_stride, slice_stride);
	}

	void CopyContext::update_texture(Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, const char* data, UINT row_stride, UINT slice_stride)
	{
	//	if (base.type != CommandListType::COPY)
			base.transition(resource, Render::ResourceState::COPY_DEST);
		base.flush_transitions();
		D3D12_RESOURCE_DESC Desc = resource->get_desc();
		UINT rows_count = box.y;

		if (Desc.Format == DXGI_FORMAT_BC7_UNORM_SRGB || Desc.Format == DXGI_FORMAT_BC7_UNORM)
			rows_count /= 4;

		int res_stride = Math::AlignUp(row_stride, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
		int size = res_stride * rows_count * box.z;
		const auto info = base.place_data(size, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
		UINT64 RequiredSize = 0;
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT Layouts;
		UINT NumRows;
		UINT64 RowSizesInBytes;
		Device::get().get_native_device()->GetCopyableFootprints(&Desc, sub_resource, 1, info.offset, &Layouts, &NumRows, &RowSizesInBytes, &RequiredSize);

		if (res_stride == row_stride)
		{
			if (slice_stride == 0 || slice_stride == row_stride * rows_count)
			{
				auto pDestSlice = info.get_cpu_data();
				const std::byte* pSrcSlice = reinterpret_cast<const std::byte*>(data);
				memcpy(pDestSlice,
					pSrcSlice,
					size);
			}

			else
			{
				for (UINT z = 0; z < static_cast<UINT>(box.z); ++z)
				{
					auto pDestSlice = info.get_cpu_data() + (res_stride * rows_count) * z;
					const std::byte* pSrcSlice = reinterpret_cast<const std::byte*>(data) + slice_stride * z;
					memcpy(pDestSlice,
						pSrcSlice,
						row_stride * rows_count);
				}
			}
		}

		else
			for (UINT z = 0; z < static_cast<UINT>(box.z); ++z)
			{
				std::byte* pDestSlice = info.get_cpu_data() + (res_stride * rows_count) * z;
				const std::byte* pSrcSlice = reinterpret_cast<const std::byte*>(data) + slice_stride * z;

				for (UINT y = 0; y < rows_count; ++y)
				{
					memcpy(pDestSlice + res_stride * y,
						pSrcSlice + row_stride * y,
						row_stride);
				}
			}

		CD3DX12_TEXTURE_COPY_LOCATION Dst(resource->get_native().Get(), sub_resource);
		CD3DX12_TEXTURE_COPY_LOCATION Src;
		Src.pResource = info.resource->get_native().Get();
		Src.Type = D3D12_TEXTURE_COPY_TYPE::D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		Src.PlacedFootprint.Offset = info.offset;
		Src.PlacedFootprint.Footprint.Width = box.x;
		Src.PlacedFootprint.Footprint.Height = box.y;
		Src.PlacedFootprint.Footprint.Depth = box.z;
		Src.PlacedFootprint.Footprint.RowPitch = res_stride;
		Src.PlacedFootprint.Footprint.Format = Layouts.Footprint.Format;
		base.get_native_list()->CopyTextureRegion(&Dst, offset.x, offset.y, offset.z, &Src, nullptr);
	}

	void  GraphicsContext::set_pipeline(PipelineState::ptr state)
	{
		set_signature(state->desc.root_signature);
		base.set_pipeline_internal(state.get());
	}

	void GraphicsContext::set_layout(Layouts layout)
	{
		set_signature(get_Signature(layout));
	}
	std::future<bool> CopyContext::read_texture(Resource::ptr resource, ivec3 offset, ivec3 box, UINT sub_resource, std::function<void(const char*, UINT64, UINT64, UINT64)> f)
	{
		return read_texture(resource.get(), offset, box, sub_resource, f);
	}
	std::future<bool> CopyContext::read_texture(const Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, std::function<void(const char*, UINT64, UINT64, UINT64)> f)
	{
		if (base.type != CommandListType::COPY)
			base.transition(resource, Render::ResourceState::COPY_SOURCE);
		else
			base.transition(resource, Render::ResourceState::COMMON);

		base.flush_transitions();
		UINT64 RequiredSize = 0;
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT Layouts;
		UINT NumRows;
		UINT64 RowSizesInBytes;
		D3D12_RESOURCE_DESC Desc = resource->get_desc();
		Device::get().get_native_device()->GetCopyableFootprints(&Desc, sub_resource, 1, 0, &Layouts, &NumRows, &RowSizesInBytes, &RequiredSize);

		if (!RequiredSize)
		{
			std::promise<bool> str;
			str.set_value(false);
			return str.get_future();
		}

		int res_stride = Math::AlignUp(RowSizesInBytes, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
		UINT size = res_stride * box.y * box.z;
		auto info = base.read_data(size, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
		CD3DX12_TEXTURE_COPY_LOCATION source(resource->get_native().Get(), sub_resource);
		CD3DX12_TEXTURE_COPY_LOCATION dest;
		dest.pResource = info.resource->get_native().Get();
		dest.Type = D3D12_TEXTURE_COPY_TYPE::D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		dest.PlacedFootprint.Offset = info.offset;
		dest.PlacedFootprint.Footprint.Width = box.x;
		dest.PlacedFootprint.Footprint.Height = box.y;
		dest.PlacedFootprint.Footprint.Depth = box.z;
		dest.PlacedFootprint.Footprint.RowPitch = res_stride;
		dest.PlacedFootprint.Footprint.Format = to_srv(Layouts.Footprint.Format);
		base.get_native_list()->CopyTextureRegion(&dest, offset.x, offset.y, offset.z, &source, nullptr);
		auto result = std::make_shared<std::promise<bool>>();
		base.on_execute_funcs.push_back([result, info, f, res_stride, NumRows]()
			{			
				f(reinterpret_cast<char*>(info.get_cpu_data()), res_stride, res_stride * NumRows, info.resource->get_size());
				result->set_value(true);
			});
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

		base.transition(resource, Render::ResourceState::COPY_SOURCE);
		base.flush_transitions();
		//  auto size = resource->get_size();
		auto info = base.read_data(size);
		//  m_commandList->CopyResource(info.resource->get_resource()->get_native().Get(), resource->get_native().Get());
		base.get_native_list()->CopyBufferRegion(info.resource->get_native().Get(), info.offset, resource->get_native().Get(), offset, size);
		base.on_execute_funcs.push_back([result, info, f, size]()
			{
				f(reinterpret_cast<char*>(info.get_cpu_data()), size);
				result->set_value(true);
			});
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

		base.flush_transitions();
		//  auto size = resource->get_size();
		auto info = base.read_data(size);
		//  m_commandList->CopyResource(info.resource->get_resource()->get_native().Get(), resource->get_native().Get());
		base.get_native_list()->ResolveQueryData(query_heap->get_native(), D3D12_QUERY_TYPE_PIPELINE_STATISTICS, 0, 1, info.resource->get_native().Get(), info.offset);
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
		Transitions::reset();
		frame_resources = nullptr;

		Device::get().id_generator.put(id);
		id = -1;


	}

	void Transitions::flush_transitions()
	{
		if (!transitions.empty())
		{
			PROFILE_GPU(L"flush_transitions");
			m_commandList->ResourceBarrier((UINT)transitions.size(), transitions.data());
			transitions.clear();
		}
	}
	std::shared_ptr<TransitionCommandList> Transitions::fix_pretransitions()
	{
		PROFILE(L"fix_pretransitions");

		std::vector<D3D12_RESOURCE_BARRIER> result;
		std::vector<Resource*> discards;

		result.reserve(used_resources.size());

		
		ResourceState states = ResourceState::COMMON;
		for (auto& r : used_resources)
		{

			states= states|r->process_transitions(result, discards, r, id, global_id);


		}

		auto transition_type = GetBestType(states, type);

		if (result.size())
		{
			transition_list = Device::get().get_queue(transition_type)->get_transition_list();
			transition_list->create_transition_list(result, discards);
			return transition_list;
		}
		return nullptr;
	}

	void Transitions::prepare_transitions(Transitions* to, bool all)
	{
		for (auto& resource : to->used_resources)
		{
			
			bool is_new = resource->is_new(id, global_id);
			if((all&&is_new)||(!all&&!is_new))
			if (resource->transition(type, transitions, resource, id, global_id, to->id, to->global_id))
			{
				if (is_new)
				{
					use_resource(resource);
					used_resources.emplace_back(const_cast<Resource*>(resource));
					assert(!resource->is_new(id, global_id));
				}
			}
		}
	}
	/*	void Transitions::assume_state(Resource* resource, ResourceState state)
		{
			resource->assume_state(id, state);
		}
		void Transitions::assume_state(const Resource::ptr& resource, ResourceState state)
		{
			resource->assume_state(id, state);
		}*/


	void Transitions::transition(const Resource::ptr& resource, ResourceState to, UINT subres)
	{
		transition(resource.get(), to, subres);
	}

	void Transitions::use_resource(const Resource* resource)
	{
		Resource* res = const_cast<Resource*>(resource);
		if (res->use_by(id, global_id))
		{
			tracked_resources.emplace_back(res->tracked_info);
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
		assert(resource->get_heap_type() != HeapType::UPLOAD && resource->get_heap_type() != HeapType::READBACK);
		assert(resource->get_native().Get());
		CommandList* list = static_cast<CommandList*>(this); // :(

		if (resource->is_new(id, global_id))
		{
			bool good = std::find(used_resources.begin(), used_resources.end(), resource) == used_resources.end();


			assert(good);
			use_resource(resource);
			used_resources.emplace_back(const_cast<Resource*>(resource));
#ifdef DEV
			const_cast<Resource*>(resource)->used(list);
#endif
			assert(!resource->is_new(id, global_id));

		}

		resource->transition(transitions, resource, to, subres, id, global_id);

		//	if (transition_count == transitions.size())
			//	flush_transitions();
	}

	void Transitions::transition(Resource* from, Resource* to)
	{
		transitions.emplace_back(CD3DX12_RESOURCE_BARRIER::Aliasing(from ? from->get_native().Get() : nullptr, to->get_native().Get()));

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
		//		flush_transitions();
	}

	void Transitions::transition_uav(Resource* resource)
	{


		transitions.emplace_back(CD3DX12_RESOURCE_BARRIER::UAV(resource->get_native().Get()));

		//	if (transition_count == transitions.size())
		//		flush_transitions();
	}

	void CopyContext::copy_buffer(Resource* dest, int s_dest, Resource* source, int s_source, int size)
	{
		//if (base.type != CommandListType::COPY)
		{
			base.transition(source, Render::ResourceState::COPY_SOURCE);
			base.transition(dest, Render::ResourceState::COPY_DEST);
		}
		base.flush_transitions();
		base.get_native_list()->CopyBufferRegion(dest->get_native().Get(), s_dest, source->get_native().Get(), s_source, size);
	}
	void CopyContext::copy_resource(Resource* dest, Resource* source)
	{
	//	if (base.type != CommandListType::COPY)
		{
			base.transition(source, Render::ResourceState::COPY_SOURCE);
			base.transition(dest, Render::ResourceState::COPY_DEST);
		}
		base.flush_transitions();
		base.get_native_list()->CopyResource(dest->get_native().Get(), source->get_native().Get());
	}
	void CopyContext::copy_resource(const Resource::ptr& dest, const Resource::ptr& source)
	{
		//if (base.type != CommandListType::COPY)
		{


			base.transition(source, Render::ResourceState::COPY_SOURCE);
			base.transition(dest, Render::ResourceState::COPY_DEST);
		}
		base.flush_transitions();
		base.get_native_list()->CopyResource(dest->get_native().Get(), source->get_native().Get());
	}
	void CopyContext::copy_texture(const Resource::ptr& dest, int dest_subres, const Resource::ptr& source, int source_subres)
	{
		//if (base.type != CommandListType::COPY) 
		{
			base.transition(source, Render::ResourceState::COPY_SOURCE, source_subres);
			base.transition(dest, Render::ResourceState::COPY_DEST, dest_subres);
		}
		base.flush_transitions();
		CD3DX12_TEXTURE_COPY_LOCATION Dst(dest->get_native().Get(), dest_subres);
		CD3DX12_TEXTURE_COPY_LOCATION Src(source->get_native().Get(), source_subres);
		base.get_native_list()->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
	}

	void CopyContext::copy_texture( const Resource::ptr& to, ivec3 to_pos, const Resource::ptr& from, ivec3 from_pos, ivec3 size)
	{
		//if (base.type != CommandListType::COPY) 
		{
			base.transition(from, Render::ResourceState::COPY_SOURCE);
			base.transition(to, Render::ResourceState::COPY_DEST);
		}
		base.flush_transitions();
		
		CD3DX12_TEXTURE_COPY_LOCATION Dst(to->get_native().Get(), 0);
		CD3DX12_TEXTURE_COPY_LOCATION Src(from->get_native().Get(), 0);
		
		D3D12_BOX box;
		box.left = from_pos.x;
		box.top = from_pos.y;
		box.front = from_pos.z;


		box.right = from_pos.x + size.x;
		box.bottom = from_pos.y + size.y;
		box.back = from_pos.z + size.z;
		base.get_native_list()->CopyTextureRegion(&Dst, to_pos.x, to_pos.y, to_pos.z, &Src, &box);
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
	

	void ComputeContext::set_signature(const RootSignature::ptr& s)
	{
		if (s != current_compute_root_signature)
		{
			current_compute_root_signature = s;
			list->SetComputeRootSignature(s->get_native().Get());
		}
	}
	void ComputeContext::dispach(int x, int y, int z)
	{

		base.setup_debug(this);

		base.flush_transitions();
		flush_binds();

		list->Dispatch(x, y, z);
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
		list->SetComputeRootDescriptorTable(i, table.gpu);
	}
	
	void ComputeContext::set_pipeline(ComputePipelineState::ptr state)
	{
		if (state->desc.root_signature)
			set_signature(state->desc.root_signature);
		base.set_pipeline_internal(state.get());
	}
	
	void CommandList::set_pipeline_internal(PipelineStateBase* pipeline)
	{
		if (current_pipeline != pipeline)
		{
			if (pipeline)
			{
				get_native_list()->SetPipelineState(pipeline->get_native().Get());
				tracked_psos.emplace_back(pipeline->get_native());
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
		if (name.size())
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
		::PIXBeginEvent(m_commandList.Get(), 0, str.c_str());
	}

	void Eventer::end_event()
	{
		::PIXEndEvent(m_commandList.Get());
	}

	void Eventer::set_marker(const wchar_t* label)
	{
		::PIXSetMarker(m_commandList.Get(), 0, label);
	}
	
	FrameResources::ptr FrameResourceManager::begin_frame()
	{

		auto result = std::make_shared<FrameResources>();


		result->frame_number = frame_number++;

		return result;
	}

	Render::CommandList::ptr FrameResources::start_list(std::string name, CommandListType type)
	{
		auto list = Render::Device::get().get_queue(type)->get_free_list();
		list->begin(name);
		list->frame_resources = shared_from_this();
		return list;
	}

	void ComputeContext::flush_binds(bool force)
	{
		base.flush_heaps(force);
	}
	
	void GraphicsContext::execute_indirect(IndirectCommand& command_types, UINT max_commands, Resource* command_buffer, UINT64 command_offset, Resource* counter_buffer, UINT64 counter_offset)
	{

		base.setup_debug(this);



		if (command_buffer) get_base().transition(command_buffer, ResourceState::INDIRECT_ARGUMENT);
		if (counter_buffer) get_base().transition(counter_buffer, ResourceState::INDIRECT_ARGUMENT);

		base.flush_transitions();

		list->ExecuteIndirect(
			command_types.command_signature.Get(),
			max_commands,
			command_buffer ? command_buffer->get_native().Get() : nullptr,
			command_offset,
			counter_buffer ? counter_buffer->get_native().Get() : nullptr,
			counter_offset);

		get_base().print_debug();
	}
	void ComputeContext::execute_indirect(IndirectCommand& command_types, UINT max_commands, Resource* command_buffer, UINT64 command_offset, Resource* counter_buffer, UINT64 counter_offset)
	{

		base.setup_debug(this);

		if (command_buffer) get_base().transition(command_buffer, ResourceState::INDIRECT_ARGUMENT);
		if (counter_buffer) get_base().transition(counter_buffer, ResourceState::INDIRECT_ARGUMENT);

		base.flush_transitions();

		list->ExecuteIndirect(
			command_types.command_signature.Get(),
			max_commands,
			command_buffer ? command_buffer->get_native().Get() : nullptr,
			command_offset,
			counter_buffer ? counter_buffer->get_native().Get() : nullptr,
			counter_offset);

		get_base().print_debug();
	}


	void ComputeContext::set_const_buffer(UINT i, const D3D12_GPU_VIRTUAL_ADDRESS& table)
	{
		list->SetComputeRootConstantBufferView(i, table);
	}
	
	void Transitions::reset()
	{
		
		used_resources.clear();
		TrackedResource::allow_resource_delete = true;

		tracked_resources.clear();
		tracked_psos.clear();
		tracked_heaps.clear();
		TrackedResource::allow_resource_delete = false;

		transition_list = nullptr;
	}
	void Readbacker::reset()
	{
		for (auto& h : handles)
			h.Free();
		handles.clear();


	}



	TransitionCommandList::TransitionCommandList(CommandListType type):type(type)
	{
		D3D12_COMMAND_LIST_TYPE t = static_cast<D3D12_COMMAND_LIST_TYPE>(type);

		TEST(Device::get().get_native_device()->CreateCommandAllocator(t, IID_PPV_ARGS(&m_commandAllocator)));
		TEST(Device::get().get_native_device()->CreateCommandList(0, t, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
		m_commandList->Close();
		m_commandList->SetName(L"TransitionCommandList");
	}

	void TransitionCommandList::create_transition_list(const std::vector<D3D12_RESOURCE_BARRIER>& transitions, std::vector<Resource*>& discards)
	{
		m_commandAllocator->Reset();
		m_commandList->Reset(m_commandAllocator.Get(), nullptr);
		m_commandList->ResourceBarrier(static_cast<UINT>(transitions.size()), transitions.data());

		for (auto e : discards)
		{
			//	m_commandList->DiscardResource(e->get_native().Get(), nullptr);
		}
		m_commandList->Close();
	}

	ComPtr<ID3D12GraphicsCommandList4> TransitionCommandList::get_native()
	{
		return m_commandList;
	}
}