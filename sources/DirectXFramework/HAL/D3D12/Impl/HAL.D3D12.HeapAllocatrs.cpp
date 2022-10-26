module Graphics;

import HAL;
import d3d12;
import Math;
import Utils;


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
