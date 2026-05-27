module;
#include "BSPoolBridge.h"
module Core:Scheduler;

import :Threading;
import stl.core;
import stl.threading;

using namespace std;

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

scheduler::scheduler()
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


scheduler::~scheduler()
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
