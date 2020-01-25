#pragma once
namespace Events
{

	class Runner
	{
		std::mutex m;
		std::list<std::function<void()>> tasks;
	protected:
	public:
		void process_tasks()
		{
			std::list<std::function<void()>> copy;
			{
				std::lock_guard<std::mutex> g(m);
				std::swap(copy, tasks);
			}

			for (auto &t : copy)
			{
				t();
			}

		}
	

		//template<class T>
		void run(std::function<void()> f)
		{
			std::lock_guard<std::mutex> g(m);
			tasks.emplace_back(f);
		}
	};
	template<typename U>
	class prop;

	class prop_helper
	{
		template<typename U>
		friend class prop;
		template<typename U>
		friend class Event;
	//	std::function<void(T)> func;
		std::function<void()> remove_func;

		Runner* runner = nullptr;
	public:
		virtual ~prop_helper()
		{
			if (remove_func)
				remove_func();
		}
	};


	template<class T>
	class prop_t_helper :public prop_helper
	{
		template<typename U>
		friend class prop;
		template<typename U>
		friend class Event;
		std::function<void(T)> func;
	//	Event<T> *main_event;

	
	};
	class prop_handler
	{
		template<typename U>
		friend class prop;

		template<typename U>
		friend class Event;

		std::vector<std::shared_ptr<prop_helper>> helpers;

	public:
		virtual ~prop_handler()
		{
			helpers.clear();
		};
	};

	template<class T>
	class Event : public prop_handler
	{
		//	using FUNC  =std::function<void)>
	//	std::vector<std::shared_ptr<std::function<void(T)>>>handlers;
		std::vector<prop_t_helper<T>*> i_helpers;
		std::mutex m;
		Event<T>& operator =(const Event<T>&) = delete;
		
	public:
		Runner* runner = nullptr;
		std::function<void(std::function<void(T)> f)> default_state;

		void operator=(std::function<void(T)> func)
		{
			register_handler(nullptr, func);

		}
		prop_t_helper<T>* register_handler(prop_handler* owner, std::function<void(T)> func)
		{
			prop_t_helper<T>* result = nullptr;
			if (!owner) owner = this;
			if (owner)
			{
				std::lock_guard<std::mutex> g(m);
				
					std::shared_ptr<prop_t_helper<T>> helper(new prop_t_helper<T>());
					helper->func = func;
					helper->runner = dynamic_cast<Runner*>(owner);

				//	auto f = std::make_shared<std::function<void(T)>>(func);
				//	handlers.push_back(f);
					auto h = helper.get();
					helper->remove_func = [this, h]()
					{
						std::lock_guard<std::mutex> g(m);

						i_helpers.erase(std::find(i_helpers.begin(), i_helpers.end(), h));
					//	handlers.erase(std::find(handlers.begin(), handlers.end(), f));
					};
					i_helpers.emplace_back(h);
					owner->helpers.push_back(helper);
					result = h;
				
			}

			if (default_state)
				default_state(func);
			return result;
		}

		template<class ...Args>
		void operator()(Args...args)
		{

			std::lock_guard<std::mutex> g(m);

				for (auto& p : i_helpers)
					if (p->runner)
					{
						p->runner->run([args...,p]() {
							p->func(args...);
						});
					}else

					p->func(args...);
		
		}

		virtual~Event()
		{
			std::lock_guard<std::mutex> g(m);
			for (auto& p : i_helpers)
			{
				if (p)p->remove_func = nullptr;
			}
		}
	};


	

	template<class T>
	class prop
	{
	public:
		using function_type = void(const T&);

	private:
		std::mutex m;

		std::unique_ptr<T> value;
		my_unique_vector<std::shared_ptr<std::function<function_type>>>on_change;
		my_unique_vector<std::shared_ptr<prop_helper>> helpers;

		prop<T>& operator =(const prop<T>&) = delete;


		template<class T>
		void send_one(T f)
		{
			f(*value);

		}

		void send()
		{
			for (auto p : on_change)
				send_one(*p);
		}
	public:
		Runner* runner = nullptr;
		prop() = default;

		prop(const T&t):value(std::make_unique<T>(t))  {};
		using event_type = Event<const T&>;
		void register_change(prop_handler* owner, std::function<function_type> func)
		{
			std::lock_guard<std::mutex> g(m);
			if (owner)
			{
				std::shared_ptr<prop_helper> helper(new prop_helper());
				auto f = std::make_shared<std::function<function_type>>(func);
				on_change.insert(f);
				helper->remove_func = [this, f]()
				{
					std::lock_guard<std::mutex> g(m);

					on_change.erase(f);
				};
				owner->helpers.push_back(helper);
				helpers.insert(helper);
			}
			if(value)
			func(*value);
		}
		void register_change(event_type& event)
		{
			std::lock_guard<std::mutex> g(m);

			auto func = [&event](const T & d)
			{
				event(d);
			};
			std::shared_ptr<prop_helper> helper(new prop_helper());
			auto f = std::make_shared<std::function<function_type>>(func);
			on_change.insert(f);
			helper->remove_func = [this, f]()
			{
				std::lock_guard<std::mutex> g(m);

				on_change.erase(f);
			};
			event.helpers.push_back(helper);
			helpers.insert(helper);
			event.default_state = [this](std::function<function_type> f2)
			{
				if (value)
					send_one(f2);
			};
			if (value)
				send_one(func);
		}

		operator const T() const
		{
			return *value;
		}

		const T operator=(const T& r)
		{

			if (!value||*value != r)
			{

				if (!value)
					value = std::make_unique<T>(r);
				else
					*value = r;
				std::lock_guard<std::mutex> g(m);

				send();
		
			}

			return r;
		}

		void set(const T& r)
		{
			if (!value)
				value = std::make_unique<T>(r);
			else
				*value = r;

			std::lock_guard<std::mutex> g(m);

			send();
		}
		const bool operator==(const T& r) const
		{
			if (!value)
				return false;
			return *value == r;
		}

		const T* operator->() const
		{
			return value;
		}

		const T& get() const
		{
			return *value;
		}

		const T& operator*() const
		{
			return *value;
		}

		virtual~prop()
		{
			std::lock_guard<std::mutex> g(m);

			for (auto p : helpers)
				p->remove_func = nullptr;
		}

		const bool has_value() const {
			return !!value;
		}
	private:
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int)
		{


			ar& NVP(value);

			if (Archive::is_loading::value)
			{
				send();
			}
		}

	};

}

template<class T>
class Variable
{
	T value;
	std::string name;
public:

	Variable(const T &def, std::string name) :value(def), name(name)
	{

	}

	operator T() const
	{
		return value;
	}
	const T operator=(const T& r)
	{

		return value = r;
	}

	std::string get_name()
	{
		return name;
	}
};