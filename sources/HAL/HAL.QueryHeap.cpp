module HAL:QueryHeap;
import Core;
import :Types;

namespace HAL
{

	std::shared_ptr<QueryHeap> QueryHeap::get_tracked() { return get_ptr<QueryHeap>(); }
	size_t QueryHeap::get_size() { return desc.Count; }

	QueryHeapFactory::ptr_type QueryHeapFactory::make_heap(QueryType type, size_t size)
	{
		QueryHeapDesc desc = { static_cast<uint>(size), type };
		return std::make_shared<HAL::QueryHeap>(device, desc);
	}

	QueryHeapFactory::QueryHeapFactory(Device& device) : device(device) {}

}
