export module Core:Debug;

import stl.core;
import stl.threading;
import :Singleton;
import :Log;
import :Tree;
import :Exceptions;

export
{

#ifndef _WIN32
	// On Windows this is an inline function in Core/Defines.h; there the forced include must stay
	// macro-only (Clang rejects declarations ahead of `module X;`), so it lives here instead.
	namespace Core
	{
		[[noreturn]] void assert_fail(const char* expr, const char* file, int line);
	}
#endif

	struct BuildOptions
	{
		static constexpr bool Debug = true;
		static constexpr bool Release = false;
#ifdef LEAK_TEST_ENABLE
		static constexpr bool ObjectTracking = true;
#else
		static constexpr bool ObjectTracking = false;
#endif

#ifdef DEV
		static constexpr bool Dev = true;
#else
		static constexpr bool Dev = false;
#endif
	};

	using ObjectTypeID = uint64;

	class Object
	{
		std::string object_name;
		Exceptions::stack_trace creation_stack;
	protected:
		Object();
	public:
		bool debug = false;
		virtual void set_name(std::string_view str);

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
		ObjectTracker(std::string_view name);
		~ObjectTracker();
		void track(Object* object);
		void untrack(Object* object);
	};

	template<class T>
	class TypedObject : public Object
	{
		// A function-local static, not a static data member: Clang rejects another module
		// instantiating an out-of-line static member that lives in this one, and an in-class
		// initializer would evaluate typeid(T) while T is still incomplete (TypedObject<Device>
		// is a base of Device), which MSVC rejects. The body is only instantiated on use.
		static ObjectTracker& tracker()
		{
			static ObjectTracker instance(typeid(T).name());
			return instance;
		}
	protected:
		TypedObject()
		{
			if constexpr (BuildOptions::ObjectTracking)
			{
				if (ObjectTracker::alive)
					tracker().track(this);
			}

		}
		~TypedObject() {
			if constexpr (BuildOptions::ObjectTracking)
			{
				if (ObjectTracker::alive)
					tracker().untrack(this);
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

		void print();

		template<class T> Counter<T> start_count(std::string name = "")
		{
			return Counter<T>(name);
		}
	};

	template<class T>
	class Counter
	{
		std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
		// Defined in-class: an out-of-line definition in this module can't be instantiated by
		// other modules under Clang ("declaration of 'time' ... follows declaration in module Core:Debug").
		inline static std::atomic_int32_t time{ 0 };
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
			time.fetch_add(count);
		}

		static void log()
		{
		}
	};



	class Checker
	{
		std::atomic<std::thread::id>& v;
	public:
		Checker(std::atomic<std::thread::id>& c);
		~Checker();
	};




}

