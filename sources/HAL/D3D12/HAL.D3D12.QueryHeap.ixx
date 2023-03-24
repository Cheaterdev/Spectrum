export module HAL:API.QueryHeap;
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
		class QueryHeap
		{
		protected:
			ComPtr<ID3D12QueryHeap> heap;
		public:
			ComPtr<ID3D12QueryHeap> get_native() const;
		};
	}
}

