export module Core:Debug;

import stl.core;
import stl.threading;
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

	for (auto&& i : instances)
	{
		OutputDebugStringA("\n");
		OutputDebugStringA(("-------------LEAK:" + std::to_string(i->alloc_number) + " \n" + i->stack + "\n").c_str());
	}

	OutputDebugStringA("\n");
	OutputDebugStringA(("SUMMARY: " + std::to_string(instances.size()) + "\n").c_str());

	for (auto&& i : name_counters)
	{
		if (i.second == 0) continue;

		OutputDebugStringA("\n");
		OutputDebugStringA((i.first + ": " + std::to_string(i.second) + "\n").c_str());
	}
}



#endif