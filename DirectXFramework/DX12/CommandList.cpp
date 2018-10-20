#include "pch.h"

namespace DX12
{


	CommandList::CommandList(CommandListType type)//:descriptor_manager_graphics(descriptor_manager), descriptor_manager_compute(descriptor_manager)

	{
		this->type = type;
		D3D12_COMMAND_LIST_TYPE t = static_cast<D3D12_COMMAND_LIST_TYPE>(type);
		Device::get().get_native_device()->CreateCommandAllocator(t, IID_PPV_ARGS(&m_commandAllocator));
		Device::get().get_native_device()->CreateCommandList(0, t, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList));
		m_commandList->Close();
	}

	void CommandList::begin(std::string name, Timer*t)
	{
		executed = false;
		id = Device::get().id_generator.get();
		static uint64_t _global_id = 0;
		global_id = _global_id++;
		topology = D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		//       Log::get() << "begin" << Log::endl;
		m_commandAllocator->Reset();
		TEST(m_commandList->Reset(m_commandAllocator.Get(), nullptr));
		//resource_index = 0;
	
		for (auto& e : heaps)
			e = nullptr;


		Uploader::reset();
		Readbacker::reset();
		Transitions::reset();
		Eventer::begin(name,t);
		Sendable::reset();

		set_heap(DescriptorHeapType::SAMPLER, DescriptorHeapManager::get().get_samplers());
		//	upload_iterator = upload_resources.begin()
	}


	void CommandList::end()
	{
		Device::get().id_generator.put(id);
	//	id = -1;
		flush_transitions();
		current_pipeline = nullptr;
		current_root_signature = nullptr;
		current_compute_pipeline = nullptr;
		current_compute_root_signature = nullptr;
		//    Log::get() << "end" << Log::endl;
		Eventer::reset();
	
		TEST(m_commandList->Close());
	}

	void Sendable::on_done(std::function<void()> f)
	{
		on_execute_funcs.emplace_back(f);

	}

	std::shared_future<UINT64> Sendable::execute(std::function<void()> f)
	{
		execution_mutex.lock();

		execute_fence = std::promise<UINT64>();
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

		Device::get().get_queue(type)->wait(execute(f).get());
	}


	void Eventer::insert_time(QueryHeap& pQueryHeap, uint32_t QueryIdx)
	{
		m_commandList->EndQuery(pQueryHeap.get_native(), D3D12_QUERY_TYPE_TIMESTAMP, QueryIdx);
	}

	void Eventer::resolve_times(Resource* resource, QueryHeap& pQueryHeap, uint32_t NumQueries, std::function<void()> f)
	{
		m_commandList->ResolveQueryData(pQueryHeap.get_native(), D3D12_QUERY_TYPE_TIMESTAMP, 0, NumQueries, resource->get_native().Get(), 0);
		on_execute_funcs.emplace_back(f);
	}

	void GraphicsContext::set_signature(const RootSignature::ptr& s)
	{
		assert(s);
		if (current_root_signature != s)
		{
			m_commandList->SetGraphicsRootSignature(s->get_native().Get());
			current_root_signature = s;
			descriptor_manager_graphics.parse(current_root_signature);
		}
	}

	void GraphicsContext::set_heaps(DescriptorHeap::ptr& a, DescriptorHeap::ptr& b)
	{
		ID3D12DescriptorHeap* heaps[2] = { 0 };

		if (a)
			heaps[0] = a->get_native().Get();

		if (b)
			heaps[1] = b->get_native().Get();

		m_commandList->SetDescriptorHeaps(b ? 2 : 1, heaps);
	}

	void  GraphicsContext::set(UINT i, const HandleTable& table)
	{
		m_commandList->SetGraphicsRootDescriptorTable(i, table.get_base().gpu);
	}

	void  GraphicsContext::set(UINT i, const Handle& table)
	{
		m_commandList->SetGraphicsRootDescriptorTable(i, table.gpu);
	}
	void  GraphicsContext::set(UINT i, const D3D12_GPU_VIRTUAL_ADDRESS& table)
	{
		m_commandList->SetGraphicsRootShaderResourceView(i, table);
	}
	void  GraphicsContext::set_uav(UINT i, const D3D12_GPU_VIRTUAL_ADDRESS& table)
	{
		m_commandList->SetGraphicsRootUnorderedAccessView(i, table);
	}
	void  GraphicsContext::set_const_buffer(UINT i, const D3D12_GPU_VIRTUAL_ADDRESS& table)
	{
		m_commandList->SetGraphicsRootConstantBufferView(i, table);
	}

	void  GraphicsContext::set(UINT i, std::vector<Handle>& table)
	{
		descriptor_manager_graphics.bind_table(i, table);
		/*if (table.empty()) return;

		auto t = descriptor_manager_graphics.get_table(table.size());
		Device::get().get_native_device()->CopyDescriptorsSimple(table.size(), t.get_base().cpu, table.data()->cpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		set(i, t);*/
	}

	void GraphicsContext::set_rtv(const HandleTable& table, Handle h)
	{
		set_rtv(table.get_count(), table.get_base(), h);
	}

	void GraphicsContext::set_rtv(int c, Handle rt, Handle h)
	{
		m_commandList->OMSetRenderTargets(c, &rt.cpu, true, h.is_valid() ? &h.cpu : nullptr);
	}


	void GraphicsContext::flush_binds(bool force)
	{
		descriptor_manager_graphics.bind(this, [this](int i, Handle t)
		{
			set(i, t);
		});

		flush_heaps(force);
	}
	void GraphicsContext::draw(UINT vertex_count, UINT vertex_offset, UINT instance_count, UINT instance_offset)
	{
		flush_transitions();
		flush_binds();
		m_commandList->DrawInstanced(vertex_count, instance_count, vertex_offset, instance_offset);
	}
	void GraphicsContext::draw_indexed(UINT index_count, UINT index_offset, UINT vertex_offset, UINT instance_count, UINT instance_offset)
	{
		if (instance_count == 0) return;

		flush_transitions();
		flush_binds();
		m_commandList->DrawIndexedInstanced(index_count, instance_count, index_offset, vertex_offset, instance_offset);
	}



	void GraphicsContext::set_scissors(sizer_long rect)
	{
		if (rect.left >= rect.right || rect.top >= rect.bottom)
			Log::get() << Log::LEVEL_WARNING << "rect is invalid" << Log::endl;

		m_commandList->RSSetScissorRects(1, reinterpret_cast<D3D12_RECT*>(&rect));
	}

	void GraphicsContext::set_viewports(std::vector<Viewport> viewports)
	{
		this->viewports = viewports;
		m_commandList->RSSetViewports(viewports.size(), viewports.data());
	}

	void GraphicsContext::set_scissor(sizer_long rect)
	{
		if (rect.left >= rect.right || rect.top >= rect.bottom)
			Log::get() << Log::LEVEL_WARNING << "rect is invalid" << Log::endl;

		m_commandList->RSSetScissorRects(1, reinterpret_cast<D3D12_RECT*>(&rect));
	}

	void GraphicsContext::set_viewport(Viewport v)
	{
		this->viewports.resize(1);
		this->viewports[0] = v;
		m_commandList->RSSetViewports(viewports.size(), viewports.data());
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
		m_commandList->RSSetViewports(viewports.size(), viewports.data());
	}

	void  CommandList::update_buffer(Resource::ptr resource, UINT offset, const  char* data, UINT size)
	{
		update_buffer(resource.get(), offset, data, size);
	}
	void  CommandList::update_buffer(Resource* resource, UINT offset, const  char* data, UINT size)
	{
		flush_transitions();
		//     return;
		auto info = place_data(size);
		memcpy(info.resource->get_data() + info.offset, data, size);
		m_commandList->CopyBufferRegion(
			resource->get_native().Get(), offset, info.resource->get_native().Get(), info.offset, size);
	}
	Uploader::UploadInfo Uploader::place_data(UINT64 uploadBufferSize, unsigned int alignment)
	{
		const size_t AlignedSize = Math::AlignUp(uploadBufferSize, alignment);
		resource_offset = Math::AlignUp(resource_offset, alignment);

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
	void  CommandList::update_resource(Resource::ptr resource, UINT first_subresource, UINT sub_count, D3D12_SUBRESOURCE_DATA* data)
	{
		flush_transitions();
		UINT64 uploadBufferSize = GetRequiredIntermediateSize(resource->get_native().Get(), first_subresource, sub_count);
		auto info = place_data(uploadBufferSize, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
		UpdateSubresources(m_commandList.Get(), resource->get_native().Get(), info.resource->get_native().Get(), info.offset, first_subresource, sub_count, data);
	}
	void CommandList::update_texture(Resource::ptr resource, ivec3 offset, ivec3 box, UINT sub_resource, const char* data, UINT row_stride, UINT slice_stride)
	{
		update_texture(resource.get(), offset, box,sub_resource,data,row_stride,slice_stride);
	}

	void CommandList::update_texture(Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, const char* data, UINT row_stride, UINT slice_stride)
	{
		flush_transitions();
		D3D12_RESOURCE_DESC Desc = resource->get_desc();
		int rows_count = box.y;

		if (Desc.Format == DXGI_FORMAT_BC7_UNORM_SRGB || Desc.Format == DXGI_FORMAT_BC7_UNORM)
			rows_count /= 4;

		int res_stride = row_stride + (D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - (row_stride) % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
		int size = res_stride * rows_count * box.z;
		auto info = place_data(size, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
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
				for (UINT z = 0; z < box.z; ++z)
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
			for (UINT z = 0; z < box.z; ++z)
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
		m_commandList->CopyTextureRegion(&Dst, offset.x, offset.y, offset.z, &Src, nullptr);
	}


	void GraphicsContext::set_const_buffer(UINT i, std::shared_ptr<GPUBuffer>& buff)
	{
		m_commandList->SetGraphicsRootConstantBufferView(i, buff->get_gpu_address());
	}

	void  GraphicsContext::set_pipeline(PipelineState::ptr& state)
	{
		set_signature(state->desc.root_signature);

		if (current_pipeline != state)
		{
			m_commandList->SetPipelineState(state->get_native().Get());
			current_pipeline = state;
			current_compute_pipeline = nullptr;
		}
	}

	std::future<bool> CommandList::read_texture(Resource::ptr resource, ivec3 offset, ivec3 box, UINT sub_resource, std::function<void(const char*, UINT64, UINT64, UINT64)> f)
	{
		return read_texture(resource.get(), offset, box, sub_resource, f);
	}
	std::future<bool> CommandList::read_texture(const Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, std::function<void(const char*, UINT64, UINT64, UINT64)> f)
	{
		flush_transitions();
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

		int res_stride = RowSizesInBytes + (D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - (RowSizesInBytes) % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
		int size = res_stride * box.y * box.z;
		auto info = read_data(size);
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
		m_commandList->CopyTextureRegion(&dest, offset.x, offset.y, offset.z, &source, nullptr);
		auto result = std::make_shared<std::promise<bool>>();
		on_execute_funcs.push_back([result, info, f, res_stride, NumRows]()
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

	std::future<bool> CommandList::read_buffer(Resource* resource, unsigned int offset, UINT64 size, std::function<void(const char*, UINT64)> f)
	{
		if (size == 0)
		{
			auto result = std::make_shared<std::promise<bool>>();
			f(nullptr, 0);
			result->set_value(true);
			return result->get_future();
		}

		transition(resource, Render::ResourceState::COPY_SOURCE);
		flush_transitions();
		//  auto size = resource->get_size();
		auto info = read_data(size);
		//  m_commandList->CopyResource(info.resource->get_resource()->get_native().Get(), resource->get_native().Get());
		m_commandList->CopyBufferRegion(info.resource->get_native().Get(), info.offset, resource->get_native().Get(), offset, size);
		auto result = std::make_shared<std::promise<bool>>();
		on_execute_funcs.push_back([result, info, f, size]()
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
	std::future<bool> CommandList::read_query(std::shared_ptr<QueryHeap>& query_heap, unsigned int offset, unsigned int size, std::function<void(const char*, UINT64)> f)
	{
		if (size == 0)
		{
			auto result = std::make_shared<std::promise<bool>>();
			f(nullptr, 0);
			result->set_value(true);
			return result->get_future();
		}

		flush_transitions();
		//  auto size = resource->get_size();
		auto info = read_data(size);
		//  m_commandList->CopyResource(info.resource->get_resource()->get_native().Get(), resource->get_native().Get());
		m_commandList->ResolveQueryData(query_heap->get_native(), D3D12_QUERY_TYPE_PIPELINE_STATISTICS, 0, 1, info.resource->get_native().Get(), info.offset);
		auto result = std::make_shared<std::promise<bool>>();
		on_execute_funcs.push_back([result, info, f, size]()
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
		for (auto && t : on_execute_funcs)
			t();
		descriptor_manager_graphics.reset();
		descriptor_manager_compute.reset();
		on_execute_funcs.clear();
		//	on_send_funcs.clear();
		BufferCache::get().on_execute_list(this);
	}

	void Transitions::flush_transitions()
	{
		if (transition_count > 0)
		{
		/*	for (int i = 0; i < transition_count; i++)
			{
				if (transitions[i].Transition.StateBefore == 0xCDCDCDCD) 
					assert(false);

			}*/
			m_commandList->ResourceBarrier(transition_count, transitions.data());
			transition_count = 0;
		}
	}
	std::shared_ptr<TransitionCommandList> Transitions::fix_pretransitions()
	{

		std::vector<D3D12_RESOURCE_BARRIER> result;


		for (auto &r : used_resources)
		{
			auto gpu_state = r->get_gpu_state();
			auto cpu_state = r->get_start_state(id,global_id);

			
			if (cpu_state != gpu_state)
			{
				result.emplace_back(CD3DX12_RESOURCE_BARRIER::Transition(r->get_native().Get(),
					static_cast<D3D12_RESOURCE_STATES>(gpu_state),
					static_cast<D3D12_RESOURCE_STATES>(cpu_state),
					D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));

				assert(!(cpu_state&D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
				assert(!(gpu_state &D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
				assert(!(r->get_end_state(id, global_id)&D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

			}

			r->assume_gpu_state(r->get_end_state(id, global_id));
		}

		if (result.size())
		{
			if (!transition_list)
				transition_list.reset(new TransitionCommandList());

			transition_list->create_transition_list(result);
			return transition_list;
		}
		return nullptr;
	}
/*	void Transitions::assume_state(Resource* resource, ResourceState state)
	{
		resource->assume_state(id, state);
	}
	void Transitions::assume_state(const Resource::ptr& resource, ResourceState state)
	{
		resource->assume_state(id, state);
	}*/

	void Transitions::transition(const std::shared_ptr<Texture>& resource, unsigned int to, UINT subres )
	{
		transition(resource.get(), to);
	}
	void Transitions::transition(const Resource::ptr& resource, unsigned int to, UINT subres )
	{
		transition(resource.get(), to);
	}
	void Transitions::transition(const Resource* resource, unsigned int to, UINT subres )
	{

		if (resource->is_new(id, global_id))
		{
			used_resources.emplace_back(const_cast<Resource*>(resource));
		}
	
		auto prev_state = resource->transition(to, subres, id, global_id);
		if (prev_state == ResourceState::UNKNOWN) return;

		transitions[transition_count++] = CD3DX12_RESOURCE_BARRIER::Transition(resource->get_native().Get(),
			static_cast<D3D12_RESOURCE_STATES>(prev_state),
			static_cast<D3D12_RESOURCE_STATES>(to),
			subres);
	
		if (transition_count == transitions.size())
			flush_transitions();
	}

	void Transitions::transition(Resource* from, Resource* to)
	{
		transitions[transition_count++] = CD3DX12_RESOURCE_BARRIER::Aliasing(from->get_native().Get(), to->get_native().Get());

		if (transition_count == transitions.size())
			flush_transitions();
	}

	void Transitions::transition_uav(Resource* resource)
	{
		transitions[transition_count++] = CD3DX12_RESOURCE_BARRIER::UAV(resource->get_native().Get());

		if (transition_count == transitions.size())
			flush_transitions();
	}
	void 	 CommandList::copy_buffer(Resource* dest, int s_dest, Resource* source, int s_source, int size)

	{
		transition(source, Render::ResourceState::COPY_SOURCE);
		transition(dest, Render::ResourceState::COPY_DEST);
		flush_transitions();
		m_commandList->CopyBufferRegion(dest->get_native().Get(), s_dest, source->get_native().Get(), s_source, size);
	}
	void CommandList::copy_resource(Resource* dest, Resource* source)
	{
		transition(source, Render::ResourceState::COPY_SOURCE);
		transition(dest, Render::ResourceState::COPY_DEST);
		flush_transitions();
		m_commandList->CopyResource(dest->get_native().Get(), source->get_native().Get());
	}
	void CommandList::copy_resource(const Resource::ptr& dest, const Resource::ptr& source)
	{
		transition(source, Render::ResourceState::COPY_SOURCE);
		transition(dest, Render::ResourceState::COPY_DEST);
		flush_transitions();
		m_commandList->CopyResource(dest->get_native().Get(), source->get_native().Get());
	}
	void CommandList::copy_texture(const Resource::ptr& dest, int dest_subres, const Resource::ptr& source, int source_subres)
	{
		transition(source, Render::ResourceState::COPY_SOURCE);
		transition(dest, Render::ResourceState::COPY_DEST);
		flush_transitions();
		CD3DX12_TEXTURE_COPY_LOCATION Dst(dest->get_native().Get(), dest_subres);
		CD3DX12_TEXTURE_COPY_LOCATION Src(source->get_native().Get(), source_subres);
		m_commandList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
	}

	void CommandList::copy_texture(const Resource::ptr& from, ivec3 from_pos, const Resource::ptr& to, ivec3 to_pos, ivec3 size)
	{
		transition(from, Render::ResourceState::COPY_SOURCE);
		transition(to, Render::ResourceState::COPY_DEST);
		flush_transitions();
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
		m_commandList->CopyTextureRegion(&Dst, to_pos.x, to_pos.y, to_pos.z, &Src, &box);
	}



	GraphicsContext& CommandList::get_graphics()
	{
		return reinterpret_cast<GraphicsContext&>(*this);
	}

	ComputeContext& CommandList::get_compute()
	{
		return reinterpret_cast<ComputeContext&>(*this);
	}

	std::shared_ptr<UploadBuffer> BufferCache::get_upload(UINT size)
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
		return std::make_shared<UploadBuffer>(size);
		//     upload_resources.emplace_back(new BufferBase(size, HeapType::UPLOAD, ResourceState::GEN_READ));
	}

	std::shared_ptr<CPUBuffer> BufferCache::get_readback(UINT size)
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
		//return nullptr;
		return std::make_shared<CPUBuffer>(size, 1);
	}



	void BufferCache::on_execute_list(CommandList* list)
	{
		{
			std::lock_guard<std::mutex> m(upload);

			for (auto && e : list->upload_resources)
				upload_resources.insert(std::make_pair(e->get_size(), e));
		}
		{
			std::lock_guard<std::mutex> m(read_back);

			for (auto && e : list->read_back_resources)
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
			m_commandList->SetComputeRootSignature(s->get_native().Get());
			descriptor_manager_compute.parse(current_compute_root_signature);
		}
	}
void ComputeContext::dispach(int x,int y,int z)
	{
		flush_transitions();
		flush_binds();
		m_commandList->Dispatch(x, y, z);
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

	void  ComputeContext::set_dynamic(UINT i, UINT offset, const Handle& table)
	{
		descriptor_manager_compute.set(i, offset, table);
	}

	void  GraphicsContext::set_dynamic(UINT i, UINT offset, const Handle& table)
	{
		descriptor_manager_graphics.set(i, offset, table);
	}

	void  GraphicsContext::set_dynamic(UINT i, UINT offset, const HandleTable& table)
	{
		descriptor_manager_graphics.set(i, offset, table);
	}

	void  ComputeContext::set_dynamic(UINT i, UINT offset, const HandleTable& table)
	{
		descriptor_manager_compute.set(i, offset, table);
	}

	/*  void  ComputeContext::set(UINT i, const Handle& table)
	  {
		  m_commandList->SetComputeRootDescriptorTable(i, table.gpu);
	  }*/
	void  ComputeContext::set_table(UINT i, const Handle& table)
	{
		m_commandList->SetComputeRootDescriptorTable(i, table.gpu);
	}
	void  ComputeContext::set_pipeline(ComputePipelineState::ptr& state)
	{
		if(state->desc.root_signature)
		set_signature(state->desc.root_signature);

		if (state != current_compute_pipeline)
		{
			m_commandList->SetPipelineState(state->get_native().Get());
			current_compute_pipeline = state;
			current_pipeline = nullptr;
		}
	}
	void  ComputeContext::set_const_buffer(UINT i, std::shared_ptr<GPUBuffer> buff)
	{
		m_commandList->SetComputeRootConstantBufferView(i, buff->get_gpu_address());
	}
	/*  void ComputeContext::set_constants(UINT i, float v1)
	  {
		  m_commandList->SetComputeRoot32BitConstant(i, *reinterpret_cast<UINT*>(&v1), 0);
	  }
	  void ComputeContext::set_constants(UINT i, float v1, float v2)
	  {
		  m_commandList->SetComputeRoot32BitConstant(i, *reinterpret_cast<UINT*>(&v1), 0);
		  m_commandList->SetComputeRoot32BitConstant(i, *reinterpret_cast<UINT*>(&v2), 1);
	  }*/
	void Eventer::on_start(Timer* timer)
	{
		names.push_back(timer->get_block().get_name());
		start_event(names.back().c_str());

		for (auto& c : timer->get_block().get_childs())
		{
			GPUBlock * b = dynamic_cast<GPUBlock*>(c.get());
			if(b)
			{
				b->gpu_counter.enabled = false;
			}
		}
	//		static_cast<GPUBlock*>(c.get())

		GPUBlock * b = dynamic_cast<GPUBlock*>(&timer->get_block());
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

		GPUBlock * b = dynamic_cast<GPUBlock*>(&timer->get_block());
		if (b)
		{
			b->gpu_counter.timer.end(this);
		}

		current = timer->get_block().get_parent().get();
		end_event();
	}

	void Eventer::reset()
	{
		timer.reset();
	}

	void Eventer::begin(std::string name, Timer*t )
	{

		current = t ? &t->get_block() : &Profiler::get();
		TimedRoot::parent = t ? t->get_root(): &Profiler::get();
		if (name.size())
		{
			timer.reset(new Timer(start(convert(name).c_str())));
		}

		else
			current = nullptr;


		names.clear();
	}

	Timer  Eventer::start(const wchar_t* name)
	{
		if (Profiler::get().enabled)
			return std::move(Timer(&current->get_child<GPUBlock>(std::wstring(name)), this));
		else return Timer(nullptr, nullptr);
		//return std::move(Timer(nullptr,nullptr));
		
	}


	void Eventer::start_event(std::wstring str)
	{
		//	::PIXBeginEvent(m_commandList.Get(), 0, str.c_str());
	}

	void Eventer::end_event()
	{
		//	::PIXEndEvent(m_commandList.Get());
	}

	void Eventer::set_marker(const wchar_t* label)
	{
		// ::PIXSetMarker(m_commandList.Get(), 0, label);
	}

	FrameResources::UploadInfo FrameResources::place_data(std::uint64_t uploadBufferSize, unsigned int alignment)
	{
		std::lock_guard<std::mutex> g(m);
		const size_t AlignedSize = uploadBufferSize;
		resource_offset = Math::AlignUp(resource_offset, alignment);

		if (upload_resources.empty() || (resource_offset + uploadBufferSize > upload_resources.back()->get_size()))
		{
			upload_resources.push_back(BufferCache::get().get_upload(std::max(heap_size, AlignedSize)));
			resource_offset = 0;
		}

		UploadInfo info;
		info.resource = upload_resources.back();
		info.offset = resource_offset;
		resource_offset += AlignedSize;
		return info;
	}


	void BufferCache::unused_upload(std::vector<std::shared_ptr<UploadBuffer>>& upload_list)
	{
		std::lock_guard<std::mutex> m(upload);

		for (auto && e : upload_list)
			upload_resources.insert(std::make_pair(e->get_size(), e));

		upload_list.clear();
	}

	void GraphicsContext::set_const_buffer(UINT i, const FrameResources::UploadInfo& info)
	{
		m_commandList->SetGraphicsRootConstantBufferView(i, info.resource->get_gpu_address() + info.offset);
	}


	std::shared_ptr<CommandList> FrameResourceManager::start_frame(std::shared_ptr<CommandList> list, std::string name)
	{
		std::lock_guard<std::mutex> g(m);
		frame_number++;
		frames.emplace_back(new FrameResources);
		current = frames.back().get();
		frames.back()->frame_number = frame_number;
		//	auto list = Render::Device::get().get_queue(Render::CommandListType::DIRECT)->get_free_list();
		list->begin(name);
		list->frame_resources = frames.back().get();
		list->on_execute_funcs.emplace_back([this]()
		{
			end_frame();
		});
		return list;
	}

	std::shared_ptr<CommandList> FrameResourceManager::set_frame(std::shared_ptr<CommandList> list, std::string name)
	{
		std::lock_guard<std::mutex> g(m);
		frame_number++;
		frames.emplace_back(new FrameResources);
		current = frames.back().get();
		frames.back()->frame_number = frame_number;
		//	auto list = Render::Device::get().get_queue(Render::CommandListType::DIRECT)->get_free_list();
	//	list->begin(name);
		list->frame_resources = frames.back().get();
		list->on_execute_funcs.emplace_back([this]()
		{
			end_frame();
		});
		return list;
	}

	std::shared_ptr<CommandList> FrameResourceManager::start_frame(std::string name)
	{
		std::lock_guard<std::mutex> g(m);
		frame_number++;
		frames.emplace_back(new FrameResources);
		current = frames.back().get();
		frames.back()->frame_number = frame_number;
		auto list = Render::Device::get().get_queue(Render::CommandListType::DIRECT)->get_free_list();
		list->begin(name);
		list->frame_resources = frames.back().get();
		list->on_execute_funcs.emplace_back([this]()
		{
			end_frame();
		});
		return list;
	}
	void  ComputeContext::set_const_buffer(UINT i, const FrameResources::UploadInfo& info)
	{
		m_commandList->SetComputeRootConstantBufferView(i, info.resource->get_gpu_address() + info.offset);
	}


	void ComputeContext::set(UINT i, FrameResource& resource)
	{
		set_const_buffer(i, resource.get_for(*frame_resources));
	}


	void GraphicsContext::set(UINT i, FrameResource& resource)
	{
		set_const_buffer(i, resource.get_for(*frame_resources));
	}


	void GraphicsContext::set_srv(UINT i, FrameResource& resource)
	{
		auto info = resource.get_for(*frame_resources);
		m_commandList->SetGraphicsRootShaderResourceView(i, info.get_gpu_address());
	}

	CommandList::ptr CommandList::get_sub_list()
	{
		auto list = Render::Device::get().get_queue(Render::CommandListType::DIRECT)->get_free_list();
		list->frame_resources = frame_resources;
		list->set_parent( get_ptr());
//		wait_for_execution_count++;
		//list->on_send_funcs.emplace_back([this]() {wait_for_execution_count--; });
		return list;
	}


	void  ComputeContext::set_srv(UINT i, const D3D12_GPU_VIRTUAL_ADDRESS& table)
	{
		m_commandList->SetComputeRootShaderResourceView(i, table);
	}

	void  ComputeContext::set_uav(UINT i, const D3D12_GPU_VIRTUAL_ADDRESS& table)
	{
		m_commandList->SetComputeRootUnorderedAccessView(i, table);
	}

	D3D12_GPU_VIRTUAL_ADDRESS FrameResources::UploadInfo::get_gpu_address()
	{
		return resource->get_gpu_address() + offset;
	}

	void  GraphicsContext::set_srv(UINT i, const D3D12_GPU_VIRTUAL_ADDRESS& table)
	{
		m_commandList->SetGraphicsRootShaderResourceView(i, table);
	}

	void ComputeContext::flush_binds(bool force)
	{
		descriptor_manager_compute.bind(this, [this](int i, Handle t)
		{
			set_table(i, t);
		});

		flush_heaps(force);
	}
	void GraphicsContext::execute_indirect(ComPtr<ID3D12CommandSignature> command_types, UINT max_commands, Resource* command_buffer, UINT64 command_offset, Resource* counter_buffer, UINT64 counter_offset)
	{
		flush_transitions();
		  flush_binds(true);
		 //  get_compute().flush_binds();

		descriptor_manager_graphics.bind(this);
	//	descriptor_manager_compute.bind(this);
		m_commandList->ExecuteIndirect(
			command_types.Get(),
			max_commands,
			command_buffer ? command_buffer->get_native().Get() : nullptr,
			command_offset,
			counter_buffer ? counter_buffer->get_native().Get() : nullptr,
			counter_offset);
	}
	void ComputeContext::execute_indirect(ComPtr<ID3D12CommandSignature> command_types, UINT max_commands, Resource* command_buffer, UINT64 command_offset, Resource* counter_buffer, UINT64 counter_offset)
	{
		flush_transitions();
		flush_binds(true);
		//  get_compute().flush_binds();

		//descriptor_manager_graphics.bind(this);
		descriptor_manager_compute.bind(this);
		m_commandList->ExecuteIndirect(
			command_types.Get(),
			max_commands,
			command_buffer ? command_buffer->get_native().Get() : nullptr,
			command_offset,
			counter_buffer ? counter_buffer->get_native().Get() : nullptr,
			counter_offset);
	}

	void Transitions::reset()
	{
		transition_count = 0;
		used_resources.clear();
	}
	void Uploader::reset()
	{
		if (upload_resources.size())
			upload_resources.clear();
		resource_offset = 0;
	}
	void Readbacker::reset()
	{
	

	}



	TransitionCommandList::TransitionCommandList()
	{
		D3D12_COMMAND_LIST_TYPE t = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;

		Device::get().get_native_device()->CreateCommandAllocator(t, IID_PPV_ARGS(&m_commandAllocator));
		Device::get().get_native_device()->CreateCommandList(0, t, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList));
		m_commandList->Close();

	}

	void TransitionCommandList::create_transition_list(const std::vector<D3D12_RESOURCE_BARRIER> &transitions)
	{
		m_commandAllocator->Reset();
		m_commandList->Reset(m_commandAllocator.Get(), nullptr);
		m_commandList->ResourceBarrier(transitions.size(), transitions.data());
		m_commandList->Close();
	}

	ComPtr<ID3D12GraphicsCommandList> TransitionCommandList::get_native()
	{
		return m_commandList;
	}
}