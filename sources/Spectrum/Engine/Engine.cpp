#include "pch.h"
/*
Engine::Engine()
{
	// Initialize engine
	Log::get() << "Start log" << Log::endl;
#ifndef ALTERNATIVE_SCHEDULER
	pScheduler = std::shared_ptr<Scheduler>(new Scheduler);
#endif
}

Engine::~Engine()
{
	pScheduler.reset(); // Should free it before managers

	// Destroy
	Log::get() << "Shutdown engine" << Log::endl;
}

void Engine::RegisterSystem(base_system::ptr pSystem)
{
	Systems.push_back(pSystem);
}

void Engine::Run()
{
	// Singlethreaded
	for (auto pSystem : Systems)
	{
		pSystem->execute_task();
	}
}*/