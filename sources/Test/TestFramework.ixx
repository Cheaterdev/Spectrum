module;

#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <sstream>

export module Test:Framework;

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
			for (const auto& test : tests)
			{
				TestResult result;
				result.name = test.name;
				result.file = test.file;
				result.line = test.line;

				try
				{
					test.func();
					result.passed = true;
				}
				catch (const std::string& e)
				{
					result.passed = false;
					result.errorMessage = e;
				}

				results.push_back(result);
			}
			return results;
		}

		void PrintResults(const std::vector<TestResult>& results)
		{
			int passed = 0, failed = 0;

			std::cout << "\n========== Test Results ==========\n";
			for (const auto& result : results)
			{
				if (result.passed)
				{
					std::cout << "[PASS] " << result.name << "\n";
					passed++;
				}
				else
				{
					std::cout << "[FAIL] " << result.name << "\n";
					std::cout << "       " << result.file << ":" << result.line << "\n";
					std::cout << "       " << result.errorMessage << "\n";
					failed++;
				}
			}

			std::cout << "\n=================================\n";
			std::cout << "Total: " << results.size() << " | Passed: " << passed << " | Failed: " << failed << "\n";
			std::cout << "=================================\n\n";
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

#define TEST(name) \
	void Test_##name(); \
	Test::TestRegistrator registrator_##name(#name, Test_##name, __FILE__, __LINE__); \
	void Test_##name()

#define ASSERT_TRUE(condition) \
	Test::AssertTrue(condition, #condition, __FILE__, __LINE__)

#define ASSERT_FALSE(condition) \
	Test::AssertFalse(condition, #condition, __FILE__, __LINE__)

#define ASSERT_EQ(expected, actual) \
	Test::AssertEqual(expected, actual, __FILE__, __LINE__)

#define ASSERT_NE(expected, actual) \
	Test::AssertNotEqual(expected, actual, __FILE__, __LINE__)
