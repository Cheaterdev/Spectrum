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
import Core;
import windows;

void SetupLogging()
{
	//Log::create<WinErrorLogger>();
	FileTXTLogger::create();
	VSOutputLogger::create();
	StdoutLogger::create();
	Log::get().set_logging_level(Log::LEVEL_ALL);
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	SetupLogging();

	auto results = Test::TestRegistry::Instance().RunAll();
	Test::TestRegistry::Instance().PrintResults(results);

	return results.empty() || std::any_of(results.begin(), results.end(),
		[](const Test::TestResult& r) { return !r.passed; }) ? 1 : 0;
}
