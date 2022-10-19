module Graphics:Memory;

import HAL;
import d3d12;
import Math;
import Utils;

#undef THIS
namespace HAL
{
	Heap::Heap(Device& device, const HeapDesc& desc):desc(desc)
	{
			D3D12_HEAP_DESC native_desc;
			native_desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
			native_desc.Flags = ::to_native(desc.Flags);
			native_desc.SizeInBytes = desc.Size;
			native_desc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			native_desc.Properties.CreationNodeMask = 0;
			native_desc.Properties.VisibleNodeMask = 0;
			native_desc.Properties.Type = ::to_native(desc.Type);
			native_desc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

			device.native_device->CreateHeap(&native_desc, IID_PPV_ARGS(&native_heap));


			if (desc.Flags != HeapFlags::TEXTURES_ONLY && desc.Flags != HeapFlags::RTDS_ONLY)
			{
				CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(desc.Size);

				D3D12_RESOURCE_STATES state = desc.Type == HeapType::UPLOAD ? D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST;
				device.native_device->CreatePlacedResource(
					native_heap.Get(),
					0,
					&resourceDesc,
					state,
					nullptr,
					IID_PPV_ARGS(&cpu_buffer));


				gpu_address = (GPUAddressPtr)cpu_buffer->GetGPUVirtualAddress();

				if (desc.Type != HeapType::DEFAULT)
				{
					D3D12_RANGE Range;
					Range.Begin = 0;
					Range.End = desc.Size;


					cpu_buffer->Map(0, &Range, reinterpret_cast<void**>(&cpu_address));

				}
			}

			buffer.reset(new HAL::Resource(/*device,*/ ResourceDesc::Buffer(desc.Size), PlacementAddress { this,0 }));

		}

	
		
		std::span<std::byte> Heap::get_data()
		{
			return std::span(cpu_address, cpu_address ? desc.Size : 0);
		}

		namespace API
		{
			Heap::~Heap()
			{
				if (cpu_address) cpu_buffer->Unmap(0, nullptr);
			}
			GPUAddressPtr Heap::get_address() const
			{
				return gpu_address;
			}

		}
}
