module HAL:HeapAllocators;
import Core;
import :Types;

namespace HAL
{

	HeapFactory::ptr_type HeapFactory::make_heap(HeapIndex index, size_t size)
	{
		HeapDesc desc = { size, index.type, index.memory, HeapFlags::NONE };
		return std::make_shared<HAL::Heap>(device, desc);
	}

	HeapFactory::HeapFactory(Device& device) : device(device) {}

}
