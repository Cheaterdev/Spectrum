module Graphics:Memory;

import HAL;
import d3d12;
import Math;
import Utils;
import :Device;

namespace HAL
{
	 std::shared_ptr<HeapFactory> HeapFactory::create_singleton()
	{
		return std::make_shared<HeapFactory>(Graphics::Device::get());
	}

}
