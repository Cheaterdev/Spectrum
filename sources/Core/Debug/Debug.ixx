export module Core:Debug;

import <Core_defs.h>;
import stl.core;
import stl.threading;
import :Singleton;
import :Log;
 import :Tree;

export
{

	struct BuildOptions
	{
		static constexpr bool Debug = true;
		static constexpr bool Release = false;
#ifdef LEAK_TEST_ENABLE
		static constexpr bool ObjectTracking = true;
#else
		static constexpr bool ObjectTracking = false;
#endif
	};

	using ObjectTypeID = uint64;

	class Object
	{
		std::string object_name;
		Exceptions::stack_trace creation_stack;
	protected:
		Object()
		{
			creation_stack = Exceptions::get_stack_trace();
		}
	public:
		bool debug = false;
		virtual void set_name(std::string_view str)
		{
			object_name = str;
		}

		std::string_view get_name() const { return object_name; }
		Exceptions::stack_trace get_creation_stack() const { return creation_stack; }
	};


	class ObjectTracker
	{
		std::string name;
		std::mutex m;
		std::set<Object*> objects;

	public:
		static bool alive;
		ObjectTracker(std::string_view name) :name(name)
		{
			Log::get() << "ObjectTracker created: " << name << Log::endl;

		}
		~ObjectTracker()
		{
			alive = false;
			std::lock_guard<std::mutex> g(m);
			for (auto& o : objects)
			{
				Log::get() << "LEAKED OBJECT (" << name << "): '" << o->get_name() << "' stacktrace \n" << o->get_creation_stack() << Log::endl;
			}
		}
		void track(Object* object)
		{
			std::lock_guard<std::mutex> g(m);
			objects.insert(object);
		}

		void untrack(Object* object)
		{
			std::lock_guard<std::mutex> g(m);
			objects.erase(object);
		}
	};

	template<class T>
	class TypedObject : public Object
	{
		static ObjectTracker tracker;
		//constexpr static int _id{};
		//static constexpr ObjectTypeID TypeID = ObjectTypeID(&_id);
	protected:
		TypedObject()
		{
			if constexpr (BuildOptions::ObjectTracking)
			{
				if (ObjectTracker::alive)
					tracker.track(this);
			}

		}
		~TypedObject() {
			if constexpr (BuildOptions::ObjectTracking)
			{
				if (ObjectTracker::alive)
					tracker.untrack(this);
			}
		}
	public:
		static std::string_view get_type_name() { return typeid(T).name(); }
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
			ASSERT(prev == std::thread::id());
		}

		~Checker()
		{
			auto id = std::this_thread::get_id();
			auto prev = v.exchange(std::thread::id());
			ASSERT(prev == id);
		}
	};


	



}


template<class T>
ObjectTracker TypedObject<T>::tracker(TypedObject<T>::get_type_name());

bool		ObjectTracker::alive = true;
