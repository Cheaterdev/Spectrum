#include "pch.h"
/*
Scheduler::Scheduler()
{
	pTaskManager = std::make_shared<TaskManager>();

	bKeepExecution = true;
	bNeedScheduling = false;
	SchedulerThread = std::make_shared<boost::thread>(ShedulerThreadFunction, this);
}

Scheduler::~Scheduler()
{
	bKeepExecution = false;
	scheduling_condtion.notify_all();
	SchedulerThread->join();

	pTaskManager.reset();
}

void Scheduler::schedule(base_task::Ptr pTask, scheduler_time_point execution_time)
{
	boost::lock_guard<boost::mutex> lock(task_mutex);

	pTask->SetStartTime(execution_time);
	tasks.merge({ pTask }, [](base_task::Ptr pEl1, base_task::Ptr pEl2) { return pEl1->GetStartTime()<pEl2->GetStartTime(); });
	bNeedScheduling = true;
	scheduling_condtion.notify_one();
}

void Scheduler::ShedulerThreadFunction(Scheduler* pScheduler)
{
	assert(pScheduler);

	while (pScheduler->bKeepExecution)
	{
		scheduler_time_point wait_time = pScheduler->schedule_tasks();

		// Wait for task scheduling
		{
			boost::unique_lock<boost::mutex> lock(pScheduler->task_mutex);
			pScheduler->scheduling_condtion.wait_until(lock, wait_time, [&]() {
				return pScheduler->bNeedScheduling || !pScheduler->bKeepExecution;
			});
		}
	}
}

Scheduler::scheduler_clock::time_point Scheduler::schedule_tasks()
{
	boost::lock_guard<boost::mutex> lock(task_mutex);
	scheduler_clock::time_point now = scheduler_clock::now();

	// Get all tasks for execution
	std::list<base_task::Ptr>::iterator it = std::find_if(tasks.begin(), tasks.end(), [&](base_task::Ptr pTask) {
		return now<pTask->GetStartTime();
	});

	// Add task for execution
	//pTaskManager->add_task_range(tasks)
	std::for_each(tasks.begin(), it, [&](base_task::Ptr pTask) {
		pTaskManager->add_task(pTask);
	});

	// Calculate waiting time
	scheduler_clock::time_point wait_time;
	if (it == tasks.end())
		wait_time = now + boost::chrono::microseconds(100); // scheduler frequency
	else
		wait_time = (*it)->GetStartTime();

	// Erase this task from execution table
	tasks.erase(tasks.begin(), it);

	bNeedScheduling = false;

	return wait_time;
}*/