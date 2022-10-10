export module HAL:DescriptorHeap;

import :Device;
import :Utils;
import :Types;
import :Descriptors;
import :Resource;

import Utils;
import Debug;

export namespace HAL
{

	struct DescriptorHeapDesc
	{
		uint Count;
		DescriptorHeapType HeapType;
		DescriptorHeapFlags Flags;
	};

	class DescriptorHeap;
	class Descriptor
	{
		DescriptorHeap& heap;
		const uint offset;

		Descriptor(DescriptorHeap& heap, uint offset) : heap(heap), offset(offset)
		{
		}

		friend class DescriptorHeap;
	public:
		void operator=(const Descriptor& r);

		void place(const Views::ShaderResource& view);
		void place(const Views::UnorderedAccess& view);
		void place(const Views::RenderTarget& view);
		void place(const Views::ConstantBuffer& view);
		void place(const Views::DepthStencil& view);

		D3D12_CPU_DESCRIPTOR_HANDLE  get_cpu();
		D3D12_GPU_DESCRIPTOR_HANDLE  get_gpu();

	};

	class DescriptorHeap
	{
	public:// TODO
		D3D::DescriptorHeap m_cpu_heap;
		D3D::DescriptorHeap m_gpu_heap;

		const DescriptorHeapDesc desc;
		const Device& device;

		CD3DX12_CPU_DESCRIPTOR_HANDLE  cpu_start;
		CD3DX12_CPU_DESCRIPTOR_HANDLE  gpu_cpu_start;


		CD3DX12_GPU_DESCRIPTOR_HANDLE  gpu_start;

		uint handle_size;
		friend class Descriptor;
	public:
		DescriptorHeap(Device& device, const DescriptorHeapDesc& desc) :device(device), desc(desc)
		{
			D3D12_DESCRIPTOR_HEAP_DESC native_desc = {};
			native_desc.NumDescriptors = desc.Count;
			native_desc.Type = to_native(desc.HeapType);
			native_desc.Flags = to_native(DescriptorHeapFlags::NONE);
			native_desc.NodeMask = 1;

			TEST(device, device.native_device->CreateDescriptorHeap(&native_desc, IID_PPV_ARGS(&m_cpu_heap)));
			cpu_start = m_cpu_heap->GetCPUDescriptorHandleForHeapStart();

			if (check(desc.Flags & DescriptorHeapFlags::SHADER_VISIBLE))
			{
				native_desc.Flags = to_native(DescriptorHeapFlags::SHADER_VISIBLE);
				TEST(device, device.native_device->CreateDescriptorHeap(&native_desc, IID_PPV_ARGS(&m_gpu_heap)));
				gpu_cpu_start = m_gpu_heap->GetCPUDescriptorHandleForHeapStart();

				gpu_start = m_gpu_heap->GetGPUDescriptorHandleForHeapStart();
			}

			handle_size = device.get_descriptor_size(desc.HeapType);
		}

		Descriptor operator[](uint i)
		{
			return Descriptor{ *this, i };
		}
	};


	void Descriptor::place(const Views::ShaderResource& view)
	{
		if constexpr (BuildOptions::Debug)
		{
			Log::get() << Log::LEVEL_DEBUG << "Creating " << view << Log::endl;
		}
		D3D12_SHADER_RESOURCE_VIEW_DESC desc;

		desc.Format = to_native(view.Format);
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;// view.Shader4ComponentMapping;
		auto native_resource = view.Resource ? view.Resource->native_resource.Get() : nullptr;


		std::visit(overloaded{
			[&](const Views::ShaderResource::Buffer& Buffer) {
				desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
				desc.Buffer.FirstElement = Buffer.FirstElement;
				desc.Buffer.NumElements = Buffer.NumElements;
				desc.Buffer.StructureByteStride = Buffer.StructureByteStride;
				desc.Buffer.Flags = Buffer.Raw ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;

				assert(Buffer.StructureByteStride == 0 || view.Format == Format::UNKNOWN);
			},
			[&](const Views::ShaderResource::Texture1D& Texture1D) {
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
				desc.Texture1D.MostDetailedMip = Texture1D.MostDetailedMip;
				desc.Texture1D.MipLevels = Texture1D.MipLevels;
				desc.Texture1D.ResourceMinLODClamp = Texture1D.ResourceMinLODClamp;

				if (desc.Texture1D.MipLevels == 0) desc.Texture1D.MipLevels = view.Resource->get_desc().as_texture().MipLevels - desc.Texture1D.MostDetailedMip;
			},
			[&](const Views::ShaderResource::Texture1DArray& Texture1DArray) {
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
				desc.Texture1DArray.MostDetailedMip = Texture1DArray.MostDetailedMip;
				desc.Texture1DArray.MipLevels = Texture1DArray.MipLevels;
				desc.Texture1DArray.ResourceMinLODClamp = Texture1DArray.ResourceMinLODClamp;
				desc.Texture1DArray.FirstArraySlice = Texture1DArray.FirstArraySlice;
				desc.Texture1DArray.ArraySize = Texture1DArray.ArraySize;

				if (desc.Texture1DArray.MipLevels == 0) desc.Texture1DArray.MipLevels = view.Resource->get_desc().as_texture().MipLevels - Texture1DArray.MostDetailedMip;
			},
			[&](const Views::ShaderResource::Texture2D& Texture2D) {

					auto tdesc = view.Resource->get_desc().as_texture();
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				desc.Texture2D.MostDetailedMip = Texture2D.MostDetailedMip;
				desc.Texture2D.MipLevels = Texture2D.MipLevels;
				desc.Texture2D.ResourceMinLODClamp = Texture2D.ResourceMinLODClamp;
				desc.Texture2D.PlaneSlice = Texture2D.PlaneSlice;


				if (desc.Texture2D.MipLevels == 0) desc.Texture2D.MipLevels = view.Resource->get_desc().as_texture().MipLevels - desc.Texture2D.MostDetailedMip;

				assert(desc.Texture2D.PlaneSlice == 0);
				assert(desc.Texture2D.MipLevels > 0);
				assert(desc.Texture2D.MostDetailedMip + desc.Texture2D.MipLevels <= view.Resource->get_desc().as_texture().MipLevels);
			},
			[&](const Views::ShaderResource::Texture2DArray& Texture2DArray) {
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.MostDetailedMip = Texture2DArray.MostDetailedMip;
				desc.Texture2DArray.MipLevels = Texture2DArray.MipLevels;
				desc.Texture2DArray.ResourceMinLODClamp = Texture2DArray.ResourceMinLODClamp;
				desc.Texture2DArray.PlaneSlice = Texture2DArray.PlaneSlice;
				desc.Texture2DArray.FirstArraySlice = Texture2DArray.FirstArraySlice;
				desc.Texture2DArray.ArraySize = Texture2DArray.ArraySize;

				if (desc.Texture2DArray.MipLevels == 0) desc.Texture2DArray.MipLevels = view.Resource->get_desc().as_texture().MipLevels - desc.Texture2DArray.MostDetailedMip;


				assert(desc.Texture2DArray.PlaneSlice == 0);
				assert(desc.Texture2DArray.MipLevels > 0);
				assert(desc.Texture2DArray.MostDetailedMip + desc.Texture2DArray.MipLevels <= view.Resource->get_desc().as_texture().MipLevels);
			},
			[&](const Views::ShaderResource::Texture3D& Texture3D) {
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
				desc.Texture3D.MostDetailedMip = Texture3D.MostDetailedMip;
				desc.Texture3D.MipLevels = Texture3D.MipLevels;
				desc.Texture3D.ResourceMinLODClamp = Texture3D.ResourceMinLODClamp;

				if (desc.Texture3D.MipLevels == 0) desc.Texture3D.MipLevels = view.Resource->get_desc().as_texture().MipLevels - desc.Texture3D.MostDetailedMip;
			},
			[&](const Views::ShaderResource::Texture2DMS& Texture2DMS) {
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
			},
			[&](const Views::ShaderResource::Texture2DMSArray& Texture2DMSArray) {
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
				desc.Texture2DMSArray.FirstArraySlice = Texture2DMSArray.FirstArraySlice;
				desc.Texture2DMSArray.ArraySize = Texture2DMSArray.ArraySize;
			},
			[&](const Views::ShaderResource::Cube& Cube) {
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
				desc.TextureCube.MostDetailedMip = Cube.MostDetailedMip;
				desc.TextureCube.MipLevels = Cube.MipLevels;
				desc.TextureCube.ResourceMinLODClamp = Cube.ResourceMinLODClamp;

				if (desc.TextureCube.MipLevels == 0) desc.TextureCube.MipLevels = view.Resource->get_desc().as_texture().MipLevels - desc.TextureCube.MostDetailedMip;

				assert(desc.TextureCube.MipLevels > 0);
				assert(desc.TextureCube.MostDetailedMip + desc.TextureCube.MipLevels <= view.Resource->get_desc().as_texture().MipLevels);
			},
			[&](const Views::ShaderResource::CubeArray& CubeArray) {
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
				desc.TextureCubeArray.MostDetailedMip = CubeArray.MostDetailedMip;
				desc.TextureCubeArray.MipLevels = CubeArray.MipLevels;
				desc.TextureCubeArray.ResourceMinLODClamp = CubeArray.ResourceMinLODClamp;
				desc.TextureCubeArray.NumCubes = CubeArray.NumCubes;
				desc.TextureCubeArray.First2DArrayFace = CubeArray.First2DArrayFace;

				if (desc.TextureCubeArray.MipLevels == 0) desc.TextureCubeArray.MipLevels = view.Resource->get_desc().as_texture().MipLevels - desc.TextureCubeArray.MostDetailedMip;

			},
			[&](const Views::ShaderResource::Raytracing& Raytracing) {
				desc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
				desc.RaytracingAccelerationStructure.Location = Raytracing.Location;
				native_resource = nullptr;
			},
			[&](auto other) {
				assert(false);
			}
			}, view.View);


		auto size = heap.device.get_descriptor_size(heap.desc.HeapType);

		{
			auto h = heap.cpu_start;
			h.Offset(offset, size);
			heap.device.native_device->CreateShaderResourceView(native_resource, &desc, h);
			if constexpr (BuildOptions::Debug)	TEST(heap.device,heap.device.native_device->GetDeviceRemovedReason());
		}

		if (heap.m_gpu_heap)
		{
			auto h = heap.gpu_cpu_start;
			h.Offset(offset, size);
			heap.device.native_device->CreateShaderResourceView(native_resource, &desc, h);
			if constexpr (BuildOptions::Debug)	TEST(heap.device, heap.device.native_device->GetDeviceRemovedReason());
		}
	}

	void Descriptor::place(const Views::RenderTarget& view)
	{
		if constexpr (BuildOptions::Debug)
		{
			Log::get() << Log::LEVEL_DEBUG << "Creating " << view << Log::endl;
		}

		D3D12_RENDER_TARGET_VIEW_DESC desc;
		desc.Format = to_native(view.Format);

		std::visit(overloaded{
			[&](const Views::RenderTarget::Buffer& Buffer) {
				desc.ViewDimension = D3D12_RTV_DIMENSION_BUFFER;
				desc.Buffer.FirstElement = Buffer.FirstElement;
				desc.Buffer.NumElements = Buffer.NumElements;
			},
			[&](const Views::RenderTarget::Texture1D& Texture1D) {
				desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
				desc.Texture1D.MipSlice = Texture1D.MipSlice;
			},
			[&](const Views::RenderTarget::Texture1DArray& Texture1DArray) {
				desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
				desc.Texture1DArray.MipSlice = Texture1DArray.MipSlice;
				desc.Texture1DArray.FirstArraySlice = Texture1DArray.FirstArraySlice;
				desc.Texture1DArray.ArraySize = Texture1DArray.ArraySize;
			},
			[&](const Views::RenderTarget::Texture2D& Texture2D) {
				desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
				desc.Texture2D.MipSlice = Texture2D.MipSlice;
				desc.Texture2D.PlaneSlice = Texture2D.PlaneSlice;


				assert(desc.Texture2D.PlaneSlice == 0);
			},
			[&](const Views::RenderTarget::Texture2DArray& Texture2DArray) {
				desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.MipSlice = Texture2DArray.MipSlice;
				desc.Texture2DArray.PlaneSlice = Texture2DArray.PlaneSlice;
				desc.Texture2DArray.FirstArraySlice = Texture2DArray.FirstArraySlice;
				desc.Texture2DArray.ArraySize = Texture2DArray.ArraySize;

				assert(desc.Texture2DArray.PlaneSlice == 0);
			},
			[&](const Views::RenderTarget::Texture3D& Texture3D) {
				desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
				desc.Texture3D.MipSlice = Texture3D.MipSlice;
				desc.Texture3D.FirstWSlice = Texture3D.FirstWSlice;
				desc.Texture3D.WSize = Texture3D.WSize;
			},
			[&](const Views::RenderTarget::Texture2DMS& Texture2DMS) {
				desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
			},
			[&](const Views::RenderTarget::Texture2DMSArray& Texture2DMSArray) {
				desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
				desc.Texture2DMSArray.FirstArraySlice = Texture2DMSArray.FirstArraySlice;
				desc.Texture2DMSArray.ArraySize = Texture2DMSArray.ArraySize;
			},
			[&](auto other) {
				assert(false);
			}
			}, view.View);


		auto native_resource = view.Resource ? view.Resource->native_resource.Get() : nullptr;
		auto size = heap.device.get_descriptor_size(heap.desc.HeapType);

		{
			auto h = heap.cpu_start;
			h.Offset(offset, size);
			heap.device.native_device->CreateRenderTargetView(native_resource, &desc, h);
			if constexpr (BuildOptions::Debug)	TEST(heap.device, heap.device.native_device->GetDeviceRemovedReason());
		}

		if (heap.m_gpu_heap)
		{
			auto h = heap.gpu_cpu_start;
			h.Offset(offset, size);
			heap.device.native_device->CreateRenderTargetView(native_resource, &desc, h);
			if constexpr (BuildOptions::Debug)	TEST(heap.device, heap.device.native_device->GetDeviceRemovedReason());
		}
	}

	void Descriptor::place(const Views::DepthStencil& view)
	{
		if constexpr (BuildOptions::Debug)
		{
			Log::get() << Log::LEVEL_DEBUG << "Creating " << view << Log::endl;
		}

		D3D12_DEPTH_STENCIL_VIEW_DESC desc;
		desc.Format = to_native(view.Format);
		desc.Flags = D3D12_DSV_FLAGS::D3D12_DSV_FLAG_NONE;

		if (check(view.Flags & Views::DepthStencil::Flags::ReadOnlyDepth))
		{
			desc.Flags |= D3D12_DSV_FLAGS::D3D12_DSV_FLAG_READ_ONLY_DEPTH;
		}

		if (check(view.Flags & Views::DepthStencil::Flags::ReadOnlyStencil))
		{
			desc.Flags |= D3D12_DSV_FLAGS::D3D12_DSV_FLAG_READ_ONLY_STENCIL;
		}

		std::visit(overloaded{
			[&](const Views::DepthStencil::Texture1D& Texture1D) {
				desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
				desc.Texture1D.MipSlice = Texture1D.MipSlice;
			},
			[&](const Views::DepthStencil::Texture1DArray& Texture1DArray) {
				desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
				desc.Texture1DArray.MipSlice = Texture1DArray.MipSlice;
				desc.Texture1DArray.FirstArraySlice = Texture1DArray.FirstArraySlice;
				desc.Texture1DArray.ArraySize = Texture1DArray.ArraySize;
			},
			[&](const Views::DepthStencil::Texture2D& Texture2D) {
				desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
				desc.Texture2D.MipSlice = Texture2D.MipSlice;
			},
			[&](const Views::DepthStencil::Texture2DArray& Texture2DArray) {
				desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.MipSlice = Texture2DArray.MipSlice;
				desc.Texture2DArray.FirstArraySlice = Texture2DArray.FirstArraySlice;
				desc.Texture2DArray.ArraySize = Texture2DArray.ArraySize;
			},
			[&](const Views::DepthStencil::Texture2DMS& Texture2DMS) {
				desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
			},
			[&](const Views::DepthStencil::Texture2DMSArray& Texture2DMSArray) {
				desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
				desc.Texture2DMSArray.FirstArraySlice = Texture2DMSArray.FirstArraySlice;
				desc.Texture2DMSArray.ArraySize = Texture2DMSArray.ArraySize;
			},
			[&](auto other) {
				assert(false);
			}
			}, view.View);


		auto native_resource = view.Resource ? view.Resource->native_resource.Get() : nullptr;
		auto size = heap.device.get_descriptor_size(heap.desc.HeapType);

		{
			auto h = heap.cpu_start;
			h.Offset(offset, size);
			heap.device.native_device->CreateDepthStencilView(native_resource, &desc, h);
		}

		if (heap.m_gpu_heap)
		{
			auto h = heap.gpu_cpu_start;
			h.Offset(offset, size);
			heap.device.native_device->CreateDepthStencilView(native_resource, &desc, h);
		}

		if constexpr (BuildOptions::Debug)	TEST(heap.device, heap.device.native_device->GetDeviceRemovedReason());

	}
	void Descriptor::place(const Views::UnorderedAccess& view)
	{
		if constexpr (BuildOptions::Debug)
		{
			Log::get() << Log::LEVEL_DEBUG << "Creating " << view << Log::endl;
		}

		Resource* counter_resource = nullptr;
		D3D12_UNORDERED_ACCESS_VIEW_DESC desc;

		desc.Format = to_native(view.Format);

		std::visit(overloaded{
			[&](const Views::UnorderedAccess::Buffer& Buffer) {
				desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
				desc.Buffer.FirstElement = Buffer.FirstElement;
				desc.Buffer.NumElements = Buffer.NumElements;
				desc.Buffer.StructureByteStride = Buffer.StructureByteStride;
				desc.Buffer.Flags = Buffer.Raw ? D3D12_BUFFER_UAV_FLAG_RAW : D3D12_BUFFER_UAV_FLAG_NONE;
				desc.Buffer.CounterOffsetInBytes = Buffer.CounterOffsetInBytes;

				counter_resource = Buffer.CounterResource;


				assert((desc.Buffer.FirstElement + desc.Buffer.NumElements) * desc.Buffer.StructureByteStride <= view.Resource->get_desc().as_buffer().SizeInBytes);
				assert(!counter_resource || Buffer.StructureByteStride);
				assert(!counter_resource || Buffer.StructureByteStride);
			},
			[&](const Views::UnorderedAccess::Texture1D& Texture1D) {
				desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
				desc.Texture1D.MipSlice = Texture1D.MipSlice;
			},
			[&](const Views::UnorderedAccess::Texture1DArray& Texture1DArray) {
				desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
				desc.Texture1DArray.MipSlice = Texture1DArray.MipSlice;
				desc.Texture1DArray.FirstArraySlice = Texture1DArray.FirstArraySlice;
				desc.Texture1DArray.ArraySize = Texture1DArray.ArraySize;
			},
			[&](const Views::UnorderedAccess::Texture2D& Texture2D) {
				desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				desc.Texture2D.MipSlice = Texture2D.MipSlice;
				desc.Texture2D.PlaneSlice = Texture2D.PlaneSlice;

				assert(desc.Texture2D.PlaneSlice == 0);
			},
			[&](const Views::UnorderedAccess::Texture2DArray& Texture2DArray) {
				desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.MipSlice = Texture2DArray.MipSlice;
				desc.Texture2DArray.PlaneSlice = Texture2DArray.PlaneSlice;
				desc.Texture2DArray.FirstArraySlice = Texture2DArray.FirstArraySlice;
				desc.Texture2DArray.ArraySize = Texture2DArray.ArraySize;

				assert(desc.Texture2DArray.PlaneSlice == 0);
			},
			[&](const Views::UnorderedAccess::Texture3D& Texture3D) {
				desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
				desc.Texture3D.MipSlice = Texture3D.MipSlice;
				desc.Texture3D.FirstWSlice = Texture3D.FirstWSlice;
				desc.Texture3D.WSize = Texture3D.WSize;
			},
			[&](auto other) {
				assert(false);
			}
			}, view.View);

		auto native_resource = view.Resource ? view.Resource->native_resource.Get() : nullptr;
		auto native_counter_resource = counter_resource ? counter_resource->native_resource.Get() : nullptr;

		auto size = heap.device.get_descriptor_size(heap.desc.HeapType);

		{
			auto h = heap.cpu_start;
			h.Offset(offset, size);

			heap.device.native_device->CreateUnorderedAccessView(native_resource, native_counter_resource, &desc, h);
			if constexpr (BuildOptions::Debug)	TEST(heap.device, heap.device.native_device->GetDeviceRemovedReason());
		}


		if (heap.m_gpu_heap)
		{
			auto h = heap.gpu_cpu_start;
			h.Offset(offset, size);
			heap.device.native_device->CreateUnorderedAccessView(native_resource, native_counter_resource, &desc, h);
			if constexpr (BuildOptions::Debug)	TEST(heap.device, heap.device.native_device->GetDeviceRemovedReason());
		}
	}

	void Descriptor::place(const Views::ConstantBuffer& view)
	{
		if constexpr (BuildOptions::Debug)
		{
			Log::get() << Log::LEVEL_DEBUG << "Creating " << view << Log::endl;
		}

		D3D12_CONSTANT_BUFFER_VIEW_DESC desc;

		desc.BufferLocation = view.Resource->get_address() + view.OffsetInBytes;
		desc.SizeInBytes = view.SizeInBytes;

		auto size = heap.device.get_descriptor_size(heap.desc.HeapType);

		{
			auto h = heap.cpu_start;
			h.Offset(offset, size);

			heap.device.native_device->CreateConstantBufferView(&desc, h);
			if constexpr (BuildOptions::Debug)	TEST(heap.device, heap.device.native_device->GetDeviceRemovedReason());
		}

		if (heap.m_gpu_heap)
		{
			auto h = heap.gpu_cpu_start;
			h.Offset(offset, size);
			heap.device.native_device->CreateConstantBufferView(&desc, h);
			if constexpr (BuildOptions::Debug)	TEST(heap.device, heap.device.native_device->GetDeviceRemovedReason());
		}
	}

	void Descriptor::operator=(const Descriptor& r)
	{
		assert(heap.desc.HeapType == r.heap.desc.HeapType);
		D3D12_DESCRIPTOR_HEAP_TYPE type = (D3D12_DESCRIPTOR_HEAP_TYPE)to_native(heap.desc.HeapType);

		auto my = heap.cpu_start;
		auto other = r.heap.cpu_start;
		auto size = heap.device.get_descriptor_size(heap.desc.HeapType);
		my.Offset(offset, size);
		other.Offset(r.offset, size);

		assert(my != other);
		heap.device.native_device->CopyDescriptorsSimple(1, my, other, type);
		if constexpr (BuildOptions::Debug)	TEST(heap.device, heap.device.native_device->GetDeviceRemovedReason());
	}

	D3D12_CPU_DESCRIPTOR_HANDLE  Descriptor::get_cpu()
	{
		return { heap.cpu_start.ptr + heap.handle_size * offset };
	}

	D3D12_GPU_DESCRIPTOR_HANDLE  Descriptor::get_gpu()
	{
		return { heap.gpu_start.ptr + heap.handle_size * offset };
	}
}