module HAL:HeapAllocators;

import :Device;
import d3d12;
import Core;


namespace HAL
{
	 std::shared_ptr<HeapFactory> HeapFactory::create_singleton()
	{
		return std::make_shared<HeapFactory>(HAL::Device::get());
	}
	 void ResourceHandle::Free()
	 {
		 if (!handle) return;
		 owner->free(handle);
	 }

}
