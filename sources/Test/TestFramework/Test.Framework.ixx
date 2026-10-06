module;

#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <map>
#include <set>
#include <algorithm>
#include <cctype>

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

	// Thrown (via SKIP_TEST) from inside a test that can't run in this
	// configuration, e.g. a feature a backend doesn't implement yet.
	class TestSkipped : public std::exception
	{
	public:
		explicit TestSkipped(const std::string& msg) : message(msg) {}
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

		// A test that raises the probe's error count fails, whatever its own
		// asserts said. Installed by the runner's main (the framework itself only
		// knows Core); `last` describes the most recent error. `settle` runs
		// before the count is read after a test: errors raised by work the test
		// queued but that hasn't been submitted yet must land in that test, not
		// in whichever test runs next.
		void SetErrorProbe(std::function<uint64_t()> count, std::function<std::string()> last,
		                   std::function<void()> settle)
		{
			error_count  = std::move(count);
			last_error   = std::move(last);
			error_settle = std::move(settle);
		}

		std::vector<TestResult> RunAll(std::string_view filter = {})
		{
			std::sort(tests.begin(), tests.end(), [](const Test& a, const Test& b) {
				if (a.category != b.category) return a.category < b.category;
				return a.name < b.name;
			});

			std::vector<TestResult> results;
			std::set<std::string> ranSetups;

			// A leading '-' inverts the filter: run everything except the matches.
			const bool exclude = filter.starts_with('-');
			const std::string_view pattern = exclude ? filter.substr(1) : filter;
			auto matches = [&](const Test& t) -> bool {
				if (pattern.empty()) return true;
				std::string full = t.category.empty() ? t.name : t.category + "::" + t.name;
				std::string lf(pattern), lful(full);
				std::transform(lf.begin(),   lf.end(),   lf.begin(),   [](unsigned char c){ return std::tolower(c); });
				std::transform(lful.begin(), lful.end(), lful.begin(), [](unsigned char c){ return std::tolower(c); });
				return lful.contains(lf) != exclude;
			};

			auto matchCount = std::count_if(tests.begin(), tests.end(), matches);

			Log::get() << Log::LEVEL_INFO << "========== Starting Tests ==========" << Log::endl;
			if (!filter.empty())
				Log::get() << Log::LEVEL_INFO << "Filter: \"" << std::string(filter) << "\" (" << matchCount << " of " << tests.size() << " test(s) selected)" << Log::endl;
			else
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
				if (!matches(test)) continue;

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

				const uint64_t errors_before = error_count ? error_count() : 0;

				try
				{
					test.func();
					result.passed = true;
				}
				catch (const TestSkipped& e)
				{
					result.skipped = true;
					result.passed = true;
					result.errorMessage = e.what();
				}
				catch (const TestFailure& e)
				{
					result.passed = false;
					result.errorMessage = e.what();
				}

				if (error_count && !result.skipped)
				{
					if (error_settle)
						error_settle();
					if (const uint64_t errors = error_count() - errors_before)
					{
						std::string msg = std::to_string(errors) + " validation error(s) during the test, last: " + last_error();
						result.errorMessage = result.passed ? msg : result.errorMessage + " | " + msg;
						result.passed = false;
					}
				}

				if (result.skipped)
					Log::get() << Log::LEVEL_INFO << "[SKIP] " << fullName << " (" << result.errorMessage << ")" << Log::endl;
				else if (result.passed)
					Log::get() << Log::LEVEL_INFO << "[PASS] " << fullName << Log::endl;
				else
				{
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
		std::function<uint64_t()>          error_count;
		std::function<std::string()>       last_error;
		std::function<void()>              error_settle;
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

