// Non-module TU: prints a symbolized call stack when a test run crashes.
// ASSERT already reports its expression and location on stderr before it
// breaks; an access violation reports nothing, so the run just ends with an
// exit code. The handler adds the stack for both -- the crashing call site
// for an AV, and who called the failing ASSERT for a breakpoint.

#include <windows.h>
#include <cstdio>
#include <stacktrace>
#include <string>

namespace
{
	LONG CALLBACK print_crash_stack(EXCEPTION_POINTERS* info)
	{
		const DWORD code = info->ExceptionRecord->ExceptionCode;
		// First-chance handler: C++ exceptions (0xE06D7363) and anything else a
		// test may catch pass through untouched.
		if (code != EXCEPTION_ACCESS_VIOLATION && code != EXCEPTION_BREAKPOINT
		    && code != EXCEPTION_STACK_OVERFLOW && code != EXCEPTION_ILLEGAL_INSTRUCTION)
			return EXCEPTION_CONTINUE_SEARCH;

		static volatile LONG reported = 0;
		if (InterlockedExchange(&reported, 1))
			return EXCEPTION_CONTINUE_SEARCH;

		std::fprintf(stderr, "CRASH: exception 0x%08lX at %p\n", code, info->ExceptionRecord->ExceptionAddress);
		if (code == EXCEPTION_ACCESS_VIOLATION && info->ExceptionRecord->NumberParameters >= 2)
			std::fprintf(stderr, "  %s address 0x%llX\n", info->ExceptionRecord->ExceptionInformation[0] ? "writing" : "reading",
				(unsigned long long)info->ExceptionRecord->ExceptionInformation[1]);
		// Raw frames first, as offsets into test.exe (look them up in bin/<config>/test.map: the
		// symbolized trace below comes out empty on some machines).
		{
			void* frames[48] = {};
			const USHORT count = CaptureStackBackTrace(0, 48, frames, nullptr);
			const auto base = reinterpret_cast<unsigned long long>(GetModuleHandleW(nullptr));
			std::fprintf(stderr, "  frames (RVA in test.exe, or the raw address outside it):\n");
			for (USHORT i = 0; i < count; i++)
			{
				const auto address = reinterpret_cast<unsigned long long>(frames[i]);
				if (address >= base && address < base + 0x10000000ull)
					std::fprintf(stderr, "    #%u 0x%llX\n", unsigned(i), address - base);
				else
					std::fprintf(stderr, "    #%u %p\n", unsigned(i), frames[i]);
			}
		}

		std::fputs(std::to_string(std::stacktrace::current()).c_str(), stderr);
		std::fputs("\n", stderr);
		std::fflush(stderr);
		return EXCEPTION_CONTINUE_SEARCH;
	}
}

void install_crash_handler()
{
	AddVectoredExceptionHandler(1, print_crash_stack);
}
