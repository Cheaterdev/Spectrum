export module Test.Events;

export import Test.Framework;

import Core;

using namespace Events;

export namespace Test
{
	// Event registration and firing tests
	TEST(Core.Events, SimpleEventRegistration)
	{
		Event<int> event;
		std::atomic<int> called(0);

		event.register_handler(nullptr, [&called](int value) {
			called.store(value, std::memory_order_relaxed);
		});

		event(42);
		ASSERT_EQ(called.load(std::memory_order_relaxed), 42);
	}

	TEST(Core.Events, EventFiring)
	{
		Event<int> event;
		std::atomic<int> result(0);

		event.register_handler(nullptr, [&result](int val) {
			result.store(val * 2, std::memory_order_relaxed);
		});

		event(21);
		ASSERT_EQ(result.load(std::memory_order_relaxed), 42);
	}

	TEST(Core.Events, MultipleHandlerRegistration)
	{
		Event<int> event;
		std::atomic<int> count(0);

		event.register_handler(nullptr, [&count](int) {
			count.fetch_add(1, std::memory_order_relaxed);
		});
		event.register_handler(nullptr, [&count](int) {
			count.fetch_add(1, std::memory_order_relaxed);
		});
		event.register_handler(nullptr, [&count](int) {
			count.fetch_add(1, std::memory_order_relaxed);
		});

		event(0);
		ASSERT_EQ(count.load(std::memory_order_relaxed), 3);
	}

	TEST(Core.Events, EventWithMultipleArguments)
	{
		Event<int, float, std::string> event;
		std::atomic<int> int_val(0);
		std::atomic<int> float_match(0);
		std::string captured_string;
		std::mutex str_mutex;

		event.register_handler(nullptr, [&int_val, &float_match, &captured_string, &str_mutex](int i, float f, const std::string& s) {
			int_val.store(i, std::memory_order_relaxed);
			if (std::abs(f - 3.14f) < 0.01f)
				float_match.store(1, std::memory_order_relaxed);
			{
				std::lock_guard<std::mutex> lock(str_mutex);
				captured_string = s;
			}
		});

		event(42, 3.14f, "test");

		ASSERT_EQ(int_val.load(std::memory_order_relaxed), 42);
		ASSERT_EQ(float_match.load(std::memory_order_relaxed), 1);
		{
			std::lock_guard<std::mutex> lock(str_mutex);
			ASSERT_EQ(captured_string, std::string("test"));
		}
	}

	TEST(Core.Events, EventWithVectorArgument)
	{
		Event<const std::vector<int>&> event;
		std::atomic<int> size(0);

		event.register_handler(nullptr, [&size](const std::vector<int>& vec) {
			size.store(static_cast<int>(vec.size()), std::memory_order_relaxed);
		});

		std::vector<int> test_vec = {1, 2, 3, 4, 5};
		event(test_vec);
		ASSERT_EQ(size.load(std::memory_order_relaxed), 5);
	}

	TEST(Core.Events, EventWithCustomType)
	{
		struct Point { int x; int y; };
		Event<Point> event;
		std::atomic<int> sum(0);

		event.register_handler(nullptr, [&sum](Point p) {
			sum.store(p.x + p.y, std::memory_order_relaxed);
		});

		Point p{10, 32};
		event(p);
		ASSERT_EQ(sum.load(std::memory_order_relaxed), 42);
	}

	// Handler callbacks and invocation tests
	TEST(Core.Events, HandlerCallbackInvocation)
	{
		Event<int> event;
		std::atomic<int> invocation_count(0);

		event.register_handler(nullptr, [&invocation_count](int val) {
			invocation_count.fetch_add(val, std::memory_order_relaxed);
		});

		event(5);
		event(10);
		event(27);

		ASSERT_EQ(invocation_count.load(std::memory_order_relaxed), 42);
	}

	TEST(Core.Events, HandlerUnregistration)
	{
		Event<int> event;
		std::atomic<int> called(0);

		auto handler = event.register_handler(nullptr, [&called](int) {
			called.fetch_add(1, std::memory_order_relaxed);
		});

		event(0);
		ASSERT_EQ(called.load(std::memory_order_relaxed), 1);

		handler->unregister();

		event(0);
		ASSERT_EQ(called.load(std::memory_order_relaxed), 1);
	}

	TEST(Core.Events, SequentialHandlerInvocation)
	{
		Event<> event;
		std::vector<int> execution_order;
		std::mutex order_mutex;

		event.register_handler(nullptr, [&execution_order, &order_mutex]() {
			std::lock_guard<std::mutex> lock(order_mutex);
			execution_order.push_back(1);
		});

		event.register_handler(nullptr, [&execution_order, &order_mutex]() {
			std::lock_guard<std::mutex> lock(order_mutex);
			execution_order.push_back(2);
		});

		event.register_handler(nullptr, [&execution_order, &order_mutex]() {
			std::lock_guard<std::mutex> lock(order_mutex);
			execution_order.push_back(3);
		});

		event();

		{
			std::lock_guard<std::mutex> lock(order_mutex);
			ASSERT_EQ(execution_order.size(), static_cast<size_t>(3));
		}
	}

	TEST(Core.Events, HandlerWithCapture)
	{
		Event<int> event;
		int multiplier = 5;
		std::atomic<int> result(0);

		event.register_handler(nullptr, [&result, multiplier](int val) {
			result.store(val * multiplier, std::memory_order_relaxed);
		});

		event(8);
		ASSERT_EQ(result.load(std::memory_order_relaxed), 40);
	}

	TEST(Core.Events, VoidEvent)
	{
		Event<> event;
		std::atomic<int> called(0);

		event.register_handler(nullptr, [&called]() {
			called.store(1, std::memory_order_relaxed);
		});

		event();
		ASSERT_EQ(called.load(std::memory_order_relaxed), 1);
	}

	// Event propagation tests
	TEST(Core.Events, EventPropagationToMultipleHandlers)
	{
		Event<int> event;
		std::atomic<int> handler1_val(0);
		std::atomic<int> handler2_val(0);
		std::atomic<int> handler3_val(0);

		event.register_handler(nullptr, [&handler1_val](int val) {
			handler1_val.store(val, std::memory_order_relaxed);
		});

		event.register_handler(nullptr, [&handler2_val](int val) {
			handler2_val.store(val * 2, std::memory_order_relaxed);
		});

		event.register_handler(nullptr, [&handler3_val](int val) {
			handler3_val.store(val * 3, std::memory_order_relaxed);
		});

		event(7);

		ASSERT_EQ(handler1_val.load(std::memory_order_relaxed), 7);
		ASSERT_EQ(handler2_val.load(std::memory_order_relaxed), 14);
		ASSERT_EQ(handler3_val.load(std::memory_order_relaxed), 21);
	}

	TEST(Core.Events, EventPropagationWithDifferentTypes)
	{
		Event<int, int, int> event;
		std::atomic<int> sum(0);

		event.register_handler(nullptr, [&sum](int a, int b, int c) {
			sum.store(a + b + c, std::memory_order_relaxed);
		});

		event(10, 20, 12);
		ASSERT_EQ(sum.load(std::memory_order_relaxed), 42);
	}

	TEST(Core.Events, EventChaining)
	{
		Event<int> event1;
		Event<int> event2;
		std::atomic<int> final_result(0);

		event1.register_handler(nullptr, [&event2](int val) {
			event2(val * 2);
		});

		event2.register_handler(nullptr, [&final_result](int val) {
			final_result.store(val, std::memory_order_relaxed);
		});

		event1(21);
		ASSERT_EQ(final_result.load(std::memory_order_relaxed), 42);
	}

	TEST(Core.Events, ThreadSafeEventPropagation)
	{
		Event<int> event;
		std::atomic<int> counter(0);

		for (int i = 0; i < 5; ++i)
		{
			event.register_handler(nullptr, [&counter](int) {
				counter.fetch_add(1, std::memory_order_relaxed);
			});
		}

		event(0);
		ASSERT_EQ(counter.load(std::memory_order_relaxed), 5);
	}

	TEST(Core.Events, ComplexEventFlow)
	{
		Event<std::string> event;
		std::vector<std::string> log;
		std::mutex log_mutex;

		event.register_handler(nullptr, [&log, &log_mutex](const std::string& msg) {
			std::lock_guard<std::mutex> lock(log_mutex);
			log.push_back("Handler1:" + msg);
		});

		event.register_handler(nullptr, [&log, &log_mutex](const std::string& msg) {
			std::lock_guard<std::mutex> lock(log_mutex);
			log.push_back("Handler2:" + msg);
		});

		event("Event1");
		event("Event2");

		{
			std::lock_guard<std::mutex> lock(log_mutex);
			ASSERT_EQ(log.size(), static_cast<size_t>(4));
		}
	}

	TEST(Core.Events, EventPropagationIntegrity)
	{
		Event<int, int> event;
		std::atomic<int> checksum(0);

		event.register_handler(nullptr, [&checksum](int a, int b) {
			checksum.store(a * 10 + b, std::memory_order_relaxed);
		});

		for (int i = 1; i <= 4; ++i)
		{
			for (int j = 1; j <= 4; ++j)
			{
				event(i, j);
				int expected = i * 10 + j;
				ASSERT_EQ(checksum.load(std::memory_order_relaxed), expected);
			}
		}
	}

	// Event with Runner tests
	TEST(Core.Events, EventWithRunner)
	{
		Runner runner;
		Event<int> event;
		event.runner = &runner;

		std::atomic<int> result(0);

		event.register_handler(&runner, [&result](int val) {
			result.store(val * 2, std::memory_order_relaxed);
		});

		event(21);
		runner.process_tasks();

		ASSERT_EQ(result.load(std::memory_order_relaxed), 42);
	}

	TEST(Core.Events, EventWithRunnerMultipleHandlers)
	{
		Runner runner;
		Event<int> event;
		event.runner = &runner;

		std::atomic<int> count(0);

		event.register_handler(&runner, [&count](int) {
			count.fetch_add(1, std::memory_order_relaxed);
		});

		event.register_handler(&runner, [&count](int) {
			count.fetch_add(2, std::memory_order_relaxed);
		});

		event.register_handler(&runner, [&count](int) {
			count.fetch_add(3, std::memory_order_relaxed);
		});

		event(0);
		runner.process_tasks();

		ASSERT_EQ(count.load(std::memory_order_relaxed), 6);
	}

	TEST(Core.Events, EventWithRunnerAccumulation)
	{
		Runner runner;
		Event<int> event;
		event.runner = &runner;

		std::atomic<int> sum(0);

		event.register_handler(&runner, [&sum](int val) {
			sum.fetch_add(val, std::memory_order_relaxed);
		});

		event(10);
		event(20);
		event(12);

		runner.process_tasks();

		ASSERT_EQ(sum.load(std::memory_order_relaxed), 42);
	}

	TEST(Core.Events, EventWithRunnerMultipleProcessing)
	{
		Runner runner;
		Event<int> event;
		event.runner = &runner;

		std::atomic<int> result(0);

		event.register_handler(&runner, [&result](int val) {
			result.store(val * 2, std::memory_order_relaxed);
		});

		event(5);
		runner.process_tasks();
		ASSERT_EQ(result.load(std::memory_order_relaxed), 10);

		event(15);
		runner.process_tasks();
		ASSERT_EQ(result.load(std::memory_order_relaxed), 30);

		event(6);
		runner.process_tasks();
		ASSERT_EQ(result.load(std::memory_order_relaxed), 12);
	}

	// Multi-threaded event tests using thread pool
	TEST(Core.Events, EventFiringFromMultipleThreads)
	{
		Event<int> event;
		std::atomic<int> counter(0);

		event.register_handler(nullptr, [&counter](int) {
			counter.fetch_add(1, std::memory_order_relaxed);
		});

		std::vector<std::future<void>> futures;

		for (int i = 0; i < 10; ++i)
		{
			futures.push_back(Core::thread_pool::get().enqueue([&event]() {
				event(0);
			}));
		}

		for (auto& f : futures)
		{
			f.get();
		}

		ASSERT_EQ(counter.load(std::memory_order_relaxed), 10);
	}

	TEST(Core.Events, MultipleHandlersFromMultipleThreads)
	{
		Event<int> event;
		std::atomic<int> counter(0);

		event.register_handler(nullptr, [&counter](int) {
			counter.fetch_add(1, std::memory_order_relaxed);
		});

		event.register_handler(nullptr, [&counter](int) {
			counter.fetch_add(1, std::memory_order_relaxed);
		});

		event.register_handler(nullptr, [&counter](int) {
			counter.fetch_add(1, std::memory_order_relaxed);
		});

		std::vector<std::future<void>> futures;

		for (int i = 0; i < 5; ++i)
		{
			futures.push_back(Core::thread_pool::get().enqueue([&event]() {
				event(0);
			}));
		}

		for (auto& f : futures)
		{
			f.get();
		}

		ASSERT_EQ(counter.load(std::memory_order_relaxed), 15);
	}

	TEST(Core.Events, EventWithDataFromThreadPool)
	{
		Event<int> event;
		std::atomic<int> sum(0);

		event.register_handler(nullptr, [&sum](int val) {
			sum.fetch_add(val, std::memory_order_relaxed);
		});

		std::vector<std::future<void>> futures;

		for (int i = 1; i <= 10; ++i)
		{
			futures.push_back(Core::thread_pool::get().enqueue([&event, i]() {
				event(i);
			}));
		}

		for (auto& f : futures)
		{
			f.get();
		}

		ASSERT_EQ(sum.load(std::memory_order_relaxed), 55);
	}

	TEST(Core.Events, ConcurrentEventFiringAndHandling)
	{
		Event<int> event;
		std::atomic<int> processed(0);

		event.register_handler(nullptr, [&processed](int val) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			processed.fetch_add(val, std::memory_order_relaxed);
		});

		std::vector<std::future<void>> futures;

		for (int i = 0; i < 20; ++i)
		{
			futures.push_back(Core::thread_pool::get().enqueue([&event, i]() {
				event(1);
			}));
		}

		for (auto& f : futures)
		{
			f.get();
		}

		ASSERT_EQ(processed.load(std::memory_order_relaxed), 20);
	}

	TEST(Core.Events, ThreadPoolEventChaining)
	{
		Event<int> event1;
		Event<int> event2;
		std::atomic<int> final_result(0);

		event1.register_handler(nullptr, [&event2](int val) {
			event2(val * 2);
		});

		event2.register_handler(nullptr, [&final_result](int val) {
			final_result.store(val, std::memory_order_relaxed);
		});

		std::vector<std::future<void>> futures;

		for (int i = 0; i < 5; ++i)
		{
			futures.push_back(Core::thread_pool::get().enqueue([&event1]() {
				event1(21);
			}));
		}

		for (auto& f : futures)
		{
			f.get();
		}

		ASSERT_EQ(final_result.load(std::memory_order_relaxed), 42);
	}

	TEST(Core.Events, RunnerWithThreadPool)
	{
		Runner runner;
		Event<int> event;
		event.runner = &runner;

		std::atomic<int> processed(0);

		event.register_handler(&runner, [&processed](int val) {
			processed.fetch_add(val, std::memory_order_relaxed);
		});

		std::vector<std::future<void>> futures;

		for (int i = 1; i <= 5; ++i)
		{
			futures.push_back(Core::thread_pool::get().enqueue([&event, i]() {
				event(i);
			}));
		}

		for (auto& f : futures)
		{
			f.get();
		}

		runner.process_tasks();

		ASSERT_EQ(processed.load(std::memory_order_relaxed), 15);
	}

	TEST(Core.Events, ComplexAsyncEventFlow)
	{
		Runner runner;
		Event<int> producer_event;
		Event<int> worker_event;

		producer_event.runner = &runner;
		worker_event.runner = &runner;

		std::atomic<int> result(0);

		producer_event.register_handler(&runner, [&worker_event](int val) {
			worker_event(val * 2);
		});

		worker_event.register_handler(&runner, [&result](int val) {
			result.store(val + 10, std::memory_order_relaxed);
		});

		std::vector<std::future<void>> futures;

		for (int i = 0; i < 5; ++i)
		{
			futures.push_back(Core::thread_pool::get().enqueue([&producer_event]() {
				producer_event(16);
			}));
		}

		for (auto& f : futures)
		{
			f.get();
		}

		runner.process_tasks();

		ASSERT_EQ(result.load(std::memory_order_relaxed), 42);
	}
}
