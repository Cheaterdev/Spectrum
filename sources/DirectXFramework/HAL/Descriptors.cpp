#include "pch_dx.h"
//
import Device;
import Resource;
import Descriptors;
import d3d12_types;

namespace DX12 {

	DescriptorHeapType get_heap_type(HandleType type)
	{
		switch (type)
		{
		case HandleType::CBV:
		case HandleType::SRV:
		case HandleType::UAV:
			return DescriptorHeapType::CBV_SRV_UAV;

		case HandleType::RTV:
			return DescriptorHeapType::RTV;

		case HandleType::DSV:
			return DescriptorHeapType::DSV;

		case HandleType::SMP:
			return DescriptorHeapType::SAMPLER;
		}

		return DescriptorHeapType();
	}

	DescriptorHeapManager::DescriptorHeapManager()
	{
		heap_cb_sr_ua_static.reset(new DescriptorHeap(65536, DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapFlags::NONE));
		heap_rt.reset(new DescriptorHeap(65536, DescriptorHeapType::RTV, DescriptorHeapFlags::NONE));
		heap_ds.reset(new DescriptorHeap(65536, DescriptorHeapType::DSV, DescriptorHeapFlags::NONE));


		gpu_heaps[DescriptorHeapType::CBV_SRV_UAV].reset(new DescriptorHeapPaged(65536*8, DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapFlags::SHADER_VISIBLE));
		gpu_heaps[DescriptorHeapType::SAMPLER].reset(new DescriptorHeapPaged(2048, DescriptorHeapType::SAMPLER, DescriptorHeapFlags::SHADER_VISIBLE));
	
		cpu_heaps[DescriptorHeapType::CBV_SRV_UAV].reset(new DescriptorHeapPaged(65536, DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapFlags::NONE));
		cpu_heaps[DescriptorHeapType::RTV].reset(new DescriptorHeapPaged(65536, DescriptorHeapType::RTV, DescriptorHeapFlags::NONE));
		cpu_heaps[DescriptorHeapType::DSV].reset(new DescriptorHeapPaged(65536, DescriptorHeapType::DSV, DescriptorHeapFlags::NONE));
		cpu_heaps[DescriptorHeapType::SAMPLER].reset(new DescriptorHeapPaged(65536, DescriptorHeapType::SAMPLER, DescriptorHeapFlags::NONE));
	}


	DescriptorHeap::DescriptorHeap(UINT num, DescriptorHeapType type, DescriptorHeapFlags flags):flags(flags)
	{
		max_count = num;
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = num;
		rtvHeapDesc.Type = static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(type);
		rtvHeapDesc.Flags = static_cast<D3D12_DESCRIPTOR_HEAP_FLAGS>(flags);
		rtvHeapDesc.NodeMask = 1;
		TEST(Device::get().get_native_device()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
		descriptor_size = Device::get().get_native_device()->GetDescriptorHandleIncrementSize(static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(type));
		assert(m_rtvHeap);

		resources.resize(max_count);
	}


	CD3DX12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::get_handle(UINT i)
	{
		return  CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), i, descriptor_size);
	}

	CD3DX12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::get_gpu_handle(UINT i)
	{
		if (flags == DescriptorHeapFlags::SHADER_VISIBLE)
			return CD3DX12_GPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetGPUDescriptorHandleForHeapStart(), i, descriptor_size);
		
		
			return CD3DX12_GPU_DESCRIPTOR_HANDLE();
	}


	void Handle::place(const Handle& r) const
	{
		assert(offset != UINT_MAX);
		D3D12_DESCRIPTOR_HEAP_TYPE type = (D3D12_DESCRIPTOR_HEAP_TYPE)get_heap_type(r.resource_info->type);

		if (cpu != r.cpu)
			Device::get().get_native_device()->CopyDescriptorsSimple(1, cpu, r.cpu, type);

		*resource_info = *r.resource_info;
	}

/*
	void Handle::clear(CommandList& list, float4 color) const
	{

		list.clear_rtv(*this, color);
	}

	*/
	Handle DescriptorPage::place()
	{
		Handle h =  heap->handle(heap_offset + (offset++));
		h.resource_info->resource_ptr = nullptr;
		return h;
	}



	HandleTableLight DescriptorPage::place(UINT count)
	{
		assert(offset+count<=this->count);
		auto table = heap->get_light_table_view(heap_offset + offset, count);

		for (UINT i = 0; i < (UINT)table.get_count(); i++)
		{
			table[i].resource_info->resource_ptr = nullptr;
		}
		offset += count;
		return table;
	}



	void DescriptorPage::free()
	{
		offset = 0;
		heap->free_page(this);
	}
	
	DescriptorPage* DescriptorHeapPaged::create_page(UINT count)
	{

		UINT pages_count = (count + 32 - 1) / 32;

		std::lock_guard<std::mutex> g(m);
		auto handle = allocator.Allocate(pages_count, 1);

		return new DescriptorPage(std::dynamic_pointer_cast<DescriptorHeapPaged>(shared_from_this()), handle, (UINT)(handle.get_offset() * 32), pages_count * 32);
	}

	void DescriptorHeapPaged::free_page(DescriptorPage* page)
	{

		std::lock_guard<std::mutex> g(m);
		allocator.Free(page->alloc_handle);
		delete page;
	}

	void for_each_rtv(const ResourceInfo* info, std::function<void(Resource*, UINT)> f)
	{
		assert(info->type == HandleType::RTV);

		auto& desc = info->resource_ptr->get_desc();
		if (info->rtv.ViewDimension == D3D12_RTV_DIMENSION_TEXTURE2D)
		{

			if (desc.MipLevels == 1 && desc.Depth() == 1)
			{
				f(info->resource_ptr,  D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
			}
			else
			{
				auto& rtv = info->rtv.Texture2D;
				UINT res = desc.CalcSubresource(rtv.MipSlice, 0, rtv.PlaneSlice);
				f(info->resource_ptr,  res);
			}
		}
		else 	if (info->rtv.ViewDimension == D3D12_RTV_DIMENSION_TEXTURE2DARRAY)
		{

			auto& rtv = info->rtv.Texture2DArray;

			for (UINT array = rtv.FirstArraySlice; array < rtv.FirstArraySlice + rtv.ArraySize; array++)
			{
				UINT res = desc.CalcSubresource(rtv.MipSlice, array, rtv.PlaneSlice);
				f(info->resource_ptr, res);
			}
		}

	}

	void for_each_uav(const ResourceInfo* info, std::function<void(Resource*, UINT)> f)
	{
		assert(info->type == HandleType::UAV);


		auto& desc = info->resource_ptr->get_desc();

		if (info->uav.desc.ViewDimension == D3D12_UAV_DIMENSION_BUFFER)
		{
			f(info->resource_ptr, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
			if (info->uav.counter) f(info->uav.counter,  D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

		}
		else if (info->uav.desc.ViewDimension == D3D12_UAV_DIMENSION_TEXTURE2D)
		{
			if (desc.MipLevels == 1 && desc.Depth() == 1)
			{
				f(info->resource_ptr, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
			}
			else
			{
				auto& uav = info->uav.desc.Texture2D;
				UINT res = desc.CalcSubresource(uav.MipSlice, 0, uav.PlaneSlice);
				f(info->resource_ptr,  res);
			}
		}
		else if (info->uav.desc.ViewDimension == D3D12_UAV_DIMENSION_TEXTURE3D)
		{
			auto& uav = info->uav.desc.Texture3D;
			if (uav.FirstWSlice == 0 && uav.WSize == desc.Depth() && desc.MipLevels == 1)
			{
				f(info->resource_ptr, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
			}
			else
			{
				UINT res = desc.CalcSubresource(uav.MipSlice, 0, 0);
				f(info->resource_ptr,  res);
			}
		}

		else if (info->uav.desc.ViewDimension == D3D12_UAV_DIMENSION_TEXTURE2DARRAY)
		{
			auto& uav = info->uav.desc.Texture2DArray;

			if (desc.MipLevels == 1 && desc.Depth() == 1 && desc.ArraySize() == 1)
			{
				f(info->resource_ptr,  D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
			}
			else
			{

				for (UINT array = uav.FirstArraySlice; array < uav.FirstArraySlice + uav.ArraySize; array++)
				{
					UINT res = desc.CalcSubresource(uav.MipSlice, array, uav.PlaneSlice);
					f(info->resource_ptr, res);
				}

			}
		}
		else assert(false);

	}


	void for_each_dsv(const ResourceInfo* info, std::function<void(Resource*, UINT)> f)
	{
		assert(info->type == HandleType::DSV);

		auto& desc = info->resource_ptr->get_desc();

		if (info->dsv.ViewDimension == D3D12_DSV_DIMENSION_TEXTURE2D)
		{
			auto& dsv = info->dsv.Texture2D;

			UINT res = desc.CalcSubresource(dsv.MipSlice, 0, 0);
			f(info->resource_ptr, res);

		}
		else if (info->dsv.ViewDimension == D3D12_DSV_DIMENSION_TEXTURE2DARRAY)
		{
			auto& dsv = info->dsv.Texture2DArray;

			for (UINT array = dsv.FirstArraySlice; array < dsv.FirstArraySlice + dsv.ArraySize; array++)
			{
				UINT res = desc.CalcSubresource(dsv.MipSlice, array, 0);
				f(info->resource_ptr, res);
			}
		}
		else
			assert(0);
	}


	void for_each_srv(const ResourceInfo* info, std::function<void(Resource*, UINT)> f)
	{
		assert(info->type == HandleType::SRV);

		auto& desc = info->resource_ptr->get_desc();

		if (info->srv.ViewDimension == D3D12_UAV_DIMENSION_BUFFER)
		{
			f(info->resource_ptr, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
		}
		else if (info->srv.ViewDimension == D3D12_SRV_DIMENSION_TEXTURE2D)
		{
			auto& srv = info->srv.Texture2D;

			if (srv.MipLevels == desc.MipLevels && srv.MostDetailedMip == 0 && desc.Depth() == 1)
			{
				f(info->resource_ptr, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
			}
			else
			{
				for (auto mip = srv.MostDetailedMip; mip < srv.MostDetailedMip + srv.MipLevels; mip++)
				{
					UINT res = desc.CalcSubresource(mip, 0, srv.PlaneSlice);
					f(info->resource_ptr, res);
				}
			}

		}
		else if (info->srv.ViewDimension == D3D12_SRV_DIMENSION_TEXTURE2DARRAY)
		{
			auto& srv = info->srv.Texture2DArray;

			if (srv.MipLevels == desc.MipLevels && srv.MostDetailedMip == 0 && srv.FirstArraySlice == 0 && srv.ArraySize == desc.ArraySize() && desc.Depth() == 1)
			{
				f(info->resource_ptr, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
			}
			else
			{
				for (auto mip = srv.MostDetailedMip; mip < srv.MostDetailedMip + srv.MipLevels; mip++)
					for (auto array = srv.FirstArraySlice; array < srv.FirstArraySlice + srv.ArraySize; array++)
					{
						UINT res = desc.CalcSubresource(mip, array, srv.PlaneSlice);
						f(info->resource_ptr, res);
					}
			}
		}
		else if (info->srv.ViewDimension == D3D12_SRV_DIMENSION_TEXTURE3D)
		{
			auto& srv = info->srv.Texture3D;

			if (srv.MipLevels == desc.MipLevels && srv.MostDetailedMip == 0)
			{
				f(info->resource_ptr, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
			}
			else
			{
				for (auto mip = srv.MostDetailedMip; mip < srv.MostDetailedMip + srv.MipLevels; mip++)
				{
					UINT res = desc.CalcSubresource(mip, 0, 0);
					f(info->resource_ptr, res);
				}
			}
		}
		else if (info->srv.ViewDimension == D3D12_SRV_DIMENSION_TEXTURECUBE)
		{
			auto& srv = info->srv.TextureCube;

			if (srv.MipLevels == desc.MipLevels && srv.MostDetailedMip == 0)
			{
				f(info->resource_ptr, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
			}
			else
			{
				for (auto mip = srv.MostDetailedMip; mip < srv.MostDetailedMip + srv.MipLevels; mip++)
					for (auto array = 0; array < 6; array++)
					{
						UINT res = desc.CalcSubresource(mip, array, 0);
						f(info->resource_ptr, res);
					}
			}
		}
		else
			assert(false);




	}
	
	
	void ResourceInfo::for_each_subres(std::function<void(Resource*, UINT)> f) const
	{
		if (type == HandleType::RTV)
		{
			for_each_rtv(this, f);
		}
		else if (type == HandleType::DSV)
		{
			for_each_dsv(this, f);
		}
		else if (type == HandleType::SRV)
		{
			for_each_srv(this, f);
		}
		else if (type == HandleType::UAV)
		{
			for_each_uav(this, f);
		}
		else
			assert(false);
	}

	namespace HLSL
	{
		void RaytracingAccelerationStructure::create(Resource* resource)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
			desc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			desc.Format = DXGI_FORMAT_UNKNOWN;
			desc.RaytracingAccelerationStructure.Location = resource->get_gpu_address();

			Device::get().create_srv(*this, nullptr, desc);
		}

		void ByteAddressBuffer::create(Resource* resource, UINT offset , UINT size )
		{
			if (size == 0) size = static_cast<UINT>(resource->get_desc().Width / 4);

			D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
			desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			desc.Format = DXGI_FORMAT_R32_TYPELESS;
			desc.Buffer.NumElements = size;
			desc.Buffer.StructureByteStride = 0;
			desc.Buffer.FirstElement = offset;

			desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

			Device::get().create_srv(*this, resource, desc);
		}

		void RWByteAddressBuffer::create(Resource* resource, UINT offset , UINT size )
		{
			if (size == 0) size = static_cast<UINT>(resource->get_desc().Width / 4);
			D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
			desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			desc.Format = DXGI_FORMAT_R32_TYPELESS;
			desc.Buffer.NumElements = size;
			desc.Buffer.StructureByteStride = 0;
			desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
			desc.Buffer.FirstElement = offset;

			Device::get().create_uav(*this, resource, desc);
		}
	}
	
}