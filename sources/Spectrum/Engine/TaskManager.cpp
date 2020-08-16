#include "pch.h"
#include "TaskManager.h"
/*
base_task::base_task()
{
	InputCount = 0;
	CurrentInputs = 0;
}

base_task::~base_task()
{}

void base_task::ExecuteTask()
{
	// Execute main work
	ProcessTask();

	// Notify tasks about execution of current task
	NotifyOutputConnections();
}

void base_task::ConnectToTask(std::shared_ptr<base_task> pTask)
{
	if (!pTask)
		return;

	OutputConnections.push_back(pTask);
	pTask->InputCount++;
}

void base_task::Notify()
{
	if (++CurrentInputs == InputCount)
	{
		// Last connected task was executed

		// Reset counters
		CurrentInputs = 0;

		// Add current task to task queue
	}
}

void base_task::NotifyOutputConnections()
{
	for (auto pTask : OutputConnections)
	{
		auto pLockedTask = pTask.lock();
		if (pLockedTask)
		{
			pLockedTask->Notify();
		}
	}
}

TaskManager::TaskManager()
{
	// Get logic core count
	int nCores = GSysInfo.GetCoreCount();

	// Create threads
	bKeepExecution = true;
	for (int i = 0; i < nCores; ++i)
		ThreadPool.push_back(boost::thread(ThreadPower, this));
}

TaskManager::~TaskManager()
{
	// Stop all threads
	bKeepExecution = false;
	TaskCondition.notify_all();
	for (auto& thread : ThreadPool)
		thread.join();
}

void TaskManager::add_task(std::shared_ptr<base_task> pTask)
{
	if (!bKeepExecution)
		return;

	// Add task to queue
	{
		boost::lock_guard<boost::mutex> lock(TaskQueueMutex);
		TaskQueue.push_back(pTask);
	}

	TaskCondition.notify_one();
}

std::shared_ptr<base_task> TaskManager::GetTaskQueue()
{
	boost::lock_guard<boost::mutex> lock(TaskQueueMutex);

	std::shared_ptr<base_task> pTask = nullptr;
	if (bKeepExecution && !TaskQueue.empty())
	{
		pTask = TaskQueue.front();
		TaskQueue.pop_front();
	}

	return pTask;
}

void TaskManager::ThreadPower(TaskManager* pTaskManager)
{
	assert(pTaskManager);

	while (pTaskManager->bKeepExecution)
	{
		// Waiting for task
		{
			boost::unique_lock<boost::mutex> lock(pTaskManager->TaskQueueMutex);
			pTaskManager->TaskCondition.wait(lock, [&](){ return !pTaskManager->TaskQueue.empty() || !pTaskManager->bKeepExecution; });
		}


		// Get Task and Execute it
		std::shared_ptr<base_task> pTask = pTaskManager->GetTaskQueue();
		if (pTask)
			pTask->ExecuteTask();
		else
			boost::this_thread::sleep_for(boost::chrono::microseconds(10));
	}
}*/