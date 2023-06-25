module HAL:Heap;

import HAL;
import <HAL.h>;
import <d3d12/d3d12_includes.h>;
import Core;

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

			if(native_desc.Properties.Type==D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD )
			{
			//native_desc.Properties.Type==D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_GPU_UPLOAD ;
			}
			device.native_device->CreateHeap(&native_desc, IID_PPV_ARGS(&native_heap));

			if constexpr (Debug::CheckErrors)	TEST(device, device.native_device->GetDeviceRemovedReason());
		

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

			if(native_desc.Properties.Type==D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD )
			{
				buffer->set_name("Upload Heap Buffer");
			}else if(native_desc.Properties.Type==D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_READBACK)
			{
				buffer->set_name("Readback Heap Buffer");
			}else
			{
				buffer->set_name("GPU Heap Buffer");
			}
		
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
