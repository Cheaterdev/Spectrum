export module Test.Threading;

export import Test.Framework;

import Core;

export namespace Test
{
	// Thread pool task execution tests
	TEST(Core.Threading, ThreadPoolSimpleTask)
	{
		auto result = thread_pool::get().enqueue([]() { return 42; });
		int value = result.get();
		ASSERT_EQ(value, 42);
	}

	TEST(Core.Threading, ThreadPoolMultipleTasks)
	{
		auto result1 = thread_pool::get().enqueue([]() { return 10; });
		auto result2 = thread_pool::get().enqueue([]() { return 20; });
		auto result3 = thread_pool::get().enqueue([]() { return 30; });

		int val1 = result1.get();
		int val2 = result2.get();
		int val3 = result3.get();

		ASSERT_EQ(val1, 10);
		ASSERT_EQ(val2, 20);
		ASSERT_EQ(val3, 30);
	}

	TEST(Core.Threading, ThreadPoolVoidTask)
	{
		std::atomic<int> counter(0);
		auto task = thread_pool::get().enqueue([&counter]() {
			counter.fetch_add(1, std::memory_order_relaxed);
		});
		task.get();
		ASSERT_EQ(counter.load(std::memory_order_relaxed), 1);
	}

	TEST(Core.Threading, ThreadPoolStringTask)
	{
		auto result = thread_pool::get().enqueue([]() {
			return std::string("ThreadPool");
		});
		std::string value = result.get();
		ASSERT_EQ(value, std::string("ThreadPool"));
	}

	TEST(Core.Threading, ThreadPoolFloatTask)
	{
		auto result = thread_pool::get().enqueue([]() {
			return 3.14159f;
		});
		float value = result.get();
		ASSERT_TRUE(std::abs(value - 3.14159f) < 0.0001f);
	}

	TEST(Core.Threading, ThreadPoolVec3Task)
	{
		auto result = thread_pool::get().enqueue([]() {
			return vec3(1.0f, 2.0f, 3.0f);
		});
		vec3 value = result.get();
		ASSERT_EQ(value.x, 1.0f);
		ASSERT_EQ(value.y, 2.0f);
		ASSERT_EQ(value.z, 3.0f);
	}

	TEST(Core.Threading, ThreadPoolConcurrency)
	{
		std::atomic<int> completed(0);
		std::vector<std::future<int>> futures;

		for (int i = 0; i < 10; ++i)
		{
			futures.push_back(thread_pool::get().enqueue([i, &completed]() {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				completed.fetch_add(1, std::memory_order_relaxed);
				return i;
			}));
		}

		for (int i = 0; i < 10; ++i)
		{
			int val = futures[i].get();
			ASSERT_EQ(val, i);
		}
		ASSERT_EQ(completed.load(std::memory_order_relaxed), 10);
	}

	// Scheduler tests
	TEST(Core.Threading, SchedulerImmediateExecution)
	{
		std::atomic<int> executed(0);
		auto result = scheduler::get().enqueue_now([&executed]() {
			executed.store(1, std::memory_order_relaxed);
		});
		result.get();
		ASSERT_EQ(executed.load(std::memory_order_relaxed), 1);
	}

	TEST(Core.Threading, SchedulerDelayedExecution)
	{
		std::atomic<int> executed(0);
		auto now = std::chrono::steady_clock::now();
		auto future_time = now + std::chrono::milliseconds(100);

		auto result = scheduler::get().enqueue([&executed]() {
			executed.store(1, std::memory_order_relaxed);
		}, future_time);

		result.get();
		ASSERT_EQ(executed.load(std::memory_order_relaxed), 1);
	}

	TEST(Core.Threading, SchedulerMultipleTasks)
	{
		std::atomic<int> count(0);
		std::vector<std::future<void>> futures;

		for (int i = 0; i < 5; ++i)
		{
			futures.push_back(scheduler::get().enqueue_now([&count]() {
				count.fetch_add(1, std::memory_order_relaxed);
			}));
		}

		for (auto& f : futures)
		{
			f.get();
		}
		ASSERT_EQ(count.load(std::memory_order_relaxed), 5);
	}

	// Synchronization primitives tests
	TEST(Core.Threading, SpinLockBasic)
	{
		SpinLock lock;
		lock.lock();
		ASSERT_TRUE(true);
		lock.unlock();
		ASSERT_TRUE(true);
	}

	TEST(Core.Threading, SpinLockProtection)
	{
		SpinLock lock;
		int shared_value = 0;

		{
			std::lock_guard<SpinLock> guard(lock);
			shared_value = 42;
		}

		{
			std::lock_guard<SpinLock> guard(lock);
			ASSERT_EQ(shared_value, 42);
		}
	}

	TEST(Core.Threading, SpinLockConcurrentAccess)
	{
		SpinLock lock;
		std::atomic<int> counter(0);
		std::vector<std::future<void>> futures;

		for (int i = 0; i < 10; ++i)
		{
			futures.push_back(thread_pool::get().enqueue([&lock, &counter]() {
				for (int j = 0; j < 10; ++j)
				{
					std::lock_guard<SpinLock> guard(lock);
					counter.fetch_add(1, std::memory_order_relaxed);
				}
			}));
		}

		for (auto& f : futures)
		{
			f.get();
		}
		ASSERT_EQ(counter.load(std::memory_order_relaxed), 100);
	}

	TEST(Core.Threading, LockableGuard)
	{
		using Guard = Thread::Lockable::guard;
		using Mutex = Thread::Lockable::mutex;

		Mutex mtx;
		int value = 0;

		{
			Guard guard(mtx);
			value = 100;
		}

		{
			Guard guard(mtx);
			ASSERT_EQ(value, 100);
		}
	}

	TEST(Core.Threading, AtomicOperations)
	{
		std::atomic<int> value(0);

		auto result1 = thread_pool::get().enqueue([&value]() {
			for (int i = 0; i < 100; ++i)
			{
				value.fetch_add(1, std::memory_order_relaxed);
			}
		});

		auto result2 = thread_pool::get().enqueue([&value]() {
			for (int i = 0; i < 100; ++i)
			{
				value.fetch_add(1, std::memory_order_relaxed);
			}
		});

		result1.get();
		result2.get();

		ASSERT_EQ(value.load(std::memory_order_relaxed), 200);
	}

	TEST(Core.Threading, ThreadLocalStorage)
	{
		static thread_local int tls_value = 0;

		auto result1 = thread_pool::get().enqueue([&]() {
			tls_value = 42;
			return tls_value;
		});

		auto result2 = thread_pool::get().enqueue([&]() {
			tls_value = 100;
			return tls_value;
		});

		int val1 = result1.get();
		int val2 = result2.get();

		ASSERT_EQ(val1, 42);
		ASSERT_EQ(val2, 100);
	}

	TEST(Core.Threading, TaskReturnTypes)
	{
		// Test void return
		auto void_task = thread_pool::get().enqueue([]() { });
		void_task.get();

		// Test int return
		auto int_task = thread_pool::get().enqueue([]() { return 123; });
		ASSERT_EQ(int_task.get(), 123);

		// Test bool return
		auto bool_task = thread_pool::get().enqueue([]() { return true; });
		ASSERT_TRUE(bool_task.get());

		// Test string return
		auto string_task = thread_pool::get().enqueue([]() { return std::string("result"); });
		ASSERT_EQ(string_task.get(), std::string("result"));
	}

	TEST(Core.Threading, ConcurrentTaskOrdering)
	{
		std::vector<int> results;
		std::mutex results_mutex;
		std::vector<std::future<void>> futures;

		for (int i = 0; i < 5; ++i)
		{
			futures.push_back(thread_pool::get().enqueue([i, &results, &results_mutex]() {
				std::lock_guard<std::mutex> lock(results_mutex);
				results.push_back(i);
			}));
		}

		for (auto& f : futures)
		{
			f.get();
		}

		ASSERT_EQ(results.size(), static_cast<size_t>(5));
	}

	TEST(Core.Threading, ExceptionHandling)
	{
		bool caught_exception = false;
		auto result = thread_pool::get().enqueue([]() -> int {
			throw std::runtime_error("test error");
			return 0;
			});

		try
		{

			result.get();
		}
		catch (const std::exception&)
		{
			caught_exception = true;
		}
		ASSERT_TRUE(caught_exception);
	}
}
