#include "pch.h"


namespace DX12
{
	void GPUTimeManager::start(GPUTimer& timer, DX12::Eventer*  list)
	{
		list->insert_time(heap, timer.id * 2);
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



}