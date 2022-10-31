export module HAL:GPUTimer;

import :Buffer;
import :Queue;
import Singleton;
import IdGenerator;



export
{

	namespace HAL
	{



		class GPUTimeManager : public Singleton<GPUTimeManager>
		{
			friend class Singleton<GPUTimeManager>;

		public:
			//riend class GPUTimer;

			HAL::QueryHeap heap;
			std::mutex buffer_lock;
			static const int MAX_TIMERS = 1024;
			IdGenerator<Thread::Lockable> ids;
			std::array<UINT64, MAX_TIMERS * 2> read_back_data;

			UINT64 fence = -1;
			UINT64 frequency;

			int max_used_timers = 0;
			GPUTimeManager() : heap(HAL::Device::get() ,{ MAX_TIMERS * 2 })
			{
				HAL::Device::get().get_queue(HAL::CommandListType::DIRECT)->get_native()->GetTimestampFrequency(&frequency);
			}
			unsigned int get_id()
			{
				return ids.get();
			}

			void put_id(unsigned int id)
			{
				ids.put(id);
			}
		public:
			void start(HAL::GPUTimer&, HAL::Eventer* list);
			void end(HAL::GPUTimer&, HAL::Eventer* list);
			float get_time(HAL::GPUTimer&);

			double get_start(HAL::GPUTimer&);
			double get_end(HAL::GPUTimer&);

			double get_now()
			{
				UINT64 cpu_start;
				UINT64 gpu_start;
				HAL::Device::get().get_queue(HAL::CommandListType::DIRECT)->get_native()->GetClockCalibration(&gpu_start, &cpu_start);
				return static_cast<double>(gpu_start) / frequency;
			}

			void read_buffer(HAL::CommandList::ptr& list, std::function<void()> f)
			{
				list->resolve_times(heap, max_used_timers, [this, f](std::span<UINT64> data) {
					std::copy(data.begin(), data.end(), read_back_data.begin());
					f();
					});
			}

		};


	}
}

