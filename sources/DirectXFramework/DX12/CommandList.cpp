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
		clear_uav(structured, structured->get_raw_uav());
	}


	void CommandList::begin(std::string name, Timer* t)
	{
		first_debug_log = true;
		executed = false;
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
		resource_update_counter.clear();

		Uploader::reset();
		Readbacker::reset();
		Transitions::reset();
		Eventer::begin(name, t);
		Sendable::reset();

		if (type != CommandListType::COPY) {
			set_heap(DescriptorHeapType::SAMPLER, DescriptorHeapManager::get().gpu_smp);
			set_heap(DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapManager::get().gpu_srv);
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

		execution_mutex.lock();

		execute_fence = std::promise<FenceWaiter>();
		execute_fence_result = execute_fence.get_future();
		executed = true;
		if (f)
			on_execute_funcs.emplace_back(f);

		if (!wait_for_execution_count) Device::get().get_queue(type)->execute(static_cast<CommandList*>(this));
		execution_mutex.unlock();

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

	void Eventer::resolve_times(Resource* resource, QueryHeap& pQueryHeap, uint32_t NumQueries, std::function<void()> f)
	{

		CommandList* list = static_cast<CommandList*>(this); // :(
		list->transition(resource, ResourceState::COPY_DEST);
		list->flush_transitions();
		m_commandList->ResolveQueryData(pQueryHeap.get_native(), D3D12_QUERY_TYPE_TIMESTAMP, 0, NumQueries, resource->get_native().Get(), 0);
		on_execute_funcs.emplace_back(f);
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

	void  GraphicsContext::set(UINT i, const HandleTable& table)
	{
		base.get_native_list()->SetGraphicsRootDescriptorTable(i, table.get_base().gpu);
	}
	void  GraphicsContext::set(UINT i, const HandleTableLight& table)
	{
		base.get_native_list()->SetGraphicsRootDescriptorTable(i, table.gpu);
	}

	void  GraphicsContext::set(UINT i, const Handle& table)
	{
		list->SetGraphicsRootDescriptorTable(i, table.gpu);
	}

	void  GraphicsContext::set_uav(UINT i, const D3D12_GPU_VIRTUAL_ADDRESS& table)
	{
		list->SetGraphicsRootUnorderedAccessView(i, table);
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

		/*	auto & count = resource_update_counter[resource];
			count++;
			if (count > 1)
				Log::get() << "count " << count << Log::endl;*/
		base.flush_transitions();
		//     return;
		auto info = base.place_data(size);
		memcpy(info.resource->get_data() + info.offset, data, size);
		base.get_native_list()->CopyBufferRegion(
			resource->get_native().Get(), offset, info.resource->get_native().Get(), info.offset, size);
	}

	void Uploader::write(UploadInfo& info, size_t offset, void* data, size_t size)
	{
		memcpy(info.resource->get_data() + info.offset + offset, data, size);
	}



	D3D12_GPU_VIRTUAL_ADDRESS Uploader::UploadInfo::get_gpu_address()
	{
		return resource->get_gpu_address() + offset;
	}

	ResourceAddress Uploader::UploadInfo::get_resource_address()
	{
		ResourceAddress address = resource->get_resource_address();
		address.address += offset;
		return address;
	}

	Handle Uploader::UploadInfo::create_cbv(CommandList& list)
	{

		D3D12_CONSTANT_BUFFER_VIEW_DESC  desc = {};
		desc.BufferLocation = resource->get_gpu_address() + offset;
		desc.SizeInBytes = (UINT)Math::AlignUp(size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

		assert(desc.SizeInBytes < 65536);
		Handle h = list.frame_resources->srv_uav_cbv_cpu.place();

		Device::get().create_cbv(h, resource.get(), desc);

		return h;
	}
	Uploader::UploadInfo Uploader::place_data(UINT64 uploadBufferSize, unsigned int alignment)
	{
		const auto AlignedSize = static_cast<UINT>(Math::roundUp(uploadBufferSize, alignment));
		resource_offset = Math::roundUp(resource_offset, alignment);

		if (upload_resources.empty() || (resource_offset + uploadBufferSize > upload_resources.back()->get_size()))
		{
			upload_resources.push_back(BufferCache::get().get_upload(std::max(heap_size, AlignedSize)));
			resource_offset = 0;
		}

		UploadInfo info;
		info.resource = upload_resources.back();
		info.offset = resource_offset;
		resource_offset += AlignedSize;
		//	UINT64 uploadBufferSize = GetRequiredIntermediateSize(resource->get_native().Get(), first_subresource, sub_count);
		/*   while (resource_index < upload_resources.size() && resource_offset + uploadBufferSize > upload_resources[resource_index]->get_size())
		   {
			   resource_index++; resource_offset = 0;
		   }

		   if (upload_resources.empty() || upload_resources.size() == resource_index)
		   {
			   upload_resources.push_back(BufferCache::get().get_upload(std::max(heap_size, uploadBufferSize)));
			   resource_offset = 0;
			   resource_index = upload_resources.size() - 1;
		   }

		   assert(!upload_resources.empty());
		   UploadInfo info;
		   info.resource = upload_resources[resource_index];
		   info.offset = resource_offset;
		   resource_offset += uploadBufferSize;

		   if (resource_offset >= upload_resources.back()->get_size())
			   resource_index++;
			*/
		info.size = AlignedSize;
		return info;
	}
	Readbacker::ReadBackInfo Readbacker::read_data(UINT64 uploadBufferSize)
	{
		auto buff = BufferCache::get().get_readback(uploadBufferSize);//std::make_shared<BufferBase>(uploadBufferSize, HeapType::READBACK, ResourceState::COPY_DEST);
		read_back_resources.push_back(buff);
		ReadBackInfo info;
		info.resource = buff;//read_back_resources.back();
		info.offset = 0;
		return info;
	}
	void  CopyContext::update_resource(Resource::ptr resource, UINT first_subresource, UINT sub_count, D3D12_SUBRESOURCE_DATA* data)
	{
		if (base.type != CommandListType::COPY)
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
		if (base.type != CommandListType::COPY)
			base.transition(resource, Render::ResourceState::COPY_DEST);
		base.flush_transitions();
		D3D12_RESOURCE_DESC Desc = resource->get_desc();
		UINT rows_count = box.y;

		if (Desc.Format == DXGI_FORMAT_BC7_UNORM_SRGB || Desc.Format == DXGI_FORMAT_BC7_UNORM)
			rows_count /= 4;

		int res_stride = row_stride + (D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - (row_stride) % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
		int size = res_stride * rows_count * box.z;
		auto info = base.place_data(size, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
		UINT64 RequiredSize = 0;
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT Layouts;
		UINT NumRows;
		UINT64 RowSizesInBytes;
		Device::get().get_native_device()->GetCopyableFootprints(&Desc, sub_resource, 1, info.offset, &Layouts, &NumRows, &RowSizesInBytes, &RequiredSize);

		if (res_stride == row_stride)
		{
			if (slice_stride == 0 || slice_stride == row_stride * rows_count)
			{
				BYTE* pDestSlice = reinterpret_cast<BYTE*>(info.resource->get_data() + info.offset);
				const BYTE* pSrcSlice = reinterpret_cast<const BYTE*>(data);
				memcpy(pDestSlice,
					pSrcSlice,
					size);
			}

			else
			{
				for (UINT z = 0; z < static_cast<UINT>(box.z); ++z)
				{
					BYTE* pDestSlice = reinterpret_cast<BYTE*>(info.resource->get_data() + info.offset) + (res_stride * rows_count) * z;
					const BYTE* pSrcSlice = reinterpret_cast<const BYTE*>(data) + slice_stride * z;
					memcpy(pDestSlice,
						pSrcSlice,
						row_stride * rows_count);
				}
			}
		}

		else
			for (UINT z = 0; z < static_cast<UINT>(box.z); ++z)
			{
				BYTE* pDestSlice = reinterpret_cast<BYTE*>(info.resource->get_data() + info.offset) + (res_stride * rows_count) * z;
				const BYTE* pSrcSlice = reinterpret_cast<const BYTE*>(data) + slice_stride * z;

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
		/* D3D12_BOX dbox;
		 dbox.left = 0;
		 dbox.right = size;
		 dbox.top = 0;
		 dbox.bottom = 1;
		 dbox.front = 0;
		 dbox.back = 1;*/
		base.get_native_list()->CopyTextureRegion(&Dst, offset.x, offset.y, offset.z, &Src, nullptr);
	}


	void GraphicsContext::set_const_buffer(UINT i, std::shared_ptr<GPUBuffer>& buff)
	{
		list->SetGraphicsRootConstantBufferView(i, buff->get_gpu_address());
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
		base.transition(resource, Render::ResourceState::COPY_SOURCE);
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

		UINT res_stride = static_cast<UINT>(RowSizesInBytes + (D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - (RowSizesInBytes) % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
		UINT size = res_stride * box.y * box.z;
		auto info = base.read_data(size);
		CD3DX12_TEXTURE_COPY_LOCATION source(resource->get_native().Get(), sub_resource);
		CD3DX12_TEXTURE_COPY_LOCATION dest;
		dest.pResource = info.resource->get_native().Get();
		dest.Type = D3D12_TEXTURE_COPY_TYPE::D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		dest.PlacedFootprint.Offset = 0;
		dest.PlacedFootprint.Footprint.Width = box.x;
		dest.PlacedFootprint.Footprint.Height = box.y;
		dest.PlacedFootprint.Footprint.Depth = box.z;
		dest.PlacedFootprint.Footprint.RowPitch = res_stride;
		dest.PlacedFootprint.Footprint.Format = to_srv(Layouts.Footprint.Format);
		base.get_native_list()->CopyTextureRegion(&dest, offset.x, offset.y, offset.z, &source, nullptr);
		auto result = std::make_shared<std::promise<bool>>();
		base.on_execute_funcs.push_back([result, info, f, res_stride, NumRows]()
			{
				char* data;
				D3D12_RANGE range;
				range.Begin = 0;
				range.End = info.resource->get_size();
				info.resource->get_native()->Map(0, &range, reinterpret_cast<void**>(&data));
				f(data, res_stride, res_stride * NumRows, info.resource->get_size());
				info.resource->get_native()->Unmap(0, nullptr);
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
				char* data;
				D3D12_RANGE range;
				range.Begin = info.offset;
				range.End = info.offset + size;
				info.resource->get_native()->Map(0, &range, reinterpret_cast<void**>(&data));
				f(data, size);
				info.resource->get_native()->Unmap(0, nullptr);
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
				char* data;
				D3D12_RANGE range;
				range.Begin = info.offset;
				range.End = info.offset + size;
				info.resource->get_native()->Map(0, &range, reinterpret_cast<void**>(&data));
				f(data, size);
				info.resource->get_native()->Unmap(0, nullptr);
				result->set_value(true);
			});
		return result->get_future();
	}

	void CommandList::on_execute()
	{
		Sendable::on_execute();
		// Log::get() << "on_execute" << Log::endl;
		for (auto&& t : on_execute_funcs)
			t();

		//	srv.reset();
		//	smp.reset();

		if (graphics) graphics->on_execute();
		if (compute) compute->on_execute();



		on_execute_funcs.clear();
		//	on_send_funcs.clear();
		BufferCache::get().on_execute_list(this);

		GPUCompiledManager::reset();
		Transitions::reset();
		frame_resources = nullptr;

		Device::get().id_generator.put(id);
		id = -1;


		// todo: move to queue tasks to save some heaps
		for (auto& e : tile_updates)
		{
			e.resource->on_tile_update(e);
		}


			tile_updates.clear();
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

	void Transitions::prepare_transitions(Transitions* to)
	{
		for (auto& resource : to->used_resources)
		{
			
			bool is_new = resource->is_new(id, global_id);

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

	void Transitions::transition(const std::shared_ptr<Texture>& resource, ResourceState to, UINT subres)
	{
		transition(resource.get(), to, subres);
	}
	void Transitions::transition(const Resource::ptr& resource, ResourceState to, UINT subres)
	{
		transition(resource.get(), to, subres);
	}

	void Transitions::use_resource(const Resource* resource)
	{

		Resource* res = const_cast<Resource*>(resource);

		tracked_resources.emplace_back(res->tracked_info);

		//if (res->load_fence.valid())
		{

			//	waits.emplace_back(res->load_fence.get);
		}
	}

	void Transitions::transition(const Resource* resource, ResourceState to, UINT subres)
	{
		assert(resource->get_heap_type() != HeapType::UPLOAD && resource->get_heap_type() != HeapType::READBACK);
		assert(resource->get_native().Get());

		if (resource->is_new(id, global_id))
		{
			bool good = std::find(used_resources.begin(), used_resources.end(), resource) == used_resources.end();


			assert(good);
			use_resource(resource);
			used_resources.emplace_back(const_cast<Resource*>(resource));
			assert(!resource->is_new(id, global_id));

		}

		resource->transition(transitions, resource, to, subres, id, global_id);

		//	if (transition_count == transitions.size())
			//	flush_transitions();
	}

	void Transitions::transition(Resource* from, Resource* to)
	{
		transitions.emplace_back(CD3DX12_RESOURCE_BARRIER::Aliasing(from ? from->get_native().Get() : nullptr, to->get_native().Get()));



		if (to->is_new(id, global_id))
		{

			to->aliasing(id, global_id);

			bool good = std::find(used_resources.begin(), used_resources.end(), to) == used_resources.end();


			assert(good);

			used_resources.emplace_back(const_cast<Resource*>(to));
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
		if (base.type != CommandListType::COPY) {
			base.transition(source, Render::ResourceState::COPY_SOURCE);
			base.transition(dest, Render::ResourceState::COPY_DEST);
		}
		base.flush_transitions();
		base.get_native_list()->CopyBufferRegion(dest->get_native().Get(), s_dest, source->get_native().Get(), s_source, size);
	}
	void CopyContext::copy_resource(Resource* dest, Resource* source)
	{
		if (base.type != CommandListType::COPY) {
			base.transition(source, Render::ResourceState::COPY_SOURCE);
			base.transition(dest, Render::ResourceState::COPY_DEST);
		}
		base.flush_transitions();
		base.get_native_list()->CopyResource(dest->get_native().Get(), source->get_native().Get());
	}
	void CopyContext::copy_resource(const Resource::ptr& dest, const Resource::ptr& source)
	{
		if (base.type != CommandListType::COPY) {


			base.transition(source, Render::ResourceState::COPY_SOURCE);
			base.transition(dest, Render::ResourceState::COPY_DEST);
		}
		base.flush_transitions();
		base.get_native_list()->CopyResource(dest->get_native().Get(), source->get_native().Get());
	}
	void CopyContext::copy_texture(const Resource::ptr& dest, int dest_subres, const Resource::ptr& source, int source_subres)
	{
		if (base.type != CommandListType::COPY) {
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
		if (base.type != CommandListType::COPY) {
			base.transition(from, Render::ResourceState::COPY_SOURCE);
			base.transition(to, Render::ResourceState::COPY_DEST);
		}
		base.flush_transitions();
		CD3DX12_TEXTURE_COPY_LOCATION Dst(to->get_native().Get(), 0);
		CD3DX12_TEXTURE_COPY_LOCATION Src(from->get_native().Get(), 0);
		/*
		CD3DX12_TEXTURE_COPY_LOCATION dest;
		dest.pResource = info.resource->get_native().Get();
		dest.Type = D3D12_TEXTURE_COPY_TYPE::D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		dest.PlacedFootprint.Offset = 0;
		dest.PlacedFootprint.Footprint.Width = box.x;
		dest.PlacedFootprint.Footprint.Height = box.y;
		dest.PlacedFootprint.Footprint.Depth = box.z;
		dest.PlacedFootprint.Footprint.RowPitch = res_stride;
		dest.PlacedFootprint.Footprint.Format = to_srv(Layouts.Footprint.Format);*/
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


	std::shared_ptr<UploadBuffer> BufferCache::get_upload(UINT64 size)
	{

		{
			std::lock_guard<std::mutex> m(upload);

			for (auto it = upload_resources.begin(); it != upload_resources.end(); ++it)
			{
				if (it->first >= size)
				{
					auto res = it->second;
					upload_resources.erase(it);
					return res;
				}
			}
		}
		PROFILE(L"UploadBuffer");
		return std::make_shared<UploadBuffer>(size);
		//     upload_resources.emplace_back(new BufferBase(size, HeapType::UPLOAD, ResourceState::GEN_READ));
	}

	std::shared_ptr<CPUBuffer> BufferCache::get_readback(UINT64 size)
	{
		{
			std::lock_guard<std::mutex> m(read_back);

			for (auto it = read_back_resources.begin(); it != read_back_resources.end(); ++it)
			{
				if (it->first >= size)
				{
					auto res = it->second;
					read_back_resources.erase(it);
					return res;
				}
			}
		}

		PROFILE(L"readback_buffer");
		auto buffer = std::make_shared<CPUBuffer>(size, 1);
		buffer->set_name("readback_buffer");
		return buffer;

	}



	void BufferCache::on_execute_list(CommandList* list)
	{
		{
			std::lock_guard<std::mutex> m(upload);

			for (auto&& e : list->upload_resources)
				upload_resources.insert(std::make_pair(e->get_size(), e));
		}
		{
			std::lock_guard<std::mutex> m(read_back);

			for (auto&& e : list->read_back_resources)
				if (e)
					read_back_resources.insert(std::make_pair(e->get_size(), e));
		}
		list->upload_resources.clear();
		list->read_back_resources.clear();
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

	void  ComputeContext::set(UINT i, const HandleTable& table)
	{
		set_table(i, table.get_base());
	}
	void  ComputeContext::set(UINT i, const HandleTableLight& table)
	{
		set_table(i, table);
	}


	/*  void  ComputeContext::set(UINT i, const Handle& table)
	  {
		  m_commandList->SetComputeRootDescriptorTable(i, table.gpu);
	  }*/
	void  ComputeContext::set_table(UINT i, const Handle& table)
	{
		list->SetComputeRootDescriptorTable(i, table.gpu);
	}

	void ComputeContext::set_pipeline(ComputePipelineState::ptr state)
	{
		if (state->desc.root_signature)
			set_signature(state->desc.root_signature);
		base.set_pipeline_internal(state.get());
	}

	void  ComputeContext::set_const_buffer(UINT i, std::shared_ptr<GPUBuffer> buff)
	{
		list->SetComputeRootConstantBufferView(i, buff->get_gpu_address());
	}
	void CommandList::set_pipeline_internal(PipelineStateBase* pipeline)
	{
		if (current_pipeline != pipeline)
		{
			get_native_list()->SetPipelineState(pipeline->get_native().Get());
			current_pipeline = pipeline;

			tracked_psos.emplace_back(pipeline->get_native());
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


	void BufferCache::unused_upload(std::vector<std::shared_ptr<UploadBuffer>>& upload_list)
	{
		std::lock_guard<std::mutex> m(upload);

		for (auto&& e : upload_list)
			upload_resources.insert(std::make_pair(e->get_size(), e));

		upload_list.clear();
	}

	void GraphicsContext::set_const_buffer(UINT i, const FrameResources::UploadInfo& info)
	{
		list->SetGraphicsRootConstantBufferView(i, info.resource->get_gpu_address() + info.offset);
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


	void  ComputeContext::set_const_buffer(UINT i, const FrameResources::UploadInfo& info)
	{
		list->SetComputeRootConstantBufferView(i, info.resource->get_gpu_address() + info.offset);
	}


	CommandList::ptr CommandList::get_sub_list()
	{
		auto list = Render::Device::get().get_queue(Render::CommandListType::DIRECT)->get_free_list();
		list->frame_resources = frame_resources;
		list->set_parent(get_ptr());
		//		wait_for_execution_count++;
				//list->on_send_funcs.emplace_back([this]() {wait_for_execution_count--; });
		return list;
	}


	void  ComputeContext::set_srv(UINT i, const D3D12_GPU_VIRTUAL_ADDRESS& table)
	{
		list->SetComputeRootShaderResourceView(i, table);
	}

	void  ComputeContext::set_uav(UINT i, const D3D12_GPU_VIRTUAL_ADDRESS& table)
	{
		list->SetComputeRootUnorderedAccessView(i, table);
	}

	void  GraphicsContext::set_srv(UINT i, const D3D12_GPU_VIRTUAL_ADDRESS& table)
	{
		list->SetGraphicsRootShaderResourceView(i, table);
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

	void ComputeContext::set(UINT i, const Handle& table)
	{
		list->SetComputeRootDescriptorTable(i, table.gpu);
	}



	void ComputeContext::set_const_buffer(UINT i, const D3D12_GPU_VIRTUAL_ADDRESS& table)
	{
		list->SetComputeRootConstantBufferView(i, table);
	}

	void ComputeContext::set_const_buffer(UINT i, std::shared_ptr<GPUBuffer>& buff)
	{
		list->SetComputeRootConstantBufferView(i, buff->get_gpu_address());
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
	void Uploader::reset()
	{
		if (!upload_resources.empty())
		{
			BufferCache::get().unused_upload(upload_resources);
			upload_resources.clear();
		}

		resource_offset = 0;
	}
	void Readbacker::reset()
	{


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