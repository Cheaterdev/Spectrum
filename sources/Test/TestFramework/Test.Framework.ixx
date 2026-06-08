module;

#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <map>
#include <set>

export module Test.Framework;

import Core;

export namespace Test
{
	struct TestResult
	{
		std::string category;
		std::string name;
		bool passed = false;
		bool skipped = false;
		std::string errorMessage;
		std::string file;
		int line = 0;
	};

	class TestFailure : public std::exception
	{
	public:
		explicit TestFailure(const std::string& msg) : message(msg) {}
		const char* what() const noexcept override { return message.c_str(); }

	private:
		std::string message;
	};

	class TestRegistry
	{
	public:
		using TestFunc = std::function<void()>;
		using SetupFunc = std::function<void()>;
		using TeardownFunc = std::function<void()>;

		static TestRegistry& Instance()
		{
			static TestRegistry instance;
			return instance;
		}

		void Register(const std::string& category, const std::string& name, TestFunc func, const std::string& file, int line)
		{
			tests.push_back({category, name, func, file, line});
		}

		void RegisterSetup(const std::string& category, SetupFunc func)
		{
			setups[category] = std::move(func);
		}

		void RegisterTeardown(const std::string& category, TeardownFunc func)
		{
			teardowns[category] = std::move(func);
		}

		void SkipCategory(const std::string& category, const std::string& reason)
		{
			skipped_categories[category] = reason;
		}

		std::vector<TestResult> RunAll()
		{
			std::vector<TestResult> results;
			std::set<std::string> ranSetups;

			Log::get() << Log::LEVEL_INFO << "========== Starting Tests ==========" << Log::endl;
			Log::get() << Log::LEVEL_INFO << "Running " << tests.size() << " test(s)..." << Log::endl;

			auto runTeardown = [&](const std::string& category)
			{
				if (category.empty()) return;
				auto it = teardowns.find(category);
				if (it != teardowns.end())
				{
					Log::get() << Log::LEVEL_INFO << "[TEARDOWN] " << category << Log::endl;
					it->second();
				}
			};

			std::string currentCategory;

			for (const auto& test : tests)
			{
				if (test.category != currentCategory)
				{
					runTeardown(currentCategory);
					currentCategory = test.category;
				}

				if (!test.category.empty() && ranSetups.find(test.category) == ranSetups.end())
				{
					auto it = setups.find(test.category);
					if (it != setups.end())
					{
						Log::get() << Log::LEVEL_INFO << "[SETUP] " << test.category << Log::endl;
						it->second();
					}
					ranSetups.insert(test.category);
				}

				TestResult result;
				result.category = test.category;
				result.name = test.name;
				result.file = test.file;
				result.line = test.line;

				std::string fullName = result.category.empty() ? result.name : result.category + "::" + result.name;

				auto skipIt = skipped_categories.find(test.category);
				if (skipIt != skipped_categories.end())
				{
					result.skipped = true;
					result.passed = true;
					result.errorMessage = skipIt->second;
					Log::get() << Log::LEVEL_INFO << "[SKIP] " << fullName << " (" << skipIt->second << ")" << Log::endl;
					results.push_back(result);
					continue;
				}

				Log::get() << Log::LEVEL_INFO << ">> Starting: " << fullName << Log::endl;

				try
				{
					test.func();
					result.passed = true;
					Log::get() << Log::LEVEL_INFO << "[PASS] " << fullName << Log::endl;
				}
				catch (const TestFailure& e)
				{
					result.passed = false;
					result.errorMessage = e.what();
					Log::get() << Log::LEVEL_ERROR << "[FAIL] " << fullName << Log::endl;
					Log::get() << Log::LEVEL_ERROR << "       " << result.errorMessage << Log::endl;
				}

				results.push_back(result);
			}

			runTeardown(currentCategory);

			return results;
		}

		void PrintResults(const std::vector<TestResult>& results)
		{
			int passed = 0, failed = 0, skipped = 0;

			struct CategoryStats { int passed = 0; int failed = 0; int skipped = 0; };
			std::map<std::string, CategoryStats> categoryStats;

			for (const auto& result : results)
			{
				auto& stats = categoryStats[result.category];
				if (result.skipped)
				{
					skipped++;
					stats.skipped++;
				}
				else if (result.passed)
				{
					passed++;
					stats.passed++;
				}
				else
				{
					failed++;
					stats.failed++;
				}
			}

			Log::get() << Log::LEVEL_INFO << "========== Test Summary ==========" << Log::endl;

			for (const auto& [category, stats] : categoryStats)
			{
				std::string categoryName = category.empty() ? "Uncategorized" : category;
				int total = stats.passed + stats.failed + stats.skipped;
				std::stringstream ss;
				ss << categoryName << ": " << total << " total, " << stats.passed << " passed";
				if (stats.skipped > 0)
					ss << ", " << stats.skipped << " skipped";
				if (stats.failed > 0)
					ss << ", " << stats.failed << " failed";
				Log::get() << Log::LEVEL_INFO << ss.str() << Log::endl;
			}

			Log::get() << Log::LEVEL_INFO << "Total Tests: " << results.size() << Log::endl;
			Log::get() << Log::LEVEL_INFO << "Passed: " << passed << Log::endl;
			if (skipped > 0)
				Log::get() << Log::LEVEL_INFO << "Skipped: " << skipped << Log::endl;
			if (failed > 0)
				Log::get() << Log::LEVEL_ERROR << "Failed: " << failed << Log::endl;
			else
				Log::get() << Log::LEVEL_INFO << "Failed: " << failed << Log::endl;
			Log::get() << Log::LEVEL_INFO << "==================================" << Log::endl;
		}

	private:
		struct Test
		{
			std::string category;
			std::string name;
			TestFunc func;
			std::string file;
			int line;
		};

		std::vector<Test> tests;
		std::map<std::string, SetupFunc> setups;
		std::map<std::string, TeardownFunc> teardowns;
		std::map<std::string, std::string> skipped_categories;
	};


	inline void AssertTrue(bool condition, const std::string& message, const std::string& file, int line)
	{
		if (!condition)
		{
			std::stringstream ss;
			ss << file << ":" << line << " - Assertion failed: " << message;
			throw TestFailure(ss.str());
		}
	}

	inline void AssertFalse(bool condition, const std::string& message, const std::string& file, int line)
	{
		if (condition)
		{
			std::stringstream ss;
			ss << file << ":" << line << " - Assertion failed (expected false): " << message;
			throw TestFailure(ss.str());
		}
	}

	template<typename T>
	inline void AssertEqual(const T& expected, const T& actual, const std::string& file, int line)
	{
		if (expected != actual)
		{
			std::stringstream ss;
			ss << file << ":" << line << " - Assertion failed: expected " << expected << " but got " << actual;
			throw TestFailure(ss.str());
		}
	}

	template<typename T>
	inline void AssertNotEqual(const T& expected, const T& actual, const std::string& file, int line)
	{
		if (expected == actual)
		{
			std::stringstream ss;
			ss << file << ":" << line << " - Assertion failed: values should not be equal";
			throw TestFailure(ss.str());
		}
	}

	class TestRegistrator
	{
	public:
		TestRegistrator(const std::string& category, const std::string& name, TestRegistry::TestFunc func, const std::string& file, int line)
		{
			TestRegistry::Instance().Register(category, name, func, file, line);
		}
	};
}

