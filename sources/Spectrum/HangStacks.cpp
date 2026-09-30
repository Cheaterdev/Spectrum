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
#include <atomic>
#include <algorithm>
#include <new>
#include <cstdlib>

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

// TEMP: Vulkan CPU heap leak -- sampling operator new. With
// SPECTRUM_TEMP_HEAP_SAMPLE set, 1 in 64 allocations records its call stack;
// 40 s after start, allocations made after 20 s and still alive are grouped
// by stack and written to heap_sites.temp. Every block carries a 16-byte
// header (0 = unsampled, else slot + 1) so delete needs no lookup.
namespace temp_heap
{
	struct Slot
	{
		std::atomic<bool> live;
		uint32_t size;
		uint32_t t_ms;
		void* pcs[18];
	};
	constexpr size_t SLOTS = 1 << 18;
	Slot slots[SLOTS];
	std::atomic<size_t> next_slot{ 0 };
	std::atomic<uint64_t> counter{ 0 };
	thread_local bool busy = false;
	const ULONGLONG start = GetTickCount64();

	bool enabled()
	{
		static const bool on = [] { char b[4]; return GetEnvironmentVariableA("SPECTRUM_TEMP_HEAP_SAMPLE", b, 4) > 0; }();
		return on;
	}

	void dump()
	{
		busy = true;
		struct Site { uint64_t bytes = 0; uint32_t count = 0; const Slot* sample = nullptr; };
		std::vector<std::pair<uint64_t, Site>> sites;
		const size_t n = std::min(next_slot.load(), SLOTS);
		{
			std::vector<std::pair<uint64_t, const Slot*>> live;
			for (size_t i = 0; i < n; i++)
				if (slots[i].live && slots[i].t_ms >= 20000)
				{
					uint64_t h = 1469598103934665603ull;
					for (int k = 2; k < 18; k++) h = (h ^ uint64_t(slots[i].pcs[k])) * 1099511628211ull;
					live.push_back({ h, &slots[i] });
				}
			std::sort(live.begin(), live.end(), [](auto& a, auto& b) { return a.first < b.first; });
			for (auto& [h, s] : live)
			{
				if (sites.empty() || sites.back().first != h) sites.push_back({ h, Site{ 0, 0, s } });
				sites.back().second.bytes += s->size;
				sites.back().second.count++;
			}
		}
		std::sort(sites.begin(), sites.end(), [](auto& a, auto& b) { return a.second.bytes > b.second.bytes; });

		HANDLE proc = GetCurrentProcess();
		SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
		SymInitialize(proc, nullptr, TRUE);
		std::ofstream out("heap_sites.temp");
		out << "slots used " << n << ", sampled 1/256, live allocations made after 20s\n\n";
		for (size_t i = 0; i < sites.size() && i < 15; i++)
		{
			out << "~" << sites[i].second.bytes * 256 / 1024 << " KB in ~" << sites[i].second.count * 256 << " blocks\n";
			for (int k = 2; k < 18 && sites[i].second.sample->pcs[k]; k++)
			{
				const DWORD64 pc = DWORD64(sites[i].second.sample->pcs[k]);
				alignas(SYMBOL_INFO) char buf[sizeof(SYMBOL_INFO) + 512]{};
				auto* sym = reinterpret_cast<SYMBOL_INFO*>(buf);
				sym->SizeOfStruct = sizeof(SYMBOL_INFO);
				sym->MaxNameLen   = 511;
				DWORD64 disp = 0;
				IMAGEHLP_LINE64 line{ sizeof(line) };
				DWORD line_disp = 0;
				out << "  " << (SymFromAddr(proc, pc, &disp, sym) ? sym->Name : "?");
				if (SymGetLineFromAddr64(proc, pc, &line_disp, &line))
					out << "  " << line.FileName << ":" << line.LineNumber;
				out << "\n";
			}
			out << "\n";
		}
	}

	struct Starter
	{
		Starter()
		{
			if (!enabled()) return;
			std::thread([] { std::this_thread::sleep_for(std::chrono::seconds(40)); dump(); }).detach();
		}
	} starter;
}

void* operator new(size_t size)
{
	auto* base = static_cast<uint64_t*>(malloc(size + 16));
	if (!base) throw std::bad_alloc();
	base[0] = 0;
	if (temp_heap::start && temp_heap::enabled() && !temp_heap::busy && GetTickCount64() - temp_heap::start >= 20000
		&& temp_heap::counter.fetch_add(1, std::memory_order_relaxed) % 256 == 0)
	{
		const size_t i = temp_heap::next_slot.fetch_add(1, std::memory_order_relaxed);
		if (i < temp_heap::SLOTS)
		{
			auto& s = temp_heap::slots[i];
			s.size = uint32_t(size);
			s.t_ms = uint32_t(GetTickCount64() - temp_heap::start);
			const USHORT got = CaptureStackBackTrace(0, 18, s.pcs, nullptr);
			for (int k = got; k < 18; k++) s.pcs[k] = nullptr;
			s.live = true;
			base[0] = i + 1;
		}
	}
	return base + 2;
}

void operator delete(void* p) noexcept
{
	if (!p) return;
	auto* base = static_cast<uint64_t*>(p) - 2;
	if (base[0]) temp_heap::slots[base[0] - 1].live = false;
	free(base);
}

void operator delete(void* p, size_t) noexcept { operator delete(p); }

void temp_dump_all_stacks_after(int seconds)
{
	std::thread([seconds] {
		std::this_thread::sleep_for(std::chrono::seconds(seconds));
		dump_all_stacks();
	}).detach();
}
