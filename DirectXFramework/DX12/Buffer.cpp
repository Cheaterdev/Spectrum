#include "pch.h"
#include "Buffer.h"

namespace DX12
{

    GPUBuffer::GPUBuffer(UINT64 size, D3D12_RESOURCE_FLAGS flags /*= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE*/ , ResourceState state) : size(size),
        count(size),
        stride(1), Resource(CD3DX12_RESOURCE_DESC::Buffer(size, flags), HeapType::DEFAULT, state)
    {
    }

	 D3D12_CONSTANT_BUFFER_VIEW_DESC GPUBuffer::get_const_view()
	{
		return{ get_gpu_address(), static_cast<UINT>(size) };
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

    void GPUBuffer::set_data(DX12::CommandList::ptr& list, unsigned int offset, const std::string& v)
    {
        list->transition(this,  ResourceState::COPY_DEST);
        list->get_copy().update_buffer(this, offset, reinterpret_cast<const char*>(v.data()), static_cast<UINT>(v.size() * sizeof(char)));
        list->transition(this, ResourceState::COMMON);
    }

	 void GPUBuffer::place_srv_buffer(const Handle & handle)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC  srv = {};
		srv.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srv.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv.Buffer.FirstElement = 0;
		srv.Buffer.Flags = D3D12_BUFFER_SRV_FLAGS::D3D12_BUFFER_SRV_FLAG_NONE;
		srv.Buffer.StructureByteStride = 0;
		srv.Buffer.NumElements = static_cast<UINT>(count / sizeof(vec4));
		Device::get().get_native_device()->CreateShaderResourceView(get_native().Get(), &srv, handle.cpu);
	}

	 void GPUBuffer::place_structured_uav(const Handle & h, GPUBuffer::ptr counter_resource, unsigned int offset)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
		desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.Buffer.NumElements = static_cast<UINT>(count);
		desc.Buffer.StructureByteStride = stride;
		desc.Buffer.CounterOffsetInBytes = offset;
		desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		Device::get().get_native_device()->CreateUnorderedAccessView(get_native().Get(), counter_resource ? counter_resource->get_native().Get() : nullptr, &desc, h.cpu);
	}

	 void GPUBuffer::place_structured_srv(const Handle& handle, UINT stride , unsigned int offset, unsigned int count)
	 {
		 D3D12_SHADER_RESOURCE_VIEW_DESC  srv = {};
		 srv.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		 srv.Format = DXGI_FORMAT_UNKNOWN;
		 srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		 srv.Buffer.FirstElement = offset;
		 srv.Buffer.Flags = D3D12_BUFFER_SRV_FLAGS::D3D12_BUFFER_SRV_FLAG_NONE;
		 srv.Buffer.StructureByteStride = stride;
		 srv.Buffer.NumElements = count;
		 Device::get().get_native_device()->CreateShaderResourceView(get_native().Get(), &srv, handle.cpu);
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
        srv = DescriptorHeapManager::get().get_csu_static()->create_table(1);
        place_srv(srv[0]);
        uavs = DescriptorHeapManager::get().get_csu_static()->create_table(1);
        place_uav(uavs[0]);
        static_uav = DescriptorHeapManager::get().get_csu_static()->create_table(1);
        place_uav(static_uav[0]);
    }


    void ByteBuffer::place_uav(const Handle& h)
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
        desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        desc.Format = DXGI_FORMAT_R32_TYPELESS;
        desc.Buffer.NumElements = static_cast<UINT>(count / 4);
        desc.Buffer.StructureByteStride = 0;
        desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
        Device::get().get_native_device()->CreateUnorderedAccessView(get_native().Get(), nullptr, &desc, h.cpu);
    }

    void ByteBuffer::place_srv(const Handle& h)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
        desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        desc.Format = DXGI_FORMAT_R32_TYPELESS;
        desc.Buffer.NumElements = static_cast<UINT>(count / 4);
        desc.Buffer.StructureByteStride = 0;
        desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
        Device::get().get_native_device()->CreateShaderResourceView(get_native().Get(), &desc, h.cpu);
    }
    IndexBuffer::IndexBuffer(UINT64 size) : GPUBuffer(size)
    {
    }

    IndexBuffer::IndexBuffer()
    {
    }

    D3D12_INDEX_BUFFER_VIEW IndexBuffer::get_index_buffer_view(bool is_32, unsigned int offset /*= 0*/, UINT size /*= 0*/)
    {
        D3D12_INDEX_BUFFER_VIEW view;
        view.BufferLocation = get_gpu_address() + offset;
        view.Format = is_32 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
        view.SizeInBytes = static_cast<UINT>(size ? size : this->size);
        return view;
    }



	 void DX12::UploadBuffer::unmap()
	{
		m_Resource->Unmap(0, nullptr);
	}

	 DX12::UploadBuffer::UploadBuffer(UINT64 count) : Resource(CD3DX12_RESOURCE_DESC::Buffer(count, D3D12_RESOURCE_FLAG_NONE), HeapType::UPLOAD, ResourceState::GEN_READ)
	{
		data = map(0, count);
	}

	 DX12::UploadBuffer::~UploadBuffer()
	{
		unmap();
	}

	 char * DX12::UploadBuffer::get_data()
	{
		return data;
	}

	 DX12::CPUBuffer::CPUBuffer(UINT64 count, int stride) : stride(stride), Resource(CD3DX12_RESOURCE_DESC::Buffer(count * stride), HeapType::READBACK, ResourceState::COPY_DEST)
	{
	}

	 void DX12::CPUBuffer::unmap()
	{
		 mapped = false;
		m_Resource->Unmap(0, nullptr);
	}

	 DX12::QueryHeap::QueryHeap(UINT max_count, D3D12_QUERY_HEAP_TYPE type)
	{
		D3D12_QUERY_HEAP_DESC QueryHeapDesc;
		QueryHeapDesc.Count = max_count;
		QueryHeapDesc.NodeMask = 1;
		QueryHeapDesc.Type = type;
		Device::get().get_native_device()->CreateQueryHeap(&QueryHeapDesc, IID_PPV_ARGS(&heap));
		heap->SetName(L"QueryHeap");
	}

	 CComPtr<ID3D12QueryHeap> DX12::QueryHeap::get_native()
	{
		return heap;
	}
}
