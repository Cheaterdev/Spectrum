#include <windows.h>

import TestFramework;
import Core;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	auto results = Test::TestRegistry::Instance().RunAll();
	Test::TestRegistry::Instance().PrintResults(results);

	return results.empty() || std::any_of(results.begin(), results.end(),
		[](const Test::TestResult& r) { return !r.passed; }) ? 1 : 0;
}
