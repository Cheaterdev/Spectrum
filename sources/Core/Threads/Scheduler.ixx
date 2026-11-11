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

// A lane is a class of work with its own queues, served by a subset of the workers (the worker's lane mask).
// A lane that is only served by some of the workers can never use more of them than that, whatever is queued,
// so a flood of one kind of task (hundreds of asset loads) leaves the rest of the pool to the other kinds.
//   General     everything by default: every worker serves it
//   Asset       asset loading and its continuations: about half of the unreserved workers
//   Background  long, low-value work (previews, saves): a few workers
// The first workers serve General only ("reserved"): background lanes never get them, so frame work always has some.
enum class TaskLane : int
{
	General,
	Asset,
	Background,
	Count
};

struct TaskOptions
{
	TaskPriority priority = TaskPriority::NORMAL;   // higher first, FIFO within a priority
	TaskLane lane = TaskLane::General;
	const wchar_t* name = nullptr;   // a literal: the task is a PROFILE scope of this name, so captures show it
};

// Do not block a worker on the result of a task of its own lane: when every worker of the lane does it, nothing
// is left to run what they wait for. Chain continuations instead.
class thread_pool : public Singleton<thread_pool>
{
	friend class Singleton<thread_pool>;

public:
	template<class F>
	auto enqueue(F&& f) -> std::future<typename std::invoke_result<F>::type>
	{
		return enqueue(TaskOptions{}, std::forward<F>(f));
	}

	template<class F>
	auto enqueue(TaskOptions options, F&& f) -> std::future<typename std::invoke_result<F>::type>
	{
		if (stop) throw std::runtime_error("wtf");
		using return_type = typename std::invoke_result<F>::type;
		auto task = std::make_shared<std::packaged_task<return_type()>>(std::forward<F>(f));
		auto fut = task->get_future();
		submit_impl([task]() { (*task)(); }, options);
		return std::move(fut);
	}

	// Fire-and-forget submit for void tasks that don't need a future — skips the
	// packaged_task/promise allocation. Caller is responsible for its own completion signal.
	template<class F>
	void enqueue_detached(F&& f)
	{
		enqueue_detached(TaskOptions{}, std::forward<F>(f));
	}

	template<class F>
	void enqueue_detached(TaskOptions options, F&& f)
	{
		if (stop) throw std::runtime_error("wtf");
		submit_impl(std::forward<F>(f), options);
	}

	// Workers, the lanes each serves, and what is queued per lane and priority now: for the log.
	std::string describe();

private:
	struct Impl;
	std::unique_ptr<Impl> pimpl;
	void submit_impl(std::function<void()> f, TaskOptions options);
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
	scheduler();
	virtual ~scheduler();
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
