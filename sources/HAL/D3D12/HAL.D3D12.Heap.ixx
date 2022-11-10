export module HAL:API.Heap;
import d3d12;
import Core;

import :Types;
import :Sampler;
import :Utils;
import :API.Device;

export namespace HAL
{
	namespace API
	{
		class Heap
		{
		protected:
			GPUAddressPtr gpu_address = 0;
			std::byte* cpu_address = nullptr;
		public:
			virtual ~Heap();

			GPUAddressPtr get_address() const;
		public:
			D3D::Heap native_heap;
			D3D::Resource cpu_buffer;
		};
	}
}

