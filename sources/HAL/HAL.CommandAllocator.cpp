module HAL:CommandAllocator;
import Core;
import :Types;

namespace HAL
{

	CommandListType CommandAllocator::get_type() const { return type; }

}
