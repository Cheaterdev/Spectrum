export module HAL:Heap;
import d3d12;
import Math; 
import Utils;

import :Types;
import :Sampler;
import :Utils;
import :Device;

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
	protected:
		std::byte* cpu_address = nullptr;
	public:
		using ptr = std::shared_ptr<Heap>;
		void init(Device& device, const HeapDesc& desc);
		~Heap();

		std::span<std::byte> cpu_data();

	public:
		D3D::Heap native_heap;
		D3D::Resource cpu_buffer;
	};
}

