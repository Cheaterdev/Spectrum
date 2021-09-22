#include <boost/stacktrace.hpp>


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


class Checker
{
	std::atomic<std::thread::id>& v;
public:
	Checker(std::atomic<std::thread::id>& c) :v(c)
	{
		auto id = std::this_thread::get_id();
		auto prev = v.exchange(id);
		assert(prev== std::thread::id());
	}

	~Checker()
	{
		auto id = std::this_thread::get_id();
		auto prev = v.exchange(std::thread::id());
		assert(prev == id);
	}
};
#define THREAD_CHECKER std::atomic<std::thread::id> __checker_;

#define ASSERT_SINGLETHREAD Checker __g__(__checker_);
