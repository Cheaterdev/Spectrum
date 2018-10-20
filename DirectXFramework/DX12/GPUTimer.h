#pragma once

namespace DX12
{
	class GPUTimeManager : public Singleton<GPUTimeManager>
	{
		friend class Singleton<GPUTimeManager>;
		friend class GPUTimer;

		QueryHeap heap;
		CPUBuffer buffer;
		static const int MAX_TIMERS = 4096;
		IdGenerator ids;
		std::array<UINT64, MAX_TIMERS*2> read_back_data;

		UINT64 fence = -1;
		UINT64 frequency;
		GPUTimeManager() : heap(MAX_TIMERS * 2), buffer(MAX_TIMERS * 2, sizeof(UINT64))
		{
			DX12::Device::get().get_queue(CommandListType::DIRECT)->get_native()->GetTimestampFrequency(&frequency);
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
		void start(GPUTimer&, DX12::Eventer*  list);
		void end(GPUTimer&, DX12::Eventer*  list);
		float get_time(GPUTimer&);

		double get_start(GPUTimer&);
		double get_end(GPUTimer&);

		double get_now()
		{
			UINT64 cpu_start;
			UINT64 gpu_start;
			DX12::Device::get().get_queue(CommandListType::DIRECT)->get_native()->GetClockCalibration(&gpu_start, &cpu_start);
			return static_cast<double>(gpu_start) / frequency;
		}


		/*	void start_read()
			{
				if (fence != -1)
					DX12::Device::get().get_queue(CommandListType::DIRECT)->wait(fence);

				read_back_data = buffer.map<UINT64>(0, MAX_TIMERS * 2);
			}

			void end_read()
			{
				buffer.unmap();
				auto list = DX12::Device::get().get_queue(CommandListType::DIRECT)->get_free_list();
				list->begin();
				list->resolve_times(&buffer, heap, MAX_TIMERS * 2);
				list->end();
				fence = list->execute();
			}
			*/
		void read_buffer(CommandList::ptr &list, std::function<void()> f)
		{

			list->resolve_times(&buffer, heap, MAX_TIMERS * 2, [this, f]() {
				memcpy(read_back_data.data(),buffer.map<UINT64>(0, MAX_TIMERS * 2), sizeof(UINT64)*MAX_TIMERS*2);
				buffer.unmap();
				f();
			
			});
		}

		/*
		void read_buffer(std::function<void()> f)
		{
			auto list = DX12::Device::get().get_queue(CommandListType::DIRECT)->get_free_list();
			list->begin();
			list->resolve_times(&buffer, heap, MAX_TIMERS * 2, [this, f]() {
				read_back_data = buffer.map<UINT64>(0, MAX_TIMERS * 2);
				f();
				buffer.unmap();
			});

			list->end();
			list->execute();



		}*/
	};


}