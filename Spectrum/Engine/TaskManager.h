#pragma once

/*
TODO:
task profiling
task cycling
task arguments, returns values
*/
/*
#include <boost/thread.hpp>
#include <thread>
#include <condition_variable>
#include <future>
#include <functional>

class base_task
{
public:
	base_task();
	virtual ~base_task();

	typedef std::shared_ptr<base_task> Ptr;

	void SetStartTime(boost::chrono::steady_clock::time_point time) { start = time; }
	boost::chrono::steady_clock::time_point GetStartTime() const { return start; }

	void ExecuteTask();
	void ConnectToTask(std::shared_ptr<base_task> pTask);

protected:
	boost::chrono::steady_clock::time_point start;

protected:
	virtual void ProcessTask() = 0;

	// Input Connection block
private:
	void Notify();
	int InputCount;
	std::atomic_int CurrentInputs;
	// Output connections block
private:
	void NotifyOutputConnections();
	std::vector<std::weak_ptr<base_task>> OutputConnections;
};



template<typename T>
class ClassTask : public base_task
{
public:
	typedef void(T::*TaskFunction)();

	ClassTask(T* pObject, TaskFunction func)
	{
		pTaskObject = pObject;
		Task = func;
	}
	void ProcessTask()
	{
		(pTaskObject->*Task)();
	}
private:
	T* pTaskObject;
	TaskFunction Task;
};

class TaskManager
{
public:
	TaskManager();
	~TaskManager();

	typedef std::shared_ptr<TaskManager> ptr;

	void add_task(std::shared_ptr<base_task> pTask);

	template<typename _Iter>
	void add_task_range(_Iter begin, _Iter end)
	{
		if (!bKeepExecution)
			return;

		{
			boost::lock_guard<boost::mutex> lock(TaskQueueMutex);
			TaskQueue.insert(TaskQueue.end(), begin, end);
		}

		TaskCondition.notify_all();
	}
private:
	boost::mutex TaskQueueMutex;
	boost::condition_variable_any TaskCondition;
	std::list<std::shared_ptr<base_task>> TaskQueue;
	std::shared_ptr<base_task> GetTaskQueue();

	bool bKeepExecution;
	std::vector<boost::thread> ThreadPool;
	static void ThreadPower(TaskManager* pTaskManager);
};*/