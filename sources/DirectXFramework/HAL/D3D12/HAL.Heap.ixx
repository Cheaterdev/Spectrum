export module HAL.Heap;
import d3d12;
import Vectors;
import Utils;

import HAL.Types;
import HAL.Sampler;

import D3D12.Utils;
import HAL.Device;

using namespace HAL;


export namespace HAL
{
	struct HeapDesc
	{
		size_t Size;
		HeapType Type;
		MemoryType Memory;
		HeapFlags Flags;
	};

	class Heap
	{
		std::byte* cpu_address = nullptr;
		HeapDesc desc;
	public:
		using ptr = std::shared_ptr<Heap>;
		Heap(Device& device, const HeapDesc& desc);
		~Heap();

		std::span<std::byte> cpu_data();

	public:
		D3D::Heap native_heap;
		D3D::Resource cpu_buffer;
	};
}

module:private;


namespace HAL
{
	Heap::Heap(Device& device, const HeapDesc& desc):desc(desc)
	{
		D3D12_HEAP_DESC native_desc;
		native_desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		native_desc.Flags = to_native(desc.Flags);
		native_desc.SizeInBytes = desc.Size;
		native_desc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		native_desc.Properties.CreationNodeMask = 0;
		native_desc.Properties.VisibleNodeMask = 0;
		native_desc.Properties.Type = to_native(desc.Type);
		native_desc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		device.native_device->CreateHeap(&native_desc, IID_PPV_ARGS(&native_heap));


		if (desc.Type != HeapType::DEFAULT)
		{
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

				D3D12_RANGE Range;
				Range.Begin = 0;
				Range.End = desc.Size;

				cpu_buffer->Map(0, &Range, reinterpret_cast<void**>(&cpu_address));

			}
		}
		
	}

	
	Heap::~Heap()
	{
		if(cpu_buffer) cpu_buffer->Unmap(0, nullptr);
	}

	std::span<std::byte> Heap::cpu_data()
	{
		return std::span(cpu_address, cpu_address? desc.Size: 0);
	}

}