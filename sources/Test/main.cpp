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
import Test.RenderSystem.FrameGraph;
import Core;
import HAL;
import RenderSystem;

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

	// A graphics-API validation error during a test fails that test.
	Test::TestRegistry::Instance().SetErrorProbe(
		[] { return (uint64_t)HAL::Debug::ValidationLog::error_count(); },
		[] { return HAL::Debug::ValidationLog::last_error(); },
		[] { if (RenderSystem::is_good()) RenderSystem::get().device().wait_idle(); });

	auto results = Test::TestRegistry::Instance().RunAll(filter);
	Test::TestRegistry::Instance().PrintResults(results);

	bool any_failed = std::any_of(results.begin(), results.end(),
		[](const Test::TestResult& r) { return !r.passed && !r.skipped; });

	// Same as RenderApplication's shutdown: ~scheduler resets thread_pool, and
	// left to static destruction the thread_pool singleton may already be gone.
	scheduler::reset();

	return (results.empty() || any_failed) ? 1 : 0;
}
