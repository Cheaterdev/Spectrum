module HAL:HeapAllocators;

import :Device;
import d3d12;
import Core;


namespace HAL
{

	 void ResourceHandle::Free()
	 {
		 if (!handle) return;
		 owner->free(handle);
	 }

}
