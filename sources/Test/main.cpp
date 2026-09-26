// TEMP: Vulkan crash investigation -- remove once the crash is found.
#include <stacktrace>
#include <fstream>
struct _EXCEPTION_POINTERS;
extern "C" __declspec(dllimport) void* __stdcall AddVectoredExceptionHandler(unsigned long, long(__stdcall*)(_EXCEPTION_POINTERS*));
static long __stdcall temp_crash_handler(_EXCEPTION_POINTERS* ep)
{
	auto code = **reinterpret_cast<unsigned long**>(ep);
	if (code == 0xC0000005 || code == 0xC00000FD)
		std::ofstream("crash_stack.temp", std::ios::app) << std::hex << "code 0x" << code << std::dec << "\n"
			<< std::stacktrace::current() << "\n----\n" << std::flush;
	return 0; // EXCEPTION_CONTINUE_SEARCH
}

import Test.Framework;
import Test.Math;
import Test.Core;
import Test.Math.Extended;
import Test.Serialization;
import Test.Threading;
import Test.Events;
import Test.FileSystem;
import Test.Profiling;
import Test.HAL;
import Test.HAL.SubresRangeMap;
import Core;

void SetupLogging()
{
	//Log::create<WinErrorLogger>();
	FileTXTLogger::create();
	VSOutputLogger::create();
	StdoutLogger::create();
	Log::get().set_logging_level(Log::LEVEL_ALL);
}

int main(int argc, char** argv)
{
	Application::test_mode = true;
	SetupLogging();

	// TEMP: Vulkan crash investigation -- remove once the crash is found.
	AddVectoredExceptionHandler(1, temp_crash_handler);

	std::string filter;
	for (int i = 1; i < argc; ++i)
	{
		std::string arg(argv[i]);
		if (arg.rfind("--filter=", 0) == 0)
			filter = arg.substr(9);
		else if ((arg == "--filter" || arg == "-k") && i + 1 < argc)
			filter = argv[++i];
		else if (arg[0] != '-')
			filter = arg;
	}

	auto results = Test::TestRegistry::Instance().RunAll(filter);
	Test::TestRegistry::Instance().PrintResults(results);

	bool any_failed = std::any_of(results.begin(), results.end(),
		[](const Test::TestResult& r) { return !r.passed && !r.skipped; });

	// Same as RenderApplication's shutdown: ~scheduler resets thread_pool, and
	// left to static destruction the thread_pool singleton may already be gone.
	scheduler::reset();

	return (results.empty() || any_failed) ? 1 : 0;
}
