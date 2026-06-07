module;

#include <string>
#include <vector>
#include <functional>
#include <sstream>

export module Test.Framework;

import Core;

export namespace Test
{
	struct TestResult
	{
		std::string category;
		std::string name;
		bool passed = false;
		std::string errorMessage;
		std::string file;
		int line = 0;
	};

	class TestRegistry
	{
	public:
		using TestFunc = std::function<void()>;

		static TestRegistry& Instance()
		{
			static TestRegistry instance;
			return instance;
		}

		void Register(const std::string& category, const std::string& name, TestFunc func, const std::string& file, int line)
		{
			tests.push_back({category, name, func, file, line});
		}

		std::vector<TestResult> RunAll()
		{
			std::vector<TestResult> results;

			Log::get() << Log::LEVEL_INFO << "========== Starting Tests ==========" << Log::endl;
			Log::get() << Log::LEVEL_INFO << "Running " << tests.size() << " test(s)..." << Log::endl;

			for (const auto& test : tests)
			{
				TestResult result;
				result.category = test.category;
				result.name = test.name;
				result.file = test.file;
				result.line = test.line;

				std::string fullName = result.category.empty() ? result.name : result.category + "::" + result.name;
				Log::get() << Log::LEVEL_INFO << ">> Starting: " << fullName << Log::endl;

				try
				{
					test.func();
					result.passed = true;
					Log::get() << Log::LEVEL_INFO << "[PASS] " << fullName << Log::endl;
				}
				catch (const std::string& e)
				{
					result.passed = false;
					result.errorMessage = e;
					Log::get() << Log::LEVEL_ERROR << "[FAIL] " << fullName << Log::endl;
					Log::get() << Log::LEVEL_ERROR << "       " << result.errorMessage << Log::endl;
				}

				results.push_back(result);
			}
			return results;
		}

		void PrintResults(const std::vector<TestResult>& results)
		{
			int passed = 0, failed = 0;
			std::map<std::string, std::pair<int, int>> categoryStats;

			for (const auto& result : results)
			{
				if (result.passed)
					passed++;
				else
					failed++;

				auto& stats = categoryStats[result.category];
				if (result.passed)
					stats.first++;
				else
					stats.second++;
			}

			Log::get() << Log::LEVEL_INFO << "========== Test Summary ==========" << Log::endl;

			for (const auto& [category, stats] : categoryStats)
			{
				std::string categoryName = category.empty() ? "Uncategorized" : category;
				Log::get() << Log::LEVEL_INFO << categoryName << ": " << (stats.first + stats.second)
					<< " total, " << stats.first << " passed";
				if (stats.second > 0)
					Log::get() << Log::LEVEL_ERROR << ", " << stats.second << " failed" << Log::endl;
				else
					Log::get() << Log::LEVEL_INFO << Log::endl;
			}

			Log::get() << Log::LEVEL_INFO << "\nTotal Tests: " << results.size() << Log::endl;
			Log::get() << Log::LEVEL_INFO << "Passed: " << passed << Log::endl;
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
	};

	class TestFailure : public std::exception
	{
	public:
		explicit TestFailure(const std::string& msg) : message(msg) {}
		const char* what() const noexcept override { return message.c_str(); }

	private:
		std::string message;
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

#define TEST(category, name) \
	void Test_##category##_##name(); \
	Test::TestRegistrator registrator_##category##_##name(#category, #name, Test_##category##_##name, __FILE__, __LINE__); \
	void Test_##category##_##name()

#define ASSERT_TRUE(condition) \
	Test::AssertTrue(condition, #condition, __FILE__, __LINE__)

#define ASSERT_FALSE(condition) \
	Test::AssertFalse(condition, #condition, __FILE__, __LINE__)

#define ASSERT_EQ(expected, actual) \
	Test::AssertEqual(expected, actual, __FILE__, __LINE__)

#define ASSERT_NE(expected, actual) \
	Test::AssertNotEqual(expected, actual, __FILE__, __LINE__)

