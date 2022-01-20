#include "pch_dx.h"

import Buffer;
using namespace HAL;

namespace DX12
{

    GPUBuffer::GPUBuffer(UINT64 _size, D3D12_RESOURCE_FLAGS flags /*= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE*/ , ResourceState state, HeapType heap_type) : size(Math::AlignUp(_size,D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)),
        count(Math::AlignUp(_size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)),
        stride(1), Resource(CD3DX12_RESOURCE_DESC::Buffer(Math::AlignUp(_size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), flags), heap_type, state)
    {
    }

	 D3D12_CONSTANT_BUFFER_VIEW_DESC GPUBuffer::get_const_view()
	{
		return{ get_gpu_address(), static_cast<UINT>(size) };
	}

	 GPUBuffer::GPUBuffer()
	 {
	 }

    GPUBuffer::GPUBuffer(UINT64 count, UINT stride) : count(count),
	                                                   stride(stride), Resource(CD3DX12_RESOURCE_DESC::Buffer(std::max(static_cast<UINT64>(4),count * stride)), HeapType::DEFAULT, ResourceState::COMMON)
    {
        size = count * stride;
    }

    void GPUBuffer::set_data(unsigned int offset, const std::string& v)
    {
        auto list = Device::get().get_upload_list();
        set_data(list, offset, v);
        list->end();
        list->execute_and_wait();
    }

    GPUBuffer::~GPUBuffer()
    {
	    size = 0;
    }

    void GPUBuffer::place_raw_uav(Handle h)
    {
	    D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
	    desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	    desc.Format = DXGI_FORMAT_R32_TYPELESS;
	    desc.Buffer.NumElements = static_cast<UINT>(get_desc().Width/4);
	    desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

				
	    Device::get().create_uav(h, this, desc);
    }

    void GPUBuffer::set_data(DX12::CommandList::ptr& list, unsigned int offset, const std::string& v)
    {
        list->get_copy().update_buffer(this, offset, reinterpret_cast<const char*>(v.data()), static_cast<UINT>(v.size() * sizeof(char)));
    }

	 void GPUBuffer::place_srv_buffer( Handle  handle)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC  srv = {};
		srv.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srv.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv.Buffer.FirstElement = 0;
		srv.Buffer.Flags = D3D12_BUFFER_SRV_FLAGS::D3D12_BUFFER_SRV_FLAG_NONE;
		srv.Buffer.StructureByteStride = 0;
		srv.Buffer.NumElements = static_cast<UINT>(count / sizeof(vec4));
    
        Device::get().create_srv(handle, this, srv);
     }

	 void GPUBuffer::place_structured_uav( Handle  h, GPUBuffer::ptr counter_resource, unsigned int offset)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
		desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.Buffer.NumElements = static_cast<UINT>(count);
		desc.Buffer.StructureByteStride = stride;
		desc.Buffer.CounterOffsetInBytes = offset;
		desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

        Device::get().create_uav(h, this, desc, counter_resource.get());
	}

	 void GPUBuffer::place_structured_srv( Handle handle, UINT stride , unsigned int offset, unsigned int count)
	 {
		 D3D12_SHADER_RESOURCE_VIEW_DESC  srv = {};
		 srv.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		 srv.Format = DXGI_FORMAT_UNKNOWN;
		 srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		 srv.Buffer.FirstElement = offset;
		 srv.Buffer.Flags = D3D12_BUFFER_SRV_FLAGS::D3D12_BUFFER_SRV_FLAG_NONE;
		 srv.Buffer.StructureByteStride = stride;
		 srv.Buffer.NumElements = count;

         Device::get().create_srv(handle, this, srv);
	 }

	const HandleTable& GPUBuffer::get_srv()
    {
        return srv;
    }

    const HandleTable& GPUBuffer::get_uav()
    {
        return uavs;
    }
    const HandleTable& GPUBuffer::get_static_uav()
    {
        return static_uav;
    }
	 UINT64 GPUBuffer::get_count()
	{
		return count;
	}
    ByteBuffer::ByteBuffer(UINT count) : GPUBuffer(count, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
    {
        init_views();
    }

    void ByteBuffer::init_views()
    {
        srv = StaticDescriptors::get().place(1);
        place_srv(srv[0]);
        uavs = StaticDescriptors::get().place(1);
        place_uav(uavs[0]);
        static_uav = StaticDescriptors::get().place(1);
        place_uav(static_uav[0]);
    }


    void ByteBuffer::place_uav( Handle h)
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
        desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        desc.Format = DXGI_FORMAT_R32_TYPELESS;
        desc.Buffer.NumElements = static_cast<UINT>(count / 4);
        desc.Buffer.StructureByteStride = 0;
        desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

        Device::get().create_uav(h, this, desc);
    }

    void ByteBuffer::place_srv( Handle h)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
        desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        desc.Format = DXGI_FORMAT_R32_TYPELESS;
        desc.Buffer.NumElements = static_cast<UINT>(count / 4);
        desc.Buffer.StructureByteStride = 0;
        desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

     
        Device::get().create_srv(h, this, desc);
    }
    IndexBuffer::IndexBuffer(UINT64 size) : GPUBuffer(size)
    {
    }

    IndexBuffer::IndexBuffer()
    {
    }

    IndexBufferView GPUBuffer::get_index_buffer_view(bool is_32, unsigned int offset /*= 0*/, UINT size /*= 0*/)
    {
        IndexBufferView view;
        view.view.BufferLocation = get_gpu_address() + offset;
        view.view.Format = is_32 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
        view.view.SizeInBytes = static_cast<UINT>(size ? size : this->size);
        view.resource = this;
        return view;
    }


	QueryHeap::QueryHeap(UINT max_count, D3D12_QUERY_HEAP_TYPE type)
	{
		D3D12_QUERY_HEAP_DESC QueryHeapDesc;
		QueryHeapDesc.Count = max_count;
		QueryHeapDesc.NodeMask = 1;
		QueryHeapDesc.Type = type;
		Device::get().get_native_device()->CreateQueryHeap(&QueryHeapDesc, IID_PPV_ARGS(&heap));
		heap->SetName(L"QueryHeap");
	}

	 ComPtr<ID3D12QueryHeap> DX12::QueryHeap::get_native()
	{
		return heap;
	}
}
