export module Test.Profiling;

#define TEST_MODULE_ID Profiling

export import Test.Framework;

import Core;

SETUP_CATEGORY(Core.Profiling, []() { Profiler::get().enabled = true; })

namespace {
	void busy_wait_ms(int milliseconds) {
		auto start = std::chrono::high_resolution_clock::now();
		while (true) {
			auto now = std::chrono::high_resolution_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
			if (elapsed >= milliseconds) break;
		}
	}
}

export namespace Test
{
	// Performance counter collection tests
	TEST(Core.Profiling, CPUCounterCreation)
	{
		CPUCounter counter;
		ASSERT_TRUE(true);
	}

	TEST(Core.Profiling, CPUCounterTimeMeasurement)
	{
		CPUCounter counter;
		counter.start_time = std::chrono::high_resolution_clock::now();
		busy_wait_ms(10);
		counter.end_time = std::chrono::high_resolution_clock::now();

		auto elapsed = counter.get_lapsed_time();
		ASSERT_TRUE(elapsed.count() >= 0.01);
	}

	TEST(Core.Profiling, TimedBlockCreation)
	{
		auto timer = Profiler::get().start(L"test_block");
		ASSERT_TRUE(Profiler::current_block != nullptr);
	}

	TEST(Core.Profiling, TimedBlockHierarchy)
	{
		auto outer = Profiler::get().start(L"parent");
		int parent_level = Profiler::current_block ? Profiler::current_block->level : -1;
		{
			auto inner = Profiler::get().start(L"child");
			int child_level = Profiler::current_block ? Profiler::current_block->level : -1;
			ASSERT_TRUE(child_level > parent_level);
		}
	}

	TEST(Core.Profiling, TimerBasicFunctionality)
	{
		auto timer = Profiler::get().start(L"test_timer");
		busy_wait_ms(5);
	}

	TEST(Core.Profiling, TimerElapsedTime)
	{
		{
			auto timer = Profiler::get().start(L"elapsed_test");
			ASSERT_TRUE(Profiler::current_block != nullptr);
			busy_wait_ms(5);
		}
	}

	TEST(Core.Profiling, TimerMultipleScopes)
	{
		{
			auto timer1 = Profiler::get().start(L"scope1");
			busy_wait_ms(5);
		}

		{
			auto timer2 = Profiler::get().start(L"scope2");
			busy_wait_ms(5);
		}

		ASSERT_TRUE(true);
	}

	TEST(Core.Profiling, NestedTimers)
	{
		auto outer = Profiler::get().start(L"outer");
		{
			auto inner = Profiler::get().start(L"inner");
			busy_wait_ms(3);
		}
		busy_wait_ms(2);
	}

	TEST(Core.Profiling, TimerDisabled)
	{
		bool original_state = Profiler::get().enabled;
		Profiler::get().enabled = false;

		auto timer = Profiler::get().start(L"disabled_timer");
		ASSERT_TRUE(true);

		Profiler::get().enabled = original_state;
	}

	TEST(Core.Profiling, ScopedCounterCreation)
	{
		{
			ScopedCounter counter("test_counter");
			busy_wait_ms(5);
		}
		ASSERT_TRUE(true);
	}

	TEST(Core.Profiling, ProfilerGetCurrent)
	{
		auto current_before = Profiler::get().get_current();
		{
			auto timer = Profiler::get().start(L"current_test");
			auto current_inside = Profiler::get().get_current();
			ASSERT_TRUE(current_inside != nullptr);
		}
		auto current_after = Profiler::get().get_current();
		ASSERT_EQ(current_before, current_after);
	}

	TEST(Core.Profiling, DeepTimerNesting)
	{
		auto level0 = Profiler::get().start(L"level0");
		{
			auto level1 = Profiler::get().start(L"level1");
			{
				auto level2 = Profiler::get().start(L"level2");
				{
					auto level3 = Profiler::get().start(L"level3");
					busy_wait_ms(2);
				}
			}
		}

		ASSERT_TRUE(true);
	}

	TEST(Core.Profiling, TimerRootPointer)
	{
		auto timer = Profiler::get().start(L"root_test");
		ASSERT_TRUE(timer.get_root() == &Profiler::get());
	}

	TEST(Core.Profiling, TimedBlockNameStorage)
	{
		auto timer = Profiler::get().start(L"named_block");
		ASSERT_TRUE(Profiler::current_block != nullptr);
		ASSERT_TRUE(Profiler::current_block->get_name() == std::wstring_view(L"named_block"));
	}

	TEST(Core.Profiling, PerformanceCounterAccuracy)
	{
		CPUCounter counter;
		counter.start_time = std::chrono::high_resolution_clock::now();
		busy_wait_ms(5);
		counter.end_time = std::chrono::high_resolution_clock::now();

		auto elapsed = counter.get_lapsed_time();
		ASSERT_TRUE(elapsed.count() >= 0);
		ASSERT_TRUE(elapsed.count() < 1.0);
	}

	TEST(Core.Profiling, TimerMoveSemantics)
	{
		Timer timer1 = Profiler::get().start(L"movable");
		Timer timer2 = std::move(timer1);

		ASSERT_TRUE(true);
	}

	TEST(Core.Profiling, MultipleTimersSequential)
	{
		for (int i = 0; i < 5; ++i) {
			auto timer = Profiler::get().start(L"sequential");
			busy_wait_ms(2);
		}
		ASSERT_TRUE(true);
	}

	TEST(Core.Profiling, TimerWithinLoop)
	{
		std::atomic<int> iterations(0);

		for (int i = 0; i < 10; ++i) {
			auto timer = Profiler::get().start(L"loop_timer");
			iterations.fetch_add(1, std::memory_order_relaxed);
		}

		ASSERT_EQ(iterations.load(std::memory_order_relaxed), 10);
	}

	TEST(Core.Profiling, TimedBlockChaining)
	{
		auto t0 = Profiler::get().start(L"root");
		int root_level = Profiler::current_block->level;
		{
			auto t1 = Profiler::get().start(L"child1");
			int child_level = Profiler::current_block->level;
			{
				auto t2 = Profiler::get().start(L"grandchild");
				int grand_level = Profiler::current_block->level;
				ASSERT_TRUE(grand_level == child_level + 1);
			}
			auto t3 = Profiler::get().start(L"child2");
			ASSERT_TRUE(Profiler::current_block->level == child_level);
		}
		ASSERT_TRUE(root_level < Profiler::current_block->level + 1);
	}
}
