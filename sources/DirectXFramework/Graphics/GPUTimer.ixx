export module Graphics:GPUTimer;

import :Buffer;
import Singleton;
import IdGenerator;

import HAL;
//using namespace HAL;
import :Definitions;


export
{

	namespace Graphics
	{


		class QueryHeap
		{

			ComPtr<ID3D12QueryHeap> heap;
		public:
			QueryHeap(UINT max_count, D3D12_QUERY_HEAP_TYPE type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP);

			ComPtr<ID3D12QueryHeap> get_native();

			using ptr = std::shared_ptr<QueryHeap>;
		};

		class GPUTimeManager : public Singleton<GPUTimeManager>
		{
			friend class Singleton<GPUTimeManager>;
			friend class GPUTimer;

			QueryHeap heap;
			std::mutex buffer_lock;
			static const int MAX_TIMERS = 1024;
			IdGenerator<Thread::Lockable> ids;
			std::array<UINT64, MAX_TIMERS * 2> read_back_data;

			UINT64 fence = -1;
			UINT64 frequency;

			int max_used_timers = 0;
			GPUTimeManager() : heap(MAX_TIMERS * 2)
			{
				HAL::Device::get().get_queue(CommandListType::DIRECT)->get_native()->GetTimestampFrequency(&frequency);
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
			void start(GPUTimer&, Graphics::Eventer* list);
			void end(GPUTimer&, Graphics::Eventer* list);
			float get_time(GPUTimer&);

			double get_start(GPUTimer&);
			double get_end(GPUTimer&);

			double get_now()
			{
				UINT64 cpu_start;
				UINT64 gpu_start;
				HAL::Device::get().get_queue(CommandListType::DIRECT)->get_native()->GetClockCalibration(&gpu_start, &cpu_start);
				return static_cast<double>(gpu_start) / frequency;
			}

			void read_buffer(CommandList::ptr& list, std::function<void()> f)
			{
				list->resolve_times(heap, max_used_timers, [this, f](std::span<UINT64> data) {
					std::copy(data.begin(), data.end(), read_back_data.begin());
					f();
					});
			}

		};


	}
}

