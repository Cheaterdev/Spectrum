module Core:Scheduler;

import :Threading;
import :Profiling;
import stl.core;
import stl.threading;

using namespace std;

struct thread_pool::Impl
{
	static constexpr int LaneCount = static_cast<int>(TaskLane::Count);
	static constexpr int PriorityCount = 3;

	struct Item
	{
		std::function<void()> function;
		const wchar_t* name = nullptr;
	};

	struct Worker
	{
		std::thread thread;
		std::condition_variable wake;
		uint32_t lanes = 0;   // bit per TaskLane this worker serves
		bool idle = false;    // waiting, and nobody has signalled it yet
	};

	std::mutex m;
	std::array<std::array<std::deque<Item>, PriorityCount>, LaneCount> queues;
	std::vector<std::unique_ptr<Worker>> workers;
	bool stopping = false;

	static uint32_t bit(TaskLane lane) { return 1u << static_cast<int>(lane); }

	Impl()
	{
		const size_t count = std::max<size_t>(2, std::thread::hardware_concurrency());

		// The first workers serve General only: the lanes below can never take them, so frame work always has a worker.
		const size_t reserved = std::max<size_t>(1, count / 8);
		const size_t open = count - reserved;
		const size_t asset_workers = std::max<size_t>(1, open / 2);
		const size_t background_workers = std::max<size_t>(1, count / 8);

		for (size_t i = 0; i < count; i++)
		{
			auto worker = std::make_unique<Worker>();
			worker->lanes = bit(TaskLane::General);

			if (i >= reserved)
			{
				const size_t j = i - reserved;
				if (j < asset_workers)
					worker->lanes |= bit(TaskLane::Asset);
				if (j >= open - std::min(open, background_workers))
					worker->lanes |= bit(TaskLane::Background);
			}
			workers.push_back(std::move(worker));
		}

		// Started after every worker exists: they look at the whole list.
		for (size_t i = 0; i < workers.size(); i++)
			workers[i]->thread = std::thread([this, i] { run(*workers[i], i); });
	}

	~Impl()
	{
		{
			std::lock_guard<std::mutex> g(m);
			stopping = true;
		}
		for (auto& w : workers)
			w->wake.notify_all();
		for (auto& w : workers)
			if (w->thread.joinable())
				w->thread.join();
	}

	void submit(std::function<void()> function, TaskOptions options)
	{
		const int lane = static_cast<int>(options.lane);
		const int priority = std::clamp(static_cast<int>(options.priority), 0, PriorityCount - 1);

		std::lock_guard<std::mutex> g(m);
		queues[lane][priority].push_back({ std::move(function), options.name });

		// One idle worker that serves the lane. If there is none, a busy one takes it when it is done.
		for (auto& w : workers)
			if (w->idle && (w->lanes & bit(options.lane)))
			{
				w->idle = false;
				w->wake.notify_one();
				break;
			}
	}

	bool has_work(const Worker& w) const
	{
		for (int lane = 0; lane < LaneCount; lane++)
			if (w.lanes & (1u << lane))
				for (int priority = 0; priority < PriorityCount; priority++)
					if (!queues[lane][priority].empty())
						return true;
		return false;
	}

	// Highest priority first over the lanes the worker serves, FIFO within a queue.
	bool take(const Worker& w, Item& out)
	{
		for (int priority = PriorityCount - 1; priority >= 0; priority--)
			for (int lane = 0; lane < LaneCount; lane++)
				if ((w.lanes & (1u << lane)) && !queues[lane][priority].empty())
				{
					out = std::move(queues[lane][priority].front());
					queues[lane][priority].pop_front();
					return true;
				}
		return false;
	}

	void run(Worker& w, size_t index)
	{
		SetThreadName("Pool worker " + std::to_string(index));

		std::unique_lock<std::mutex> lock(m);
		for (;;)
		{
			Item item;
			if (take(w, item))
			{
				lock.unlock();
				{
					LiteralWStr label;
					label.ptr = item.name;
					Timer scope = item.name ? Profiler::get().start(label) : Timer();
					item.function();
				}
				item = {};   // the task's captures go before the lock is taken again
				lock.lock();
				continue;
			}

			if (stopping)
				return;

			// Plain wait, then back to the top: a wake-up whose task another worker took first must leave this worker
			// flagged idle again, or no later submit would ever signal it (it would be lost from the pool for good).
			w.idle = true;
			w.wake.wait(lock);
		}
	}

	std::string describe()
	{
		static const char* lane_names[] = { "general", "asset", "background" };
		static const char* priority_names[] = { "low", "normal", "high" };

		std::lock_guard<std::mutex> g(m);
		std::string out = "thread pool: " + std::to_string(workers.size()) + " workers;";
		for (int lane = 0; lane < LaneCount; lane++)
		{
			size_t served = 0;
			for (auto& w : workers)
				served += (w->lanes >> lane) & 1u;

			out += std::string(" ") + lane_names[lane] + " " + std::to_string(served) + " workers, queued";
			for (int priority = PriorityCount - 1; priority >= 0; priority--)
				out += std::string(" ") + priority_names[priority] + " " + std::to_string(queues[lane][priority].size());
			out += ";";
		}
		return out;
	}
};

thread_pool::thread_pool() : pimpl(std::make_unique<Impl>()), stop(false) {}

thread_pool::~thread_pool()
{
	stop = true;
}

void thread_pool::submit_impl(std::function<void()> f, TaskOptions options)
{
	pimpl->submit(std::move(f), options);
}

std::string thread_pool::describe()
{
	return pimpl->describe();
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
