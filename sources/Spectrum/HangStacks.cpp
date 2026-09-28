// TEMP: Vulkan 3D bring-up hang investigation -- delete this file once found.
// Writes every thread's call stack to hang_stacks.temp, `seconds` after the
// call. Non-module TU: <dbghelp.h> next to the module imports in main.cpp
// reintroduces the std double-definition problem.
#include <windows.h>
#include <tlhelp32.h>
#include <dbghelp.h>
#include <fstream>
#include <thread>
#include <chrono>
#include <vector>

#pragma comment(lib, "dbghelp.lib")

static void dump_all_stacks()
{
	HANDLE proc = GetCurrentProcess();
	SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
	SymInitialize(proc, nullptr, TRUE);

	std::ofstream out("hang_stacks.temp");
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	THREADENTRY32 te{ sizeof(te) };
	for (BOOL ok = Thread32First(snap, &te); ok; ok = Thread32Next(snap, &te))
	{
		if (te.th32OwnerProcessID != GetCurrentProcessId() || te.th32ThreadID == GetCurrentThreadId())
			continue;
		HANDLE th = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, te.th32ThreadID);
		if (!th) continue;

		// Only raw PCs while the thread is suspended: symbolizing allocates, and
		// the suspended thread may hold the heap lock.
		DWORD64 pcs[64];
		int count = 0;
		SuspendThread(th);
		CONTEXT ctx{};
		ctx.ContextFlags = CONTEXT_FULL;
		if (GetThreadContext(th, &ctx))
		{
			STACKFRAME64 f{};
			f.AddrPC.Offset    = ctx.Rip; f.AddrPC.Mode    = AddrModeFlat;
			f.AddrFrame.Offset = ctx.Rbp; f.AddrFrame.Mode = AddrModeFlat;
			f.AddrStack.Offset = ctx.Rsp; f.AddrStack.Mode = AddrModeFlat;
			while (count < 64 && StackWalk64(IMAGE_FILE_MACHINE_AMD64, proc, th, &f, &ctx, nullptr,
				SymFunctionTableAccess64, SymGetModuleBase64, nullptr) && f.AddrPC.Offset)
				pcs[count++] = f.AddrPC.Offset;
		}
		ResumeThread(th);
		CloseHandle(th);

		out << "thread " << te.th32ThreadID << "\n";
		for (int i = 0; i < count; i++)
		{
			alignas(SYMBOL_INFO) char buf[sizeof(SYMBOL_INFO) + 512]{};
			auto* sym = reinterpret_cast<SYMBOL_INFO*>(buf);
			sym->SizeOfStruct = sizeof(SYMBOL_INFO);
			sym->MaxNameLen   = 511;
			DWORD64 disp = 0;
			IMAGEHLP_LINE64 line{ sizeof(line) };
			DWORD line_disp = 0;
			out << "  " << (SymFromAddr(proc, pcs[i], &disp, sym) ? sym->Name : "?");
			if (SymGetLineFromAddr64(proc, pcs[i], &line_disp, &line))
				out << "  " << line.FileName << ":" << line.LineNumber;
			out << "\n";
		}
		out << "\n";
	}
	CloseHandle(snap);
}

void temp_dump_all_stacks_after(int seconds)
{
	std::thread([seconds] {
		std::this_thread::sleep_for(std::chrono::seconds(seconds));
		dump_all_stacks();
	}).detach();
}
