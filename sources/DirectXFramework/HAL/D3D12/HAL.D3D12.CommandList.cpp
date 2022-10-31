module;

import d3d12;
#include <pix3.h>

module HAL:API.CommandList;


import Debug;
import Exceptions;
import Profiling;

import :Private.CommandListCompiler;
import HAL;

//using namespace HAL;
namespace HAL

{
	namespace API
	{
//	GPUTimer::GPUTimer()
//	{
//		id = Singleton<Graphics::GPUTimeManager>::get().get_id();
//	}
//
//	GPUTimer::~GPUTimer()
//	{
//		Singleton<Graphics::GPUTimeManager>::get().put_id(id);
//	}
//
//	void GPUTimer::start(Eventer* list)
//	{
//		Singleton<Graphics::GPUTimeManager>::get().start(*this, list);
//	}
//
//	void GPUTimer::end(Eventer* list)
//	{
//		Singleton<Graphics::GPUTimeManager>::get().end(*this, list);
//		list = nullptr;
//	}
//
//	float GPUTimer::get_time()
//	{
//		return Singleton<Graphics::GPUTimeManager>::get().get_time(*this);
//	}
//
//	double GPUTimer::get_start()
//	{
//		return Singleton<Graphics::GPUTimeManager>::get().get_start(*this);
//	}
//
//	double GPUTimer::get_end()
//	{
//		return Singleton<Graphics::GPUTimeManager>::get().get_end(*this);
//	}
//	QueryHeap::QueryHeap(UINT max_count, D3D12_QUERY_HEAP_TYPE type)
//	{
//		D3D12_QUERY_HEAP_DESC QueryHeapDesc;
//		QueryHeapDesc.Count = max_count;
//		QueryHeapDesc.NodeMask = 1;
//		QueryHeapDesc.Type = type;
//		HAL::Device::get().get_native_device()->CreateQueryHeap(&QueryHeapDesc, IID_PPV_ARGS(&heap));
//		heap->SetName(L"QueryHeap");
//	}
//
//	ComPtr<ID3D12QueryHeap> QueryHeap::get_native()
//	{
//		return heap;
//	}
//
//	CommandList::CommandList(CommandListType type)
//
//	{
//		this->type = type;
//
//		compiler.create(type);
//
//		if (type == CommandListType::DIRECT || type == CommandListType::COMPUTE)
//			compute.reset(new ComputeContext(*this));
//
//		//	if (type == CommandListType::DIRECT || type == CommandListType::COPY)
//		copy.reset(new CopyContext(*this));
//
//		if (type == CommandListType::DIRECT)
//			graphics.reset(new GraphicsContext(*this));
//
//		compiler.SetName(L"SpectrumCommandList");
//
//
//		//debug_buffer = std::make_shared<Graphics::StructureBuffer<Table::DebugStruct>>(64, Graphics::counterType::NONE, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);
//	}
//
//	void CommandList::setup_debug(SignatureDataSetter* setter)
//	{
//		//if (!current_pipeline->debuggable) return;
//		//Slots::DebugInfo info;
//		//Graphics::StructureBuffer<Table::DebugStruct>* structured = static_cast<Graphics::StructureBuffer<Table::DebugStruct>*>(debug_buffer.get());
//		//info.GetDebug() = structured->rwStructuredBuffer;
//		//info.set(*setter);
//
//	}
//
//	void CommandList::print_debug()
//	{
//		/*if (!current_pipeline->debuggable) return;
//
//		auto pso_name = current_pipeline->name;
//		get_copy().read_buffer(debug_buffer.get(), 0, 3 * sizeof(Table::DebugStruct), [this, pso_name](const char* data, UINT64 size)
//			{
//
//				LogBlock block(Log::get(), log_level_internal::level_all);
//
//				if (first_debug_log)
//				{
//					block << "-----------------------------------------\n";
//
//					first_debug_log = false;
//				}
//				auto result = reinterpret_cast<const Table::DebugStruct*>(data);
//
//				block << "DEBUG(" << name << "): " << pso_name << "\n";
//				for (int i = 0; i < 3; i++)
//				{
//					block << "debug(" << i << "): " << result[i].v.x << " " << result[i].v.y << " " << result[i].v.z << " " << result[i].v.w << " " << "\n";
//				}
//				Log::get() << block;
//			});
//
//		StructureBuffer<Table::DebugStruct>* structured = static_cast<StructureBuffer<Table::DebugStruct>*>(debug_buffer.get());
//		clear_uav(structured->rwByteAddressBuffer);*/
//	}
//
//
//	void CommandList::begin(std::string name, Timer* t)
//	{
//		if (name.empty())
//		{
//			compiler.SetName(L"EmptyName");
//		}
//		else
//			compiler.SetName(convert(name).c_str());
//		compiled = HAL::Private::CommandListCompiled();
//#ifdef DEV
//		begin_stack = Exceptions::get_stack_trace();
//#endif
//
//		HAL::Device::get().context_generator.generate(this);
//		first_debug_log = true;
//
//
//		//       Log::get() << "begin" << Log::endl;
//		compiler.reset();
//		//resource_index = 0;
//
//		first_pipeline = nullptr;
//
//		if (graphics) graphics->begin();
//		if (compute) compute->begin();
//
//
//		Transitions::begin();
//		Eventer::begin(name, t);
//
//		if (type != CommandListType::COPY) {
//			std::array<ID3D12DescriptorHeap*, 2> heaps;
//
//			heaps[0] = HAL::DescriptorHeapManager::get().get_gpu_heap(DescriptorHeapType::SAMPLER)->get_dx();
//			heaps[1] = HAL::DescriptorHeapManager::get().get_gpu_heap(DescriptorHeapType::CBV_SRV_UAV)->get_dx();
//			compiler.SetDescriptorHeaps(2, heaps.data());
//		}
//	}
//
//
//	void CommandList::end()
//	{
//		//	create_transition_point(false);
//			//	id = -1;
//		Transitions::make_split_barriers();
//		current_pipeline = nullptr;
//
//		if (graphics) graphics->end();
//		if (compute) compute->end();
//
//		Eventer::reset();
//
//	}
//
//	void GraphicsContext::begin()
//	{
//		reset();
//		native_topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
//		reset_tables();
//		index = HAL::Views::IndexBuffer();
//	}
//	void GraphicsContext::end()
//	{
//		//current_root_signature = nullptr;
//	}
//
//	void ComputeContext::begin()
//	{
//		reset();
//		reset_tables();
//	}
//	void ComputeContext::end()
//	{
//		current_compute_root_signature = nullptr;
//	}
//
//	void GraphicsContext::on_execute()
//	{
//
//	}
//
//	void ComputeContext::on_execute()
//	{
//
//	}
//
//	void Sendable::on_done(std::function<void()> f)
//	{
//		on_execute_funcs.emplace_back(f);
//
//	}
//	void Sendable::compile()
//	{
//		compiled = compiler.compile();
//	}
//	std::shared_future<FenceWaiter> Sendable::execute(std::function<void()> f)
//	{
//
//		//TEST(compiler.Close());
//		if (!compiled)
//			compile();
//
//		execute_fence = std::promise<FenceWaiter>();
//		execute_fence_result = execute_fence.get_future();
//		if (f)
//			on_execute_funcs.emplace_back(f);
//
//		Device::get().get_queue(type)->execute(static_cast<CommandList*>(this));
//
//		return execute_fence_result;
//	}
//
//	void Sendable::execute_and_wait(std::function<void()> f)
//	{
//		execute(f).get().wait();
//	}
//
//
//	void Eventer::insert_time(QueryHeap& pQueryHeap, uint32_t QueryIdx)
//	{
//		compiler.EndQuery(pQueryHeap.get_native().Get(), D3D12_QUERY_TYPE_TIMESTAMP, QueryIdx);
//	}
//
//	void Eventer::resolve_times(QueryHeap& pQueryHeap, uint32_t NumQueries, std::function<void(std::span<UINT64>)> f)
//	{
//		CommandList* list = static_cast<CommandList*>(this); // :(
//		auto info = list->read_data(NumQueries * sizeof(UINT64));
//
//		compiler.ResolveQueryData(pQueryHeap.get_native().Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, NumQueries, info.resource->get_dx(), info.resource_offset);
//		on_execute_funcs.emplace_back([info, f, NumQueries]() {
//
//			UINT64* data = reinterpret_cast<UINT64*>(info.get_cpu_data());
//			f(std::span(data, NumQueries));
//
//			});
//	}
//
//	void GraphicsContext::on_set_signature(const RootSignature::ptr& s)
//	{
//		list->SetGraphicsRootSignature(s->get_native().Get());
//	}
//
//	void GraphicsContext::set_heaps(DescriptorHeap::ptr& a, DescriptorHeap::ptr& b)
//	{
//		ID3D12DescriptorHeap* heaps[2] = { 0 };
//
//		if (a)
//			heaps[0] = a->get_dx();
//
//		if (b)
//			heaps[1] = b->get_dx();
//
//		list->SetDescriptorHeaps(b ? 2 : 1, heaps);
//	}
//
//
//	void  GraphicsContext::set(UINT i, const HandleTableLight& table)
//	{
//		assert(false);//list->SetGraphicsRootDescriptorTable(i, table.get_gpu());
//	}
//
//
//	void  GraphicsContext::set_const_buffer(UINT i, const D3D12_GPU_VIRTUAL_ADDRESS& table)
//	{
//		list->SetGraphicsRootConstantBufferView(i, table);
//	}
//
//	void GraphicsContext::set_rtv(const HandleTable& table, Handle h)
//	{
//		set_rtv(table.get_count(), table[0], h);
//	}
//	void GraphicsContext::set_rtv(const HandleTableLight& table, Handle h)
//	{
//		set_rtv(table.get_count(), table, h);
//	}
//
//	void GraphicsContext::set_rtv(int c, Handle rt, Handle h)
//	{
//		base.create_transition_point();
//		for (int i = 0; i < c; i++)
//		{
//			get_base().transition(&rt.get_resource_info()[i]);
//		}
//
//		if (h.is_valid())
//			get_base().transition(h.get_resource_info());
//
//		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv;
//		CD3DX12_CPU_DESCRIPTOR_HANDLE dsv;
//		CD3DX12_CPU_DESCRIPTOR_HANDLE* dsv_ptr = nullptr;
//		CD3DX12_CPU_DESCRIPTOR_HANDLE* rtv_ptr = nullptr;
//
//		if (rt.is_valid())
//		{
//			rtv = rt.get_cpu();
//			rtv_ptr = &rtv;
//		}
//
//
//		if (h.is_valid())
//		{
//			dsv = h.get_cpu();
//			dsv_ptr = &dsv;
//		}
//
//		list->OMSetRenderTargets(c, rtv_ptr, true, dsv_ptr);
//
//		base.create_transition_point(false);
//	}
//
//	void GraphicsContext::set_rtv(std::initializer_list<Handle> rt, Handle h)
//	{
//		set_rtv(place_rtv(rt), h);
//	}
//
//	void GraphicsContext::draw(UINT vertex_count, UINT vertex_offset, UINT instance_count, UINT instance_offset)
//	{
//		PROFILE_GPU(L"draw");
//		base.create_transition_point();
//		base.setup_debug(this);
//
//		commit_tables();
//		list->DrawInstanced(vertex_count, instance_count, vertex_offset, instance_offset);
//		base.create_transition_point(false);
//
//		get_base().print_debug();
//
//	}
//	void GraphicsContext::draw_indexed(UINT index_count, UINT index_offset, UINT vertex_offset, UINT instance_count, UINT instance_offset)
//	{
//		if (instance_count == 0) return;
//		PROFILE_GPU(L"draw_indexed");
//		base.create_transition_point();
//		base.setup_debug(this);
//
//		commit_tables();
//		get_base().transition(static_cast<HAL::Resource*>(index.Resource), ResourceState::INDEX_BUFFER);
//		D3D12_INDEX_BUFFER_VIEW native;
//		native.SizeInBytes = index.Resource ? index.SizeInBytes : 0;
//		native.Format = ::to_native(index.Format);
//		native.BufferLocation = index.Resource ? to_native(static_cast<HAL::Resource*>(index.Resource)->get_resource_address().offset(index.OffsetInBytes)):0;// index.Resource ? static_cast<HAL::Resource*>(index.Resource)->get_resource_address() + index.OffsetInBytes : 0;
//		list->IASetIndexBuffer(&native);
//
//
//		list->DrawIndexedInstanced(index_count, instance_count, index_offset, vertex_offset, instance_offset);
//		base.create_transition_point(false);
//		get_base().print_debug();
//	}
//	void GraphicsContext::dispatch_mesh(D3D12_DISPATCH_MESH_ARGUMENTS args)
//	{
//		dispatch_mesh(ivec3{ args.ThreadGroupCountX, args.ThreadGroupCountY, args.ThreadGroupCountZ });
//	}
//
//	void GraphicsContext::dispatch_mesh(ivec3 v)
//	{
//
//		PROFILE_GPU(L"dispatch_mesh");
//		base.create_transition_point();
//		base.setup_debug(this);
//
//		commit_tables();
//		//get_base().transition(index.resource, ResourceState::INDEX_BUFFER);
//		//list->IASetIndexBuffer(&index.view);
//
//		list->DispatchMesh(v.x, v.y, v.z);
//		base.create_transition_point(false);
//		get_base().print_debug();
//	}
//
//
//	void GraphicsContext::set_scissors(sizer_long rect)
//	{
//		if (rect.left >= rect.right || rect.top >= rect.bottom)
//			Log::get() << Log::LEVEL_WARNING << "rect is invalid" << Log::endl;
//
//		list->RSSetScissorRects(1, reinterpret_cast<D3D12_RECT*>(&rect));
//	}
//
//	void GraphicsContext::set_viewports(std::vector<Viewport> viewports)
//	{
//		this->viewports = viewports;
//
//		std::vector<D3D12_VIEWPORT> vps(viewports.size());
//
//		for(uint i=0;i<vps.size();i++)
//		{
//			vps[i] = to_native(viewports[i]);
//		}
//		list->RSSetViewports(static_cast<UINT>(vps.size()), vps.data());
//	}
//
//	void GraphicsContext::set_scissor(sizer_long rect)
//	{
//		if (rect.left >= rect.right || rect.top >= rect.bottom)
//			Log::get() << Log::LEVEL_WARNING << "rect is invalid" << Log::endl;
//
//		list->RSSetScissorRects(1, reinterpret_cast<D3D12_RECT*>(&rect));
//	}
//
//	void GraphicsContext::set_viewport(Viewport v)
//	{
//		this->viewports.resize(1);
//		this->viewports[0] = v;
//
//		set_viewports(viewports);
//	}
//
//	void GraphicsContext::set_viewport(vec4 v)
//	{
//		this->viewports.resize(1);
//		this->viewports[0].pos.x = v.x;
//		this->viewports[0].pos.y = v.y;
//		this->viewports[0].size.x = v.z;
//		this->viewports[0].size.y = v.w;
//		this->viewports[0].depths = { 0, 1 };
//		set_viewports(viewports);
//	}
//
//	void  CopyContext::update_buffer(Resource::ptr resource, UINT offset, const  char* data, UINT size)
//	{
//		update_buffer(resource.get(), offset, data, size);
//	}
//	void  CopyContext::update_buffer(Resource* resource, UINT offset, const  char* data, UINT size)
//	{
//		base.create_transition_point();
//		//	if (base.type != CommandListType::COPY)
//		base.transition(resource, ResourceState::COPY_DEST);
//
//		auto info = base.place_data(size);
//		memcpy(info.get_cpu_data(), data, size);
//		list->CopyBufferRegion(
//			resource->get_dx(), offset, info.resource->get_dx(), info.resource_offset, size);
//		base.create_transition_point(false);
//	}
//
//
//	
//	void CopyContext::update_texture(Resource::ptr resource, ivec3 offset, ivec3 box, UINT sub_resource, const char* data, UINT row_stride, UINT slice_stride)
//	{
//		update_texture(resource.get(), offset, box, sub_resource, data, row_stride, slice_stride);
//	}
//
//	void CopyContext::update_texture(Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, const char* data, UINT row_stride, UINT slice_stride)
//	{
//		base.create_transition_point();
//
//		base.transition(resource, ResourceState::COPY_DEST);
//
//		D3D12_RESOURCE_DESC Desc = ::to_native(resource->get_desc());
//		UINT rows_count = box.y;
//
//		if (Desc.Format == DXGI_FORMAT_BC7_UNORM_SRGB || Desc.Format == DXGI_FORMAT_BC7_UNORM)
//			rows_count /= 4;
//		assert(row_stride >= box.x * from_native(Desc.Format).size() / 8);
//		int res_stride = Math::AlignUp(row_stride, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
//		int size = res_stride * rows_count * box.z;
//		const auto info = base.place_data(size, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
//		UINT64 RequiredSize = 0;
//		D3D12_PLACED_SUBRESOURCE_FOOTPRINT Layouts;
//		UINT NumRows;
//		UINT64 RowSizesInBytes;
//		HAL::Device::get().get_native_device()->GetCopyableFootprints(&Desc, sub_resource, 1, info.resource_offset, &Layouts, &NumRows, &RowSizesInBytes, &RequiredSize);
//
//		if (res_stride == row_stride)
//		{
//			if (slice_stride == 0 || slice_stride == row_stride * rows_count)
//			{
//				auto pDestSlice = info.get_cpu_data();
//				const std::byte* pSrcSlice = reinterpret_cast<const std::byte*>(data);
//				memcpy(pDestSlice,
//					pSrcSlice,
//					size);
//			}
//
//			else
//			{
//				for (UINT z = 0; z < static_cast<UINT>(box.z); ++z)
//				{
//					auto pDestSlice = info.get_cpu_data() + (res_stride * rows_count) * z;
//					const std::byte* pSrcSlice = reinterpret_cast<const std::byte*>(data) + slice_stride * z;
//					memcpy(pDestSlice,
//						pSrcSlice,
//						row_stride * rows_count);
//				}
//			}
//		}
//
//		else
//			for (UINT z = 0; z < static_cast<UINT>(box.z); ++z)
//			{
//				std::byte* pDestSlice = info.get_cpu_data() + (res_stride * rows_count) * z;
//				const std::byte* pSrcSlice = reinterpret_cast<const std::byte*>(data) + slice_stride * z;
//
//				for (UINT y = 0; y < rows_count; ++y)
//				{
//					memcpy(pDestSlice + res_stride * y,
//						pSrcSlice + row_stride * y,
//						row_stride);
//				}
//			}
//
//
//		assert(box.z > 0);
//		CD3DX12_TEXTURE_COPY_LOCATION Dst(resource->get_dx(), sub_resource);
//		CD3DX12_TEXTURE_COPY_LOCATION Src;
//		Src.pResource = info.resource->get_dx();
//		Src.Type = D3D12_TEXTURE_COPY_TYPE::D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
//		Src.PlacedFootprint.Offset = info.resource_offset;
//		Src.PlacedFootprint.Footprint.Width = box.x;
//		Src.PlacedFootprint.Footprint.Height = box.y;
//		Src.PlacedFootprint.Footprint.Depth = box.z;
//		Src.PlacedFootprint.Footprint.RowPitch = res_stride;
//		Src.PlacedFootprint.Footprint.Format = Layouts.Footprint.Format;
//		list->CopyTextureRegion(&Dst, offset.x, offset.y, offset.z, &Src, nullptr);
//		if constexpr (BuildOptions::Debug)	TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->GetDeviceRemovedReason());
//		base.create_transition_point(false);
//	}
//	/*
//	void  GraphicsContext::set_pipeline(PipelineState::ptr state)
//	{
//		set_signature(state->desc.root_signature);
//		base.set_pipeline_internal(state.get());
//	}
//	*/
//	//void SignatureDataSetter::set_layout(Layouts layout)
//	//{
//	//	set_signature(Graphics::get_Signature(layout));
//	//}
//	std::future<bool> CopyContext::read_texture(Resource::ptr resource, ivec3 offset, ivec3 box, UINT sub_resource, std::function<void(const char*, UINT64, UINT64, UINT64)> f)
//	{
//		return read_texture(resource.get(), offset, box, sub_resource, f);
//	}
//	std::future<bool> CopyContext::read_texture(const Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, std::function<void(const char*, UINT64, UINT64, UINT64)> f)
//	{
//
//
//		UINT64 RequiredSize = 0;
//		D3D12_PLACED_SUBRESOURCE_FOOTPRINT Layouts;
//		UINT NumRows;
//		UINT64 RowSizesInBytes;
//		D3D12_RESOURCE_DESC Desc = ::to_native(resource->get_desc());
//		HAL::Device::get().get_native_device()->GetCopyableFootprints(&Desc, sub_resource, 1, 0, &Layouts, &NumRows, &RowSizesInBytes, &RequiredSize);
//
//		if (!RequiredSize)
//		{
//			std::promise<bool> str;
//			str.set_value(false);
//			return str.get_future();
//		}
//
//
//		base.create_transition_point();
//		//	if (base.type != CommandListType::COPY)
//		base.transition(resource, ResourceState::COPY_SOURCE);
//		//else
//	//		base.transition(resource, ResourceState::COMMON);
//		UINT64 res_stride = Math::AlignUp(RowSizesInBytes, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
//		UINT64 size = res_stride * box.y * box.z;
//		auto info = base.read_data(size, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
//		CD3DX12_TEXTURE_COPY_LOCATION source(resource->get_dx(), sub_resource);
//		CD3DX12_TEXTURE_COPY_LOCATION dest;
//
//		assert(box.z > 0);
//		dest.pResource = info.resource->get_dx();
//		dest.Type = D3D12_TEXTURE_COPY_TYPE::D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
//		dest.PlacedFootprint.Offset = info.resource_offset;
//		dest.PlacedFootprint.Footprint.Width = box.x;
//		dest.PlacedFootprint.Footprint.Height = box.y;
//		dest.PlacedFootprint.Footprint.Depth = box.z;
//		dest.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(res_stride);
//		dest.PlacedFootprint.Footprint.Format = ::to_native(from_native(Layouts.Footprint.Format).to_srv());
//		list->CopyTextureRegion(&dest, offset.x, offset.y, offset.z, &source, nullptr);
//
//		if constexpr (BuildOptions::Debug)	TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->GetDeviceRemovedReason());
//		auto result = std::make_shared<std::promise<bool>>();
//		base.on_execute_funcs.push_back([result, info, f, res_stride, NumRows]()
//			{
//				f(reinterpret_cast<char*>(info.get_cpu_data()), res_stride, res_stride * NumRows, info.resource->get_size());
//				result->set_value(true);
//			});
//
//		base.create_transition_point(false);
//		return result->get_future();
//	}
//
//	std::future<bool> CopyContext::read_buffer(Resource* resource, unsigned int offset, UINT64 size, std::function<void(const char*, UINT64)> f)
//	{
//
//		auto result = std::make_shared<std::promise<bool>>();
//
//		if (size == 0)
//		{
//			f(nullptr, 0);
//			result->set_value(true);
//			return result->get_future();
//		}
//
//		base.create_transition_point();
//
//		base.transition(resource, ResourceState::COPY_SOURCE);
//
//		//  auto size = resource->get_size();
//		auto info = base.read_data(size);
//		//  compiler.CopyResource(info.resource->get_resource()->get_native().Get(), resource->get_native().Get());
//		list->CopyBufferRegion(info.resource->get_dx(), info.resource_offset, resource->get_dx(), offset, size);
//		if constexpr (BuildOptions::Debug)	TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->GetDeviceRemovedReason());
//		base.on_execute_funcs.push_back([result, info, f, size]()
//			{
//				f(reinterpret_cast<char*>(info.get_cpu_data()), size);
//				result->set_value(true);
//			});
//
//		base.create_transition_point(false);
//		return result->get_future();
//	}
//
//	std::future<bool> CopyContext::read_query(std::shared_ptr<QueryHeap>& query_heap, unsigned int offset, unsigned int size, std::function<void(const char*, UINT64)> f)
//	{
//		if (size == 0)
//		{
//			auto result = std::make_shared<std::promise<bool>>();
//			f(nullptr, 0);
//			result->set_value(true);
//			return result->get_future();
//		}
//
//		//  auto size = resource->get_size();
//		auto info = base.read_data(size);
//		//  compiler.CopyResource(info.resource->get_resource()->get_native().Get(), resource->get_native().Get());
//		list->ResolveQueryData(query_heap->get_native().Get(), D3D12_QUERY_TYPE_PIPELINE_STATISTICS, 0, 1, info.resource->get_dx(), info.resource_offset);
//		if constexpr (BuildOptions::Debug)	TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->GetDeviceRemovedReason());
//		auto result = std::make_shared<std::promise<bool>>();
//		base.on_execute_funcs.push_back([result, info, f, size]()
//			{
//				f(reinterpret_cast<char*>(info.get_cpu_data()), size);
//				result->set_value(true);
//			});
//		return result->get_future();
//	}
//
//	void CommandList::on_execute()
//	{
//
//		// Log::get() << "on_execute" << Log::endl;
//		for (auto&& t : on_execute_funcs)
//			t();
//
//		//	srv.reset();
//		//	smp.reset();
//
//		if (graphics) graphics->on_execute();
//		if (compute) compute->on_execute();
//
//
//
//		on_execute_funcs.clear();
//		Uploader::reset();
//		Readbacker::reset();
//
//		GPUCompiledManager::reset();
//		Transitions::on_execute();
//		frame_resources = nullptr;
//
//		free_tracked_objects();
//
//		HAL::Device::get().context_generator.free(this);
//
//	}
//	void Transitions::create_transition_point(bool end)
//	{
//		auto prev_point = transition_points.empty() ? nullptr : &transition_points.back();
//		auto point = &transition_points.emplace_back(type);
//
//		if (prev_point) prev_point->next_point = point;
//		point->prev_point = prev_point;
//
//		point->start = !end;
//
//		if (end)
//		{
//			assert(point->prev_point->start);
//		}
//		compiler.func([point, this](ID3D12GraphicsCommandList4* list)
//			{
//				HAL::Barriers  transitions(type);
//
//				for (auto uav : point->uav_transitions)
//				{
//					transitions.uav(uav);
//				}
//
//				for (auto& uav : point->aliasing)
//				{
//					transitions.alias(nullptr, uav);
//				}
//
//				for (auto& transition : point->transitions)
//				{
//					auto prev_transition = transition.prev_transition;
//
//					if (!prev_transition) continue;
//
//					if (prev_transition->wanted_state == transition.wanted_state) continue;
//
//					//					assert(!point->start);
//					transitions.transition(transition.resource,
//						prev_transition->wanted_state,
//						transition.wanted_state,
//						transition.subres, transition.flags);
//				}
//
//				auto& native_transitions = transitions.get_native();
//				if (!native_transitions.empty())
//				{
//					list->ResourceBarrier((UINT)native_transitions.size(), native_transitions.data());
//				}
//
//				{
//
//					auto& native_transitions = point->compiled_transitions.get_native();
//					if (!native_transitions.empty())
//					{
//						list->ResourceBarrier((UINT)native_transitions.size(), native_transitions.data());
//					}
//				}
//
//			});
//	}
//
//
//	void Transitions::make_split_barriers()
//	{
//		return;
//		for (auto& point : transition_points)
//		{
//			for (auto& transition : point.transitions)
//			{
//				auto prev_transition = transition.prev_transition;
//
//				if (!prev_transition) continue;
//
//				if (prev_transition->wanted_state == transition.wanted_state) continue;
//
//				auto prev_point = prev_transition->point->next_point;
//
//				assert(prev_point->start);
//				prev_point->compiled_transitions.transition(transition.resource,
//					prev_transition->wanted_state,
//					transition.wanted_state,
//					transition.subres, HAL::BarrierFlags::BEGIN);
//
//				transition.flags = HAL::BarrierFlags::END;
//				/*
//				transitions.transition(transition.resource,
//					prev_transition->wanted_state,
//					transition.wanted_state,
//					transition.subres);*/
//			}
//
//		}
//	}
//
//	std::shared_ptr<TransitionCommandList> Transitions::fix_pretransitions()
//	{
//		PROFILE(L"fix_pretransitions");
//
//		HAL::Barriers result(CommandListType::DIRECT);
//		std::vector<HAL::Resource*> discards;
//
//
//		ResourceState states = ResourceState::COMMON;
//		for (auto& r : used_resources)
//		{
//			states = states | r->get_state_manager().process_transitions(result, discards, this);
//		}
//
//		auto transition_type = GetBestType(states, type);
//
//		if (result)
//		{
//			transition_list = (HAL::Device::get().get_queue(transition_type)->get_transition_list());
//			transition_list->create_transition_list(result, discards);
//			return transition_list;
//		}
//		return nullptr;
//	}
//	void Transitions::merge_transition(Transitions* to, Resource* resource)
//	{
//
//
//		if (resource->get_state_manager().transition(this, to))
//		{
//			track_object(*resource);
//			use_resource(resource);
//		}
//	}
//	void Transitions::prepare_transitions(Transitions* to, bool all)
//	{
//		for (auto& resource : to->used_resources)
//		{
//			bool is_new = !resource->get_state_manager().is_used(this);
//			if ((all && is_new) || (!all && !is_new))
//				if (resource->get_state_manager().transition(this, to))
//				{
//					track_object(*resource);
//					use_resource(resource);
//				}
//		}
//	}
//
//
//	void Transitions::transition(const Resource::ptr& resource, ResourceState to, UINT subres)
//	{
//		transition(resource.get(), to, subres);
//	}
//
//	void Transitions::use_resource(const Resource* resource)
//	{
//
//		auto& state = const_cast<Resource*>(resource)->get_state_manager().get_cpu_state(this);
//		if (!state.used)
//		{
//			state.used = true;
//			used_resources.emplace_back(const_cast<Resource*>(resource));
//		}
//	}
//
//	void Transitions::free_resources()
//	{
//#ifdef DEV
//		CommandList* list = static_cast<CommandList*>(this); // :(
//
//		for (auto r : used_resources)
//		{
//			r->not_used(list);
//		}
//#endif
//	}
//
//	void Transitions::transition(const Resource* resource, ResourceState to, UINT subres)
//	{
//		if (!resource) return;
//
//		if (type == CommandListType::COPY && (to == ResourceState::COPY_DEST || to == ResourceState::COPY_SOURCE))
//			to = ResourceState::COMMON;
//
//
//		track_object(*const_cast<Resource*>(resource));
//
//		CommandList* list = static_cast<CommandList*>(this); // :(
//
//		if (resource->get_heap_type() == HeapType::DEFAULT || resource->get_heap_type() == HeapType::RESERVED)
//		{
//			use_resource(resource);
//			const_cast<Resource*>(resource)->get_state_manager().transition(this, to, subres);
//		}
//	}
//
//	void Transitions::transition(Resource* from, Resource* to)
//	{
//
//		track_object(*to);
//		create_aliasing_transition(to);
//		/*	transitions.emplace_back(CD3DX12_RESOURCE_BARRIER::Aliasing(from ? from->get_native().Get() : nullptr, to->get_native().Get()));
//
//			CommandList* list = static_cast<CommandList*>(this); // :(
//
//
//			if (to->is_new(id, global_id))
//			{
//
//				to->aliasing(id, global_id);
//
//				bool good = std::find(used_resources.begin(), used_resources.end(), to) == used_resources.end();
//
//
//				assert(good);
//
//				used_resources.emplace_back(const_cast<Resource*>(to));
//	#ifdef DEV
//				const_cast<Resource*>(to)->used(list);
//	#endif
//				tracked_resources.emplace_back(const_cast<Resource*>(to)->tracked_info);
//				assert(!to->is_new(id, global_id));
//			}
//			else
//			{
//				auto state = to->get_cpu_state(id, global_id);
//
//				if (check(state & ResourceState::RENDER_TARGET) || check(state & ResourceState::DEPTH_WRITE))
//				{
//			//		flush_transitions();
//				//	get_native_list()->DiscardResource(to->get_native().Get(), nullptr);
//				}
//			}
//
//			//	get_native_list()->DiscardResource(to->get_native().Get(),nullptr);
//			//	if(to->gpu_state.subres[0].state)
//			//	if (transition_count == transitions.size())
//			//		flush_transitions();*/
//	}
//
//	void Transitions::transition_uav(Resource* resource)
//	{
//		track_object(*resource);
//		create_uav_transition(resource);
//	}
//
//	void CopyContext::copy_buffer(Resource* dest, int s_dest, Resource* source, int s_source, int size)
//	{
//		base.create_transition_point();
//		//if (base.type != CommandListType::COPY)
//		{
//			base.transition(source, ResourceState::COPY_SOURCE);
//			base.transition(dest, ResourceState::COPY_DEST);
//		}
//
//		list->CopyBufferRegion(dest->get_dx(), s_dest, source->get_dx(), s_source, size);
//		base.create_transition_point(false);
//	}
//	void CopyContext::copy_resource(Resource* dest, Resource* source)
//	{
//		base.create_transition_point();
//		//	if (base.type != CommandListType::COPY)
//		{
//			base.transition(source, ResourceState::COPY_SOURCE);
//			base.transition(dest, ResourceState::COPY_DEST);
//		}
//		list->CopyResource(dest->get_dx(), source->get_dx());
//		base.create_transition_point(false);
//	}
//	void CopyContext::copy_resource(const Resource::ptr& dest, const Resource::ptr& source)
//	{
//		base.create_transition_point();
//		//if (base.type != CommandListType::COPY)
//		{
//
//
//			base.transition(source, ResourceState::COPY_SOURCE);
//			base.transition(dest, ResourceState::COPY_DEST);
//		}
//
//		list->CopyResource(dest->get_dx(), source->get_dx());
//		base.create_transition_point(false);
//	}
//	void CopyContext::copy_texture(const Resource::ptr& dest, int dest_subres, const Resource::ptr& source, int source_subres)
//	{
//		base.create_transition_point();
//		//if (base.type != CommandListType::COPY) 
//		{
//			base.transition(source, ResourceState::COPY_SOURCE, source_subres);
//			base.transition(dest, ResourceState::COPY_DEST, dest_subres);
//		}
//
//		CD3DX12_TEXTURE_COPY_LOCATION Dst(dest->get_dx(), dest_subres);
//		CD3DX12_TEXTURE_COPY_LOCATION Src(source->get_dx(), source_subres);
//		list->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
//		if constexpr (BuildOptions::Debug)	TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->GetDeviceRemovedReason());
//		base.create_transition_point(false);
//	}
//
//	void CopyContext::copy_texture(const Resource::ptr& to, ivec3 to_pos, const Resource::ptr& from, ivec3 from_pos, ivec3 size)
//	{
//		base.create_transition_point();
//		//if (base.type != CommandListType::COPY) 
//		{
//			base.transition(from, ResourceState::COPY_SOURCE);
//			base.transition(to, ResourceState::COPY_DEST);
//		}
//
//
//		CD3DX12_TEXTURE_COPY_LOCATION Dst(to->get_dx(), 0);
//		CD3DX12_TEXTURE_COPY_LOCATION Src(from->get_dx(), 0);
//
//		D3D12_BOX box;
//		box.left = from_pos.x;
//		box.top = from_pos.y;
//		box.front = from_pos.z;
//
//
//		box.right = from_pos.x + size.x;
//		box.bottom = from_pos.y + size.y;
//		box.back = from_pos.z + size.z;
//		list->CopyTextureRegion(&Dst, to_pos.x, to_pos.y, to_pos.z, &Src, &box);
//		if constexpr (BuildOptions::Debug)	TEST(Device::get(), Device::get().get_native_device()->GetDeviceRemovedReason());
//		base.create_transition_point(false);
//	}
//
//
//
//	GraphicsContext& CommandList::get_graphics()
//	{
//		return *graphics.get();// reinterpret_cast<GraphicsContext&>(*this);
//	}
//
//	ComputeContext& CommandList::get_compute()
//	{
//		return *compute.get();//return reinterpret_cast<ComputeContext&>(*this);
//	}
//
//	CopyContext& CommandList::get_copy()
//	{
//		return *copy.get();//return reinterpret_cast<ComputeContext&>(*this);
//	}
//
//
//	void ComputeContext::on_set_signature(const RootSignature::ptr& s)
//	{
//		list->SetComputeRootSignature(s->get_native().Get());
//	}
//
//	void ComputeContext::dispach(int x, int y, int z)
//	{
//		PROFILE_GPU(L"Dispatch");
//		base.create_transition_point();
//		base.setup_debug(this);
//		commit_tables();
//
//		list->Dispatch(x, y, z);
//		base.create_transition_point(false);
//		get_base().print_debug();
//
//	}
//
//
//	void ComputeContext::dispach(ivec2 a, ivec2 b)
//	{
//		dispach(ivec3(a, 1), ivec3(b, 1));
//	}
//
//	void ComputeContext::dispach(ivec3 a, ivec3 b)
//	{
//
//		ivec3 res;
//		res.x = Math::DivideByMultiple(a.x, b.x);
//		res.y = Math::DivideByMultiple(a.y, b.y);
//		res.z = Math::DivideByMultiple(a.z, b.z);
//		dispach(res.x, res.y, res.z);
//	}
//
//
//	void  ComputeContext::set(UINT i, const HandleTableLight& table)
//	{
//		assert(false);//list->SetComputeRootDescriptorTable(i, table.get_gpu());
//	}
//
//
//	void CommandList::set_pipeline_internal(PipelineStateBase* pipeline)
//	{
//		if (current_pipeline != pipeline)
//		{
//			if (pipeline)
//			{
//				auto pso = pipeline->get_native();
//				if (pso)	compiler.SetPipelineState(pso.Get());
//				else
//				{
//					auto state = pipeline->get_native_state();
//					compiler.SetPipelineState1(state.Get());
//				}
//
//				track_object(*pipeline);
//
//				if (!first_pipeline) first_pipeline = pipeline;
//			}
//			current_pipeline = pipeline;
//		}
//	}
//	void Eventer::on_start(Timer* timer)
//	{
//		names.push_back(timer->get_block().get_name());
//		start_event(names.back().c_str());
//
//		for (auto& c : timer->get_block().get_childs())
//		{
//			GPUBlock* b = dynamic_cast<GPUBlock*>(c.get());
//			if (b)
//			{
//				//	b->gpu_counter.enabled = false;
//			}
//		}
//		//		static_cast<GPUBlock*>(c.get())
//
//		GPUBlock* b = dynamic_cast<GPUBlock*>(&timer->get_block());
//		if (b)
//		{
//			b->gpu_counter.timer.start(this);
//		}
//
//
//		//	static_cast<GPUBlock&>(timer->get_block()).gpu_counter.timer.start(this);
//		//	timer->block.begin_timings(executed ? nullptr : this);
//		current = &timer->get_block();
//	}
//	void  Eventer::on_end(Timer* timer)
//	{
//		//timer->get_block().end_timings(executed ? nullptr : this);
//	//	static_cast<GPUBlock&>(timer->get_block()).gpu_counter.timer.end(this);
//
//		GPUBlock* b = dynamic_cast<GPUBlock*>(&timer->get_block());
//		if (b)
//		{
//			b->gpu_counter.timer.end(this);
//		}
//
//		current = timer->get_block().get_parent().get();
//		end_event();
//	}
//	thread_local Eventer* Eventer::thread_current = nullptr;
//	void Eventer::reset()
//	{
//		thread_current = nullptr;
//		timer.reset();
//	}
//
//	void Eventer::begin(std::string name, Timer* t)
//	{
//		this->name = name;
//		thread_current = this;
//
//		current = t ? &t->get_block() : &Profiler::get();
//		TimedRoot::parent = t ? t->get_root() : &Profiler::get();
//		if (type != CommandListType::COPY && name.size())
//		{
//			timer.reset(new Timer(start(convert(name).c_str())));
//		}
//
//		else
//			current = nullptr;
//
//
//		names.clear();
//	}
//
//	Timer  Eventer::start(std::wstring_view name)
//	{
//		if (Profiler::get().enabled)
//			return Timer(&current->get_child<GPUBlock>(name), this);
//		else return Timer(nullptr, nullptr);
//		//return std::move(Timer(nullptr,nullptr));
//
//	}
//
//
//	void Eventer::start_event(std::wstring str)
//	{
//		compiler.func([str](ID3D12GraphicsCommandList4* list)
//			{
//				PIXBeginEvent(list, 0, str.c_str());
//			});
//		//
//	}
//
//	void Eventer::end_event()
//	{
//		compiler.func([](ID3D12GraphicsCommandList4* list)
//			{
//				PIXEndEvent(list);
//			});
//		//::PIXEndEvent(m_commandList.Get());
//	}
//
//	void Eventer::set_marker(const wchar_t* label)
//	{
//
//		//::PIXSetMarker(m_commandList.Get(), 0, label);
//	}
//
//
//
//	void GraphicsContext::execute_indirect(IndirectCommand& command_types, UINT max_commands, Resource* command_buffer, UINT64 command_offset, Resource* counter_buffer, UINT64 counter_offset)
//	{
//		PROFILE_GPU(L"execute_indirect");
//		base.create_transition_point();
//
//		base.setup_debug(this);
//
//		if (command_buffer) get_base().transition(command_buffer, ResourceState::INDIRECT_ARGUMENT);
//		if (counter_buffer) get_base().transition(counter_buffer, ResourceState::INDIRECT_ARGUMENT);
//
//		get_base().transition(static_cast<HAL::Resource*>(index.Resource), ResourceState::INDEX_BUFFER);
//
//
//		D3D12_INDEX_BUFFER_VIEW native;
//		native.SizeInBytes = index.Resource?index.SizeInBytes:0;
//		native.Format = ::to_native(index.Format);
//		native.BufferLocation = index.Resource?to_native(static_cast<HAL::Resource*>(index.Resource)->get_resource_address().offset(index.OffsetInBytes)):0;
//		list->IASetIndexBuffer(&native);
//
//		commit_tables();
//
//		list->ExecuteIndirect(
//			command_types.command_signature.Get(),
//			max_commands,
//			command_buffer ? command_buffer->get_dx() : nullptr,
//			command_offset,
//			counter_buffer ? counter_buffer->get_dx() : nullptr,
//			counter_offset);
//		base.create_transition_point(false);
//		get_base().print_debug();
//	}
//	void ComputeContext::execute_indirect(IndirectCommand& command_types, UINT max_commands, Resource* command_buffer, UINT64 command_offset, Resource* counter_buffer, UINT64 counter_offset)
//	{
//		PROFILE_GPU(L"execute_indirect");
//		base.create_transition_point();
//		base.setup_debug(this);
//
//		assert(command_buffer);
//
//		if (command_buffer) get_base().transition(command_buffer, ResourceState::INDIRECT_ARGUMENT);
//		if (counter_buffer) get_base().transition(counter_buffer, ResourceState::INDIRECT_ARGUMENT);
//
//		commit_tables();
//
//		list->ExecuteIndirect(
//			command_types.command_signature.Get(),
//			max_commands,
//			command_buffer ? command_buffer->get_dx() : nullptr,
//			command_offset,
//			counter_buffer ? counter_buffer->get_dx() : nullptr,
//			counter_offset);
//		base.create_transition_point(false);
//		get_base().print_debug();
//	}
//
//	void ComputeContext::build_ras(const HAL::RaytracingBuildDescStructure& build_desc, const HAL::RaytracingBuildDescBottomInputs& bottom)
//	{
//		base.create_transition_point();
//
//		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC desc = {};
//
//		desc.DestAccelerationStructureData = to_native(build_desc.DestAccelerationStructureData);
//		desc.SourceAccelerationStructureData = to_native(build_desc.SourceAccelerationStructureData);
//		desc.ScratchAccelerationStructureData = to_native(build_desc.ScratchAccelerationStructureData);
//
//		desc.Inputs = Device::get().to_native(bottom);
//
//		for (auto g : bottom.geometry)
//		{
//			base.transition(g.IndexBuffer.resource, ResourceState::NON_PIXEL_SHADER_RESOURCE);
//			base.transition(g.VertexBuffer.resource, ResourceState::NON_PIXEL_SHADER_RESOURCE);
//			base.transition(g.Transform3x4.resource, ResourceState::NON_PIXEL_SHADER_RESOURCE);
//
//		}
//
//		base.transition(build_desc.DestAccelerationStructureData.resource, ResourceState::RAYTRACING_STRUCTURE);
//		base.transition(build_desc.SourceAccelerationStructureData.resource, ResourceState::RAYTRACING_STRUCTURE);
//		base.transition(build_desc.ScratchAccelerationStructureData.resource, ResourceState::UNORDERED_ACCESS);
//		commit_tables();
//		list->BuildRaytracingAccelerationStructure(&desc, 0, nullptr);
//
//		base.create_transition_point(false);
//	}
//
//
//	void ComputeContext::build_ras(const HAL::RaytracingBuildDescStructure& build_desc, const HAL::RaytracingBuildDescTopInputs& top)
//	{
//		base.create_transition_point();
//
//		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC desc = {};
//
//		desc.DestAccelerationStructureData = to_native(build_desc.DestAccelerationStructureData);
//		desc.SourceAccelerationStructureData = to_native(build_desc.SourceAccelerationStructureData);
//		desc.ScratchAccelerationStructureData = to_native(build_desc.ScratchAccelerationStructureData);
//
//		desc.Inputs = HAL::Device::get().to_native(top);
//		base.transition(build_desc.DestAccelerationStructureData.resource, ResourceState::RAYTRACING_STRUCTURE);
//		base.transition(build_desc.SourceAccelerationStructureData.resource, ResourceState::RAYTRACING_STRUCTURE);
//		base.transition(build_desc.ScratchAccelerationStructureData.resource, ResourceState::UNORDERED_ACCESS);
//
//
//		base.transition(top.instances.resource, ResourceState::NON_PIXEL_SHADER_RESOURCE);
//
//
//		commit_tables();
//		list->BuildRaytracingAccelerationStructure(&desc, 0, nullptr);
//
//		base.create_transition_point(false);
//	}
//	void ComputeContext::set_const_buffer(UINT i, const D3D12_GPU_VIRTUAL_ADDRESS& table)
//	{
//		list->SetComputeRootConstantBufferView(i, table);
//	}
//	void  Transitions::begin()
//	{
//		transition_count = 0;
//		//	create_zero_transition();
//		create_transition_point(false);
//	}
//
//	void Transitions::on_execute()
//	{
//
//		transition_points.clear();
//		used_resources.clear();
//
//
//		transition_list = nullptr;
//
//
//	}
//
//
//
//	void SignatureDataSetter::set_pipeline(std::shared_ptr<PipelineStateBase> pipeline)
//	{
//		/*
//				{
//					for (auto& s : used_slots.slots_usage)
//					{
//						auto slot_id = get_slot_id(s);
//						auto& slot = slots[slot_id];
//
//						auto& used_tables = *slot.tables;
//
//						for (auto& id : used_tables)
//						{
//							auto& table = tables[id].table;
//
//							for (UINT i = 0; i < (UINT)table.get_count(); ++i)
//							{
//								const auto& h = table[i];
//
//								base.stop_using(h.get_resource_info());
//
//							}
//						}
//					}
//				}
//				*/
//		if (pipeline->root_signature)
//			set_signature(pipeline->root_signature);
//
//		base.set_pipeline_internal(pipeline.get());
//
//
//		//	used_slots = pipeline->slots;
//	}
//




	ComPtr<ID3D12GraphicsCommandList4> TransitionCommandList::get_native()
	{
		return m_commandList;
	}


		}


		void TransitionCommandList::create_transition_list(const HAL::Barriers& transitions, std::vector<HAL::Resource*>& discards)
		{
			m_commandAllocator->Reset();
			m_commandList->Reset(m_commandAllocator.Get(), nullptr);

			auto& native_transitions = transitions.get_native();
			m_commandList->ResourceBarrier(static_cast<UINT>(native_transitions.size()), native_transitions.data());

			for (auto e : discards)
			{
				//	compiler.DiscardResource(e->get_native().Get(), nullptr);
			}
			m_commandList->Close();
		}



		TransitionCommandList::TransitionCommandList(CommandListType type) :type(type)
		{
			D3D12_COMMAND_LIST_TYPE t = ::to_native(type);

			TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->CreateCommandAllocator(t, IID_PPV_ARGS(&m_commandAllocator)));
			TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->CreateCommandList(0, t, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
			m_commandList->Close();
			m_commandList->SetName(L"TransitionCommandList");
		}

}