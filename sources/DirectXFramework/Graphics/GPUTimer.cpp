module;


module Graphics:GPUTimer;

import HAL;
namespace Graphics
{




	void GPUTimeManager::start(HAL::GPUTimer& timer, HAL::Eventer* list)
	{
		timer.queue_type = list->get_type();
		list->insert_time(heap, timer.id * 2);

		max_used_timers = std::max(max_used_timers, timer.id * 2);

	}

	void GPUTimeManager::end(HAL::GPUTimer& timer, HAL::Eventer* list)
	{
		list->insert_time(heap, timer.id * 2 + 1);
	}
	float GPUTimeManager::get_time(HAL::GPUTimer& timer)
	{
		return static_cast<float>(static_cast<double>(read_back_data[2 * timer.id + 1] - read_back_data[2 * timer.id]) / frequency);
	}

	double GPUTimeManager::get_start(HAL::GPUTimer& timer)
	{
		return static_cast<double>(read_back_data[2 * timer.id]) / frequency;
	}

	double GPUTimeManager::get_end(HAL::GPUTimer& timer)
	{
		return static_cast<double>(read_back_data[2 * timer.id + 1]) / frequency;
	}

}
