export module Core:Events;

import :Math;
import :Data;
import :serialization;
import :my_unique_vector;

export namespace Events
{
	template<class ...T> class Event;
	class Runner
	{
		std::mutex m;
		std::list<std::function<void()>> tasks;
	protected:

		bool has_tasks()
		{
			return !tasks.empty();
		}
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

	class prop_handler;
	class prop_helper
	{
		template<typename U>
		friend class prop;
	   	   friend class prop_handler;
   	protected:
		
		std::function<void()> remove_func;
		 prop_handler* event = nullptr;
		Runner* runner = nullptr;



	public:
		void clear_remove()
		{
		  	remove_func = nullptr;
		}
		void unregister()
		{
			if (remove_func)
				remove_func();
			remove_func = nullptr;
		}
		virtual ~prop_helper()
		{
			unregister();
		}
	};


	template<class ...T>
	class prop_t_helper :public prop_helper
	{
		template<typename U>
		friend class prop;
		friend class Event<T...>;
	   friend class prop_handler;

		std::function<void(T...)> func;

					
		void run(T...args)
		{
			if (runner)
			{
				runner->run([f = func, captured = std::tuple<T...>(args...)]() {
					std::apply(f, captured);
				});
			}
			else
				func(args...);
		}

	};

	class prop_handler
	{
		std::vector<std::shared_ptr<prop_helper>> helpers;
	protected:

		void add_helper(prop_handler* handler, std::shared_ptr<prop_helper> helper)
		{
		   handler->helpers.push_back(helper);
		
		}

		/*void remove_helper(prop_helper* helper)
		{
			helpers.erase(std::find_if(helpers.begin(), helpers.end(), [helper](const std::shared_ptr<prop_helper>& h){ return h.get()==helper;}));

		}  */

	/*template<class ...Args> 
		void run(Args...args)
		{
			  	for (auto& p : helpers)
				{
				   auto typed = static_cast<prop_t_helper<Args...>>(p.get());
					  p->run(args...);
				
				}
					
		}
		*/


	
		void clear_remove_funcs()
		{
		  for (auto& p : helpers)
			{
				if (p)p->remove_func = nullptr;
			}
		}

	
	public:

			void unregister(prop_handler *owner)
		{
			for(auto&helper:owner->helpers)
			{
			   if(helper->event==this)
			   {
			   	   helper->unregister();
				   
			   }
			
			}
		}


		virtual ~prop_handler()
		{
			helpers.clear();
		};
	};

	template<class ...T>
	class Event : public prop_handler
	{
	public:
		using func_type = std::function<void(T...)>;

	private:
		//	using FUNC  =std::function<void)>
	//	std::vector<std::shared_ptr<std::function<void(T)>>>handlers;
		std::vector<prop_t_helper<T...>*> i_helpers;
		std::mutex m;
		Event<T...>& operator =(const Event<T...>&) = delete;
		
	public:
		Runner* runner = nullptr;
		std::function<void(func_type f)> default_state;
		
		void operator=(func_type func)
		{
			 register_handler(nullptr, func);
		}

		prop_t_helper<T...>* register_handler(prop_handler* owner, func_type func)
		{
			prop_t_helper<T...>* result = nullptr;
			if (!owner) owner = this;
			if (owner)
			{
				std::lock_guard<std::mutex> g(m);
				
					std::shared_ptr<prop_t_helper<T...>> helper(new prop_t_helper<T...>());
					helper->event = this;
					helper->func = func;
					helper->runner = dynamic_cast<Runner*>(owner);
					auto h = helper.get();
					helper->remove_func = [this, h]()
					{
						std::lock_guard<std::mutex> g(m);
						i_helpers.erase(std::find(i_helpers.begin(), i_helpers.end(), h));
					};
					i_helpers.emplace_back(h);	 

					add_helper(owner, helper);
		
					result = h;
				
			}

			if (default_state)
				default_state(func);
			return result;
		}


	//	template<class ...Args>
		void operator()(T...args)
		{

			std::lock_guard<std::mutex> g(m);


				for (auto& p : i_helpers)			
					p->run(std::forward<T>(args)...);
		
		}

		virtual~Event()
		{
			std::lock_guard<std::mutex> g(m);
			for (auto& p : i_helpers)
			{
				if (p)p->clear_remove();
			}
		}
	};


	

	template<class T>
	class prop: public prop_handler
	{
	public:
		using function_type = void(const T&);

	private:
		std::mutex m;

		T value;
		my_unique_vector<std::shared_ptr<std::function<function_type>>>on_change;

		prop<T>& operator =(const prop<T>&) = delete;


		template<class T>
		void send_one(T f)
		{
			f(value);

		}

		void send()
		{
			for (auto p : on_change)
				send_one(*p);
		}
	public:
		Runner* runner = nullptr;
		prop() = default;

		prop(const T&t):value(t)  {};
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
				add_helper(owner, helper);
				add_helper(this, helper);
			}
		//	if(value)
			func(value);
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

			add_helper(&event, helper);
		   	add_helper(this, helper);

			event.default_state = [this](std::function<function_type> f2)
			{
			//	if (value)
					send_one(f2);
			};
		//	if (value)
				send_one(func);
		}

		operator const T() const
		{
			return value;
		}

		const T operator=(const T& r)
		{

			if (value != r)
			{

				value = r;
				std::lock_guard<std::mutex> g(m);

				send();
		
			}

			return r;
		}

		void set(const T& r)
		{
		
				value = r;

			std::lock_guard<std::mutex> g(m);

			send();
		}
		const bool operator==(const T& r) const
		{
		
			return value == r;
		}

		const T* operator->() const
		{
			return value;
		}

		const T& get() const
		{
			return value;
		}

		const T& operator*() const
		{
			return value;
		}

		virtual~prop()
		{
			std::lock_guard<std::mutex> g(m);

			clear_remove_funcs();
		}


	private:
		SERIALIZE()
		{
			ar& NVP(value);

			IF_LOAD()
			{
				send();
			}
		}
	};

}

