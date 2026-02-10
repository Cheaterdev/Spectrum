export module Core:Debug;

import <stl/core.h>;
import <stl/threading.h>;
import :Singleton;
import :Log;

export
{

	struct BuildOptions
	{
		static constexpr bool Debug = true;
		static constexpr bool Release = false;

	};

#ifdef LEAK_TEST_ENABLE
	class LeakDetectorInstance;
	class LeakDetector : public Singleton<LeakDetector>
	{
		std::mutex m;
		std::set<long> breaks;
		std::map<std::string, std::set<LeakDetectorInstance*>> name_counters;
		long alloc_number = 0;
	public:

		void add(LeakDetectorInstance* e);

		void remove(LeakDetectorInstance* e);

		void break_on(long i)
		{
			breaks.insert(i);
		}

		virtual ~LeakDetector();


		void print(std::string);
	};
	class LeakDetectorInstance
	{
		friend class LeakDetector;
		Exceptions::stack_trace stack;
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
			 stack = Exceptions::get_stack_trace();
			this->name = name;
			LeakDetector::get().add(this);
		}

		virtual ~LeakDetectorInstance()
		{
			LeakDetector::get().remove(this);
		}
	};



#endif

	class Object
	{

		std::string name;
	
	public:
			bool debug = false;
		virtual void set_name(std::string_view str)
		{
			name = str;
		}


	
	};

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
			assert(prev == std::thread::id());
		}

		~Checker()
		{
			auto id = std::this_thread::get_id();
			auto prev = v.exchange(std::thread::id());
			assert(prev == id);
		}
	};


}




#ifdef LEAK_TEST_ENABLE
void LeakDetector::add(LeakDetectorInstance* e)
{
	std::lock_guard<std::mutex> g(m);
	name_counters[e->name].insert(e);

	if (breaks.count(alloc_number) > 0)
		assert(false);

	e->alloc_number = alloc_number++;
}

void LeakDetector::remove(LeakDetectorInstance* e)
{
	std::lock_guard<std::mutex> g(m);
	name_counters[e->name].erase(e);

}

void LeakDetector::print(std::string name)
{
	std::lock_guard<std::mutex> g(m);
		 auto&list =name_counters[name];
			 Log::get() << "LEAKS COUNT: " << name << " " << list.size() << Log::endl;

		for (auto e : list)
		{
			 Log::get() << "LEAKS" << e->alloc_number<< " " <<  e->stack<< Log::endl;
	}
}

LeakDetector::~LeakDetector()
{
	std::lock_guard<std::mutex> g(m);
	for (auto& [name, list] : name_counters)
	{

		if (list.empty()) continue;
		Log::get() << "LEAKS COUNT: " << name << " " << list.size() << Log::endl;

		for (auto e : list)
		{
			 Log::get() << "LEAKS" << e->alloc_number<< " " <<  e->stack<< Log::endl;
		}
	}


}



#endif