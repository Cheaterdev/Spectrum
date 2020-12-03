#pragma once

namespace DX12
{
	class GPUTimeManager : public Singleton<GPUTimeManager>
	{
		friend class Singleton<GPUTimeManager>;
		friend class GPUTimer;

		QueryHeap heap;
		std::mutex buffer_lock;
		std::queue<std::shared_ptr<CPUBuffer>> buffers;
	

		static const int MAX_TIMERS = 1024;
		IdGenerator ids;
		std::array<UINT64, MAX_TIMERS*2> read_back_data;

		UINT64 fence = -1;
		UINT64 frequency;
		GPUTimeManager() : heap(MAX_TIMERS * 2)
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
			std::shared_ptr<CPUBuffer> current_buffer;
			{
				std::lock_guard<std::mutex> g(buffer_lock);

				if(buffers.empty())
				{
					current_buffer.reset(new CPUBuffer(MAX_TIMERS * 2, sizeof(UINT64)));
				}
				else {
					current_buffer = buffers.front();
					buffers.pop();
				}
			}

			list->resolve_times(current_buffer.get(), heap, MAX_TIMERS * 2, [this, f, current_buffer]() {
				memcpy(read_back_data.data(), current_buffer->map<UINT64>(0, MAX_TIMERS * 2), sizeof(UINT64)*MAX_TIMERS*2);
				current_buffer->unmap();
				{
					std::lock_guard<std::mutex> g(buffer_lock);
					buffers.push(current_buffer);
				}
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