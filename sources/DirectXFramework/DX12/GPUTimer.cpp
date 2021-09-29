#include "pch.h"

#include "GPUTimer.h"

namespace DX12
{
	void GPUTimeManager::start(GPUTimer& timer, DX12::Eventer* list)
	{
		timer.queue_type = list->get_type();
		list->insert_time(heap, timer.id * 2);

		max_used_timers = std::max(max_used_timers, timer.id * 2);

	}

	void GPUTimeManager::end(GPUTimer& timer, DX12::Eventer*  list)
	{
		list->insert_time(heap, timer.id * 2 + 1);
	}
	float GPUTimeManager::get_time(GPUTimer& timer)
	{
		return static_cast<float>(static_cast<double>(read_back_data[2 * timer.id + 1] - read_back_data[2 * timer.id]) / frequency);
	}

	double GPUTimeManager::get_start(GPUTimer&timer)
	{
		return static_cast<double>(read_back_data[2 * timer.id]) / frequency;
	}

	double GPUTimeManager::get_end(GPUTimer&timer)
	{
		return static_cast<double>(read_back_data[2 * timer.id + 1]) / frequency;
	}

	GPUTimer::GPUTimer()
	{
		id = Singleton<GPUTimeManager>::get().get_id();
	}

	GPUTimer::~GPUTimer()
	{
		Singleton<GPUTimeManager>::get().put_id(id);
	}

	void GPUTimer::start(DX12::Eventer* list)
	{
		Singleton<GPUTimeManager>::get().start(*this, list);
	}

	void GPUTimer::end(DX12::Eventer* list)
	{
		Singleton<GPUTimeManager>::get().end(*this, list);
		list = nullptr;
	}

	float GPUTimer::get_time()
	{
		return Singleton<GPUTimeManager>::get().get_time(*this);
	}

	double GPUTimer::get_start()
	{
		return Singleton<GPUTimeManager>::get().get_start(*this);
	}

	double GPUTimer::get_end()
	{
		return Singleton<GPUTimeManager>::get().get_end(*this);
	}
}
