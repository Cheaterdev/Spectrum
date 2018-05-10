#pragma once

#include <ppltasks.h>
#include <ppl.h>
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

	/*    void enqueue_task(std::function<void()> f)
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			tasks.emplace(f);
			condition.notify_one();
		}
		*/
	bool is_free() const
	{
		return free_count == workers.size();
	}

	void wait_for_all()
	{
		{
			std::unique_lock<std::mutex> lock(this->free_mutex);
			this->condition_free.wait(lock,
				[this]
			{

				return is_free() && this->tasks.empty();

			});
		}
	}


private:

	explicit thread_pool(size_t size = 32);
	virtual    ~thread_pool();



	uint32_t free_count = 0;
	uint32_t high_tasks = 0;
	// need to keep track of threads so we can join them
	std::vector< std::thread > workers;

	struct TaskInfo
	{
		std::function<void()> func;
		TaskPriority priority = TaskPriority::NORMAL;
		std::string task_name;
		TaskInfo(const std::function<void()>& f, TaskPriority p) noexcept : func(std::move(f)), priority(p) {}

		TaskInfo(const TaskInfo&& o) noexcept : func(std::move(o.func)), priority(o.priority) {}
		TaskInfo() = default;

		void operator=(const TaskInfo&& o)
		{
			func = (std::move(o.func));
			priority = (o.priority);
		};
	};
	// the task queue
	std::priority_queue<TaskInfo, std::vector<TaskInfo>, std::function<bool(const TaskInfo&, const TaskInfo&)>> tasks;

	// synchronization
	std::mutex queue_mutex;
	std::condition_variable condition;

	std::mutex free_mutex;

	std::condition_variable condition_free;
	bool stop;
};

// the constructor just launches some amount of workers
inline thread_pool::thread_pool(size_t threads)
	: stop(false), tasks([](const TaskInfo & a, const TaskInfo & b)->bool
{
	return static_cast<int>(a.priority) < static_cast<int>(b.priority);
})
{
	for (size_t i = 0; i < threads; ++i)
		workers.emplace_back(
			[this, i]
	{


		SetThreadName(std::string("thread_pool: ") + std::to_string(i));

		for (;;)
		{
			TaskInfo task;
			bool need_restart = false;
			auto started = false;
			{
				std::unique_lock<std::mutex> lock(this->queue_mutex);
				this->condition.wait(lock,
					[this, &started, &task]
				{
					if (!started)
					{
						free_count++;
						condition_free.notify_all();
						started = true;
					}
					return this->stop || !this->tasks.empty();
				});

				if (this->stop && this->tasks.empty())
					return;

				if (i < 8 && this->tasks.top().priority != TaskPriority::HIGH) // buggy, but useful kostil'
					need_restart = true;
				else
				{
					free_count--;
					task = std::move(this->tasks.top());
					this->tasks.pop();
				}

				//   if (task.priority == TaskPriority::HIGH)
				//   high_tasks++;
			}

			SPECTRUM_TRY
				if (need_restart)
				{
					this->condition.notify_one();
					continue;
				}

				task.func();
				//     if (task.priority == TaskPriority::HIGH)
				//      high_tasks--;
			SPECTRUM_CATCH
			//	if (stop) return;
		}
	}
	);
}

// add new work item to the pool
template<class F/*, class... Args*/>
auto thread_pool::enqueue(F&& f, TaskPriority priority/*, Args&& ... args*/)
-> std::future<typename std::result_of<F(/*Args...*/)>::type>
{
	using return_type = typename std::result_of<F(/*Args...*/)>::type;

	auto task = std::make_shared< std::packaged_task<return_type()> >(
		std::bind(std::forward<F>(f)/*, std::forward<Args>(args)...*/)
		);

	std::future<return_type> res = task->get_future();
	{
		std::unique_lock<std::mutex> lock(queue_mutex);

		// don't allow enqueueing after stopping the pool
		if (!stop)
			//   throw std::runtime_error("enqueue on stopped ThreadPool");

			tasks.emplace([task]() { (*task)(); }, priority);
	}
	condition.notify_one();
	return res;
}

// the destructor joins all threads
inline thread_pool::~thread_pool()
{
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		stop = true;
	}
	condition.notify_all();

	for (auto& worker : workers)
		worker.join();
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

