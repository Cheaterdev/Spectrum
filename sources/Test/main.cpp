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

void SetupLogging()
{
	//Log::create<WinErrorLogger>();
	FileTXTLogger::create();
	VSOutputLogger::create();
	StdoutLogger::create();
	Log::get().set_logging_level(Log::LEVEL_ALL);
}

int main()
{
	SetupLogging();

	auto results = Test::TestRegistry::Instance().RunAll();
	Test::TestRegistry::Instance().PrintResults(results);

	bool any_failed = std::any_of(results.begin(), results.end(),
		[](const Test::TestResult& r) { return !r.passed && !r.skipped; });
	return (results.empty() || any_failed) ? 1 : 0;
}
