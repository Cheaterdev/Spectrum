/*struct stack_trace_element
{
	DWORD64 address;
	stack_trace_element(DWORD64 address) :address(address)
	{

	}
	std::string get_name();
};
*/
/*
struct stack_trace :public std::vector<stack_trace_element>
{

	std::string to_string();
};

*/

#include <boost/stacktrace.hpp>


namespace Exceptions
{
	using stack_trace = boost::stacktrace::stacktrace;


	class Exception : public std::exception
	{
		stack_trace stack;
	public:
		Exception(std::string_view wtf);
		stack_trace get_stack_trace();


	};

	stack_trace get_stack_trace();
}
#ifdef SPECTRUM_ENABLE_EXEPTIONS
#define SPECTRUM_TRY try{
#define SPECTRUM_CATCH }catch (const std::system_error& e){Log::get() << Log::LEVEL_ERROR << e.what() << Log::endl;}
#else
#define SPECTRUM_TRY {
#define SPECTRUM_CATCH }
#endif

// #define LEAK_TEST_ENABLE
#ifdef LEAK_TEST_ENABLE
class LeakDetectorInstance;
class LeakDetector : public Singleton<LeakDetector>
{
	std::set<LeakDetectorInstance*> instances;
	std::set<long> breaks;
	std::map<std::string, int> name_counters;
	long alloc_number = 0;
public:

	void add(LeakDetectorInstance* e);

	void remove(LeakDetectorInstance* e);

	void break_on(long i)
	{
		breaks.insert(i);
	}

	virtual ~LeakDetector();

};
class LeakDetectorInstance
{
	friend class LeakDetector;
	std::string stack;
	std::string name;
	long alloc_number = -1;
	LeakDetectorInstance() {}
public:
	LeakDetectorInstance& operator = (const LeakDetectorInstance& r)
	{
		this->stack = r.stack;
		this->name = r.name;
		LeakDetector::get().add(this);
		return *this;
	}

	LeakDetectorInstance(const LeakDetectorInstance& r)
	{
		stack = r.stack;
		this->name = r.name;
		LeakDetector::get().add(this);
	}


	LeakDetectorInstance(std::string name)
	{
		//         stack = get_stack_trace(true);
		this->name = name;
		LeakDetector::get().add(this);
	}

	virtual ~LeakDetectorInstance()
	{
		LeakDetector::get().remove(this);
	}
};

#define LEAK_TEST(x) LeakDetectorInstance ___leak_tester = LeakDetectorInstance(#x);

#else


#define LEAK_TEST(x) ;

#endif



template<class T>
class Counter;

class CounterManager : public Singleton<CounterManager>
{
public:
	std::vector<std::function<void()>> print_functions;

	void print()
	{
		for (auto& f : print_functions)
			f();
	}

	template<class T> Counter<T> start_count(std::string name = "")
	{
		return Counter<T>(name);
	}
};
template<class T>
class Counter
{
	std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
	static std::atomic_int32_t time;
	std::string name;
public:

	Counter(std::string name)
	{
		this->name = name;
		static std::once_flag flag;
		call_once(flag, []()
		{
			CounterManager::get().print_functions.push_back([]()
			{
				Log::get() << "summary [" << typeid(T).name() << "]: " << long(time) << Log::endl;
			});
		});
	}
	~Counter()
	{
		long count = static_cast<long>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count());
		// Log::get() << "timer [" << typeid(T).name() << "] " << name << " :" << count << Log::endl;
		time.fetch_add(count);
	}

	static void log()
	{
	}
};

template<class T>
std::atomic_int32_t Counter<T>::time = 0;