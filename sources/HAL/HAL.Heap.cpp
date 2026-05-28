module HAL:Heap;
import Core;
import :Types;

namespace HAL
{

	HeapType Heap::get_type() { return desc.Type; }
	size_t Heap::get_size() { return desc.Size; }
	std::shared_ptr<Buffer> Heap::as_buffer() { return buffer; }

}
