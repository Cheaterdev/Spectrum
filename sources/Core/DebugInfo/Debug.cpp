#include "pch.h"

#include <DbgHelp.h>

namespace Exceptions
{

	Exception::Exception(std::string_view wtf)  : std::exception((std::string(wtf) + "\n At:" + boost::stacktrace::to_string(stack)).c_str())
	{
	}

	stack_trace Exception::get_stack_trace()
	{
		return 	stack;
	}


	stack_trace get_stack_trace()
	{
		return boost::stacktrace::stacktrace();
	}
}



#ifdef LEAK_TEST_ENABLE
void LeakDetector::add(LeakDetectorInstance* e)
{
	instances.insert(e);
	name_counters[e->name]++;

	if (breaks.count(alloc_number) > 0)
		assert(false);

	e->alloc_number = alloc_number++;
}

void LeakDetector::remove(LeakDetectorInstance* e)
{
	name_counters[e->name]--;
	instances.erase(e);
}

LeakDetector::~LeakDetector()
{
	Log::get() << "LEAKS COUNT: " << instances.size() << Log::endl;
	OutputDebugStringA(("LEAKS COUNT: " + std::to_string(instances.size()) + "\n").c_str());
	OutputDebugStringA("\n");

	for (auto && i : instances)
	{
		OutputDebugStringA("\n");
		OutputDebugStringA(("-------------LEAK:" + std::to_string(i->alloc_number) + " \n" + i->stack + "\n").c_str());
	}

	OutputDebugStringA("\n");
	OutputDebugStringA(("SUMMARY: " + std::to_string(instances.size()) + "\n").c_str());

	for (auto && i : name_counters)
	{
		if (i.second == 0) continue;

		OutputDebugStringA("\n");
		OutputDebugStringA((i.first + ": " + std::to_string(i.second) + "\n").c_str());
	}
}



#endif
/*
std::string stack_trace_element::get_name()
{
	SYMBOL_INFO*   symbol;

	std::string result;

	symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
	symbol->MaxNameLen = 255;
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);


	SymFromAddr(GetCurrentProcess(), address, 0, symbol);
	result = symbol->Name;

	free(symbol);
	return result;
}

std::string stack_trace::to_string()
{
	std::string res;

	for (auto&e : *this)
	{
		res += e.get_name() + "\n";
	};

	return res;
}
*/