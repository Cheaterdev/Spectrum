export module Core:Scheduler;

import :Profiling;
import :Threading;
import :Singleton;
import stl.core;
import stl.threading;
import ppl;

using namespace concurrency;
using namespace std;


export
{

enum class TaskPriority : int
{
	LOW,
	NORMAL,
	HIGH
};

class thread_pool : public Singleton<thread_pool>
{
	friend class Singleton<thread_pool>;

public:
	template<class F>
	auto enqueue(F&& f) -> std::future<typename std::invoke_result<F>::type>
	{
		if (stop) throw std::exception("wtf");
		using return_type = typename std::invoke_result<F>::type;
		auto task = std::make_shared<std::packaged_task<return_type()>>(std::forward<F>(f));
		auto fut = task->get_future();
		submit_impl([task]() { (*task)(); });
		return fut;
	}

private:
	struct Impl;
	std::unique_ptr<Impl> pimpl;
	void submit_impl(std::function<void()> f);
	explicit thread_pool();
	virtual ~thread_pool();
	bool stop = false;
};


class scheduler : public Singleton<scheduler>
{

	friend class Singleton<scheduler>;

	struct scheduled_task
	{
		std::function<void()> function;
		std::chrono::steady_clock::time_point start_time;

		bool operator<(const scheduled_task& other) const
		{
			return start_time < other.start_time;
		}
	};
	std::mutex queue_mutex;
	std::list<scheduled_task> tasks;
	std::thread main_thread;
	std::condition_variable condition;
	bool alive;
	scheduler()
	{
		main_thread = std::thread([this]()
		{
			SetThreadName(std::string("Main Scheduler"));
			alive = true;
			auto need_wait = true;
			auto local_alive = true;
			std::chrono::steady_clock::time_point start_time_local;

			while (local_alive)
			{
				if (need_wait)
				{
					std::unique_lock<std::mutex>locker(queue_mutex);
					condition.wait(locker, [&]
					{
						if (!alive)
						{
							local_alive = false;
							return true;
						}

						if (!tasks.empty())
						{
							start_time_local = tasks.front().start_time;
							return true;
						}

						return false;
					});

					if (!local_alive)
						break;
				}

				std::unique_lock<std::mutex>locker(queue_mutex);
				condition.wait_until(locker, start_time_local, [&]
				{
					if (!alive)
					{
						local_alive = false;
						return true;
					}

					if (tasks.size() && tasks.front().start_time < start_time_local)
					{
						start_time_local = tasks.front().start_time;
						need_wait = false;
						return true;
					}

					while (tasks.size())
						if (tasks.front().start_time <= std::chrono::steady_clock::now())
						{
							thread_pool::get().enqueue(tasks.front().function);
							tasks.pop_front();
							need_wait = tasks.empty();

							if (!need_wait)
								start_time_local = tasks.front().start_time;
						}
						else
							break;

					return false;
				});
			}
		});
	}


	virtual ~scheduler()
	{
		{
			queue_mutex.lock();
			alive = false;
			queue_mutex.unlock();
		}
		condition.notify_one();
		main_thread.join();
		tasks.clear();
		thread_pool::reset();
	}
public:
	template<class F, class... Args>
	auto enqueue_now(F&& f, Args&& ... args)
		->std::future<typename std::invoke_result<F>::type>
	{
		return thread_pool::get().enqueue(f);
	}

	template<class F, class... Args>
	auto enqueue(F&& f, std::chrono::steady_clock::time_point time, Args&& ... args)
		->std::future<typename std::invoke_result<F>::type>
	{
		if (time <= std::chrono::steady_clock::now())
			return thread_pool::get().enqueue(f);

		using return_type = typename std::invoke_result<F>::type;

		auto task = std::make_shared<std::packaged_task<return_type()>>(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
			);

		std::future<return_type> res = task->get_future();
		{
			std::unique_lock<std::mutex> lock(queue_mutex);

			auto need_notify = tasks.empty() || time < tasks.front().start_time;

			scheduled_task t;
			t.start_time = time;
			t.function = [task]() { (*task)(); };

			auto it = tasks.begin();

			while (it != tasks.end() && it->start_time > time) ++it;

			tasks.insert(it, t);

			if (need_notify)
				condition.notify_one();
		}

		return res;
	}


};



}


//module : private;

#include "BSPoolBridge.h"	
struct thread_pool::Impl
{
	BSPoolHandle* handle;
	Impl() : handle(BSPool_Create()) {}
	~Impl() { BSPool_Destroy(handle); }
};

thread_pool::thread_pool() : pimpl(std::make_unique<Impl>()), stop(false) {}

thread_pool::~thread_pool()
{
	stop = true;
}

void thread_pool::submit_impl(std::function<void()> f)
{
	struct Box { std::function<void()> fn; };
	auto* box = new Box{ std::move(f) };
	BSPool_Submit(pimpl->handle, box, [](void* ctx)
	{
		std::unique_ptr<Box> owned{ static_cast<Box*>(ctx) };
		owned->fn();
	});
}