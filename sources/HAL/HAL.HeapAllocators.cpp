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

	void HeapFactory::_add(HeapIndex index, size_t n) noexcept
	{
		if (index.type == HeapType::UPLOAD)        _upload_bytes   += n;
		else if (index.type == HeapType::READBACK) _readback_bytes += n;
	}

	void HeapFactory::_sub(HeapIndex index, size_t n) noexcept
	{
		if (index.type == HeapType::UPLOAD)        _upload_bytes   -= n;
		else if (index.type == HeapType::READBACK) _readback_bytes -= n;
	}

	HeapFactory::page_type HeapFactory::AllocatePage(HeapIndex index, size_t size, Allocators::PageOwnerInterface<HAL::Heap>& owner)
	{
		auto page = Base::AllocatePage(index, size, owner);
		_add(index, page->inner_heap_page_handle.get_size());
		return page;
	}

	void HeapFactory::Free(HeapIndex index, page_type page)
	{
		_sub(index, page->inner_heap_page_handle.get_size());
		Base::Free(index, page);
	}

	HeapFactory::HeapFactory(Device& device) : device(device) {}

}
