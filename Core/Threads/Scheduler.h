#pragma once

#include <ppltasks.h>
#include <ppl.h>
#include <agents.h>
#include "../Profiling/Profiling.h"
#include <boost/lockfree/spsc_queue.hpp>
using namespace concurrency;

#define GSysInfo SystemInfo::get()
enum class TaskPriority : int
{
	LOW,
	NORMAL,
	HIGH
};

class thread_pool : public Singleton<thread_pool>
{

	friend class  Singleton<thread_pool>;

public:

	template<class F/*, class... Args*/>
	auto enqueue(F&& f, TaskPriority priority = TaskPriority::NORMAL/*, Args&& ... args*/)
		->std::future<typename std::result_of<F(/*Args...*/)>::type>;

private:

	explicit thread_pool();
	virtual    ~thread_pool();

	bool stop;
};

// the constructor just launches some amount of workers
inline thread_pool::thread_pool()
	: stop(false)
{
	
}

// add new work item to the pool
template<class F/*, class... Args*/>
auto thread_pool::enqueue(F&& f, TaskPriority priority/*, Args&& ... args*/)
-> std::future<typename std::result_of<F(/*Args...*/)>::type>
{
	if (stop) throw std::exception("wtf");
	using return_type = typename std::result_of<F(/*Args...*/)>::type;

	auto task = std::make_shared< std::packaged_task<return_type()> >(
		std::bind(std::forward<F>(f)/*, std::forward<Args>(args)...*/)
		);

	std::future<return_type> res = task->get_future();


	create_task([task]() {(*task)(); });
	return res;
}

// the destructor joins all threads
inline thread_pool::~thread_pool()
{
	stop = true;
}


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
	// thread_pool& pool;
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
			std::chrono::steady_clock::time_point start_time;

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

						// yeah! we have a task!
						if (!tasks.empty())
						{
							start_time = tasks.front().start_time;
							return true;
						}

						return false;
					});

					if (!local_alive)
						break;
				}

				//wait for timed task
				std::unique_lock<std::mutex>locker(queue_mutex);
				condition.wait_until(locker, start_time, [&]
				{
					if (!alive)
					{
						local_alive = false;
						return true;
					}

					// if new task was inserted and we need to reinitialize wait_until
					if (tasks.size() && tasks.front().start_time < start_time)
					{
						start_time = tasks.front().start_time;
						need_wait = false;
						return true;
					}

					// if it's time to do our task(s)
					while (tasks.size())
						if (tasks.front().start_time <= std::chrono::steady_clock::now())
						{
							thread_pool::get().enqueue(tasks.front().function, TaskPriority::HIGH);
							tasks.pop_front();
							need_wait = tasks.empty();

							if (!need_wait)
								start_time = tasks.front().start_time;
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
	auto enqueue(F&& f, std::chrono::steady_clock::time_point time, Args&& ... args)
		->std::future<typename std::result_of<F(Args...)>::type>
	{

		if (time <= std::chrono::steady_clock::now())
			return  thread_pool::get().enqueue(f, TaskPriority::HIGH);

		using return_type = typename std::result_of<F(Args...)>::type;

		auto task = std::make_shared< std::packaged_task<return_type()> >(
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


class SingleThreadExecutor : public concurrency::agent, public concurrency::unbounded_buffer<std::function<void()>>

{
public:
	explicit SingleThreadExecutor(bool start = true)
	{
		if (start)
			concurrency::agent::start();
	}

	void stop_and_wait()
	{
		enqueue(nullptr);

		concurrency::agent::wait(this);
	}

protected:
	void run()
	{

		// Read from the source block until we receive the 
		// sentinel value.
		std::function<void()> n;
		while ((n = receive(*this)) != nullptr)
		{
			n();
		}


		// Set the agent to the finished state.
		done();
	}

private:
	// The source buffer to read from.
	//ISource<std::function<void()>>& _source;

};




using Batch = std::vector<std::function<void()>>;
class SingleThreadExecutorBatched /*: public concurrency::agent, public concurrency::unbounded_buffer<Batch>*/

{
	concurrency::task<void> task;
	boost::lockfree::spsc_queue<int, boost::lockfree::capacity<7> > spsc_queue;
public:
	explicit SingleThreadExecutorBatched(int count = 16, bool start = true)
	{
		for(auto &d:datas)
d.reserve(count);

		index = 0;
	/*	if (start)
			concurrency::agent::start();*/

		task= create_task([this]
	{
		run();
	});
	}

	void flush()
	{
		while (!spsc_queue.push(index));

		index = (index+1)%datas.size();
		
		//current_data->clear();
	}

	void stop_and_wait()
	{
		add(nullptr);
		flush();
		task.wait();
	//	
	/*	enqueue(Batch());*/

	//	wait(this);
	}

	void add(std::function<void()>&& f)
	{

	//	while (!spsc_queue.push(f));
		//auto new_c = std::make_shared<Render::context>(render_context);
		datas[index].emplace_back(std::forward<std::function<void()>>(f));

		if (datas[index].capacity() == datas[index].size())
		{
			flush();
		}

	
	}

protected:
	std::array<Batch,8> datas;
	int index;

	void run()
	{
		bool alive = true;
		while(alive)
		while (alive&&spsc_queue.consume_one([&](int id)
		{	
		//	auto &timer = Profiler::get().start(L"Batch");
			for (auto &f : datas[id]) 
				if(f)
					f();
			else
				alive = false;

			datas[id].clear();
		}));
		// Read from the source block until we receive the 
		// sentinel value.
	/*	Batch n;
		while (!(n = receive(*this)).empty())
		{
			auto &timer = Profiler::get().start(L"Batch");
			for (auto &f : n) f();
		}


		// Set the agent to the finished state.
		done();*/
	}

private:
	// The source buffer to read from.
	//ISource<std::function<void()>>& _source;

};
