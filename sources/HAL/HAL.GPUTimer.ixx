export module HAL:GPUTimer;

import :Buffer;
import :Queue;
import :Device;
import :QueryHeap;
import Core;


export
{

	namespace HAL
	{
		class GPUTimeManager
		{

			friend class GPUTimer;
			Device& device;
			QueryHeapPageManager factory;
		public:

			UINT64 max_used_timers = 0;
			GPUTimeManager(Device & device) : device(device), factory(device)
			{
				
			}
			
		public:

			void read_buffer(HAL::CommandList::ptr& list, std::function<void()> f)
			{
				factory.for_each([&,this](const QueryType& type, size_t max_usage, QueryHeap::ptr heap)
				{
						list->resolve_times(*heap, max_usage, [heap](std::span<UINT64> data) {
							std::copy(data.begin(), data.end(), heap->read_back_data.begin());
							});
				});

				list->on_done(f);
			}

		};


	}
}

