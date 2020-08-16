#pragma once
/*
#include "boost\thread.hpp"

class Scheduler
{
public:
	Scheduler();
	~Scheduler();

	typedef std::shared_ptr<Scheduler> ptr;
	typedef boost::chrono::steady_clock scheduler_clock;
	typedef boost::chrono::steady_clock::time_point scheduler_time_point;

	void schedule(base_task::Ptr pTask, scheduler_time_point execution_time = scheduler_clock::now());
private:

	bool bKeepExecution;
	boost::condition_variable scheduling_condtion;
	std::shared_ptr<boost::thread> SchedulerThread;
	static void ShedulerThreadFunction(Scheduler* pScheduler);

	scheduler_clock::time_point schedule_tasks();
	boost::mutex task_mutex;
	bool bNeedScheduling; // if true then we add task that need scheduler attention
	std::list<base_task::Ptr> tasks;

	TaskManager::ptr pTaskManager;
};*/