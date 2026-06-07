module;

#include <string>
#include <vector>
#include <functional>
#include <sstream>

export module TestFramework;

import Core;

export namespace Test
{
	struct TestResult
	{
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

		void Register(const std::string& name, TestFunc func, const std::string& file, int line)
		{
			tests.push_back({name, func, file, line});
		}

		std::vector<TestResult> RunAll()
		{
			std::vector<TestResult> results;

			Log::get() << Log::LEVEL_INFO << "========== Starting Tests ==========\n" << Log::endl;
			Log::get() << Log::LEVEL_INFO << "Running " << tests.size() << " test(s)...\n" << Log::endl;

			for (const auto& test : tests)
			{
				TestResult result;
				result.name = test.name;
				result.file = test.file;
				result.line = test.line;

				Log::get() << Log::LEVEL_INFO << "Running: " << result.name << Log::endl;

				try
				{
					test.func();
					result.passed = true;
					Log::get() << Log::LEVEL_INFO << "  [PASS] " << result.name << Log::endl;
				}
				catch (const std::string& e)
				{
					result.passed = false;
					result.errorMessage = e;
					Log::get() << Log::LEVEL_ERROR << "  [FAIL] " << result.name << Log::endl;
					Log::get() << Log::LEVEL_ERROR << "  Error: " << result.errorMessage << Log::endl;
				}

				results.push_back(result);
			}
			return results;
		}

		void PrintResults(const std::vector<TestResult>& results)
		{
			int passed = 0, failed = 0;

			for (const auto& result : results)
			{
				if (result.passed)
					passed++;
				else
					failed++;
			}

			Log::get() << Log::LEVEL_INFO << "\n========== Test Summary ==========\n" << Log::endl;
			Log::get() << Log::LEVEL_INFO << "Total Tests: " << results.size() << Log::endl;
			Log::get() << Log::LEVEL_INFO << "Passed: " << passed << Log::endl;
			if (failed > 0)
				Log::get() << Log::LEVEL_ERROR << "Failed: " << failed << Log::endl;
			else
				Log::get() << Log::LEVEL_INFO << "Failed: " << failed << Log::endl;
			Log::get() << Log::LEVEL_INFO << "=================================\n" << Log::endl;
		}

	private:
		struct Test
		{
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
		TestRegistrator(const std::string& name, TestRegistry::TestFunc func, const std::string& file, int line)
		{
			TestRegistry::Instance().Register(name, func, file, line);
		}
	};
}

