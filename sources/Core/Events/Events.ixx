export module Core:Events;

import :Math;
import :Data;
import :serialization;

export namespace Events
{
	template<class ...T> class Event;
	class Runner
	{
		std::mutex m;
		std::list<std::function<void()>> tasks;
	protected:
		bool has_tasks();
	public:
		void process_tasks();
		void run(std::function<void()> f);
	};

	template<typename U>
	class prop;

	class prop_handler;
	class prop_helper
	{
		friend class prop_handler;
	protected:

		std::function<void()> remove_func;
		// The publisher (Event, or formerly prop) this helper is registered
		// against -- used only for identity comparison in
		// prop_handler::unregister. Named `owner` (not `event`) because
		// nothing here is an Event: renamed from the original `event` field,
		// which read as if it held an actual Event object.
		prop_handler* owner = nullptr;
		Runner* runner = nullptr;

	public:
		void clear_remove();
		void unregister();
		virtual ~prop_helper();
	};


	template<class ...T>
	class prop_t_helper :public prop_helper
	{
		friend class Event<T...>;

		std::function<void(T...)> func;


		// T can be a reference (Event<const T&>, e.g. what prop's event_type
		// instantiates, or FrameGraph::Graph::on_compile broadcasting itself
		// by const ref). Marshaling through a Runner defers the call, so
		// std::tuple<T...>(args...) -- which for a reference T stores the
		// REFERENCE itself, not a copy -- would have the deferred lambda read
		// whatever the caller's argument pointed to, quite possibly after
		// that object is gone (a temporary, a stack local in the emitter's
		// now-returned frame). Decaying to a value type and copying fixes
		// that -- but only when the decayed type is actually copyable; some
		// Events (on_compile above) deliberately broadcast a large,
		// non-copyable object by reference for identity, not by value, and
		// have no copy to make. For those, fall back to the original
		// reference-capturing behavior: it's on whoever registers a
		// Runner-owning subscriber to such an Event to ensure the referenced
		// object outlives every task that Runner has queued.
		static constexpr bool can_copy_capture = std::conjunction_v<std::is_copy_constructible<std::decay_t<T>>...>;

		void run(T...args)
		{
			if (runner)
			{
				if constexpr (can_copy_capture)
				{
					runner->run([f = func, captured = std::tuple<std::decay_t<T>...>(args...)]() {
						std::apply(f, captured);
					});
				}
				else
				{
					runner->run([f = func, captured = std::tuple<T...>(args...)]() {
						std::apply(f, captured);
					});
				}
			}
			else
				func(args...);
		}

	};

	class prop_handler
	{
		std::vector<std::shared_ptr<prop_helper>> helpers;
	protected:

		void add_helper(prop_handler* handler, std::shared_ptr<prop_helper> helper);

	public:

		void unregister(prop_handler* owner);

		virtual ~prop_handler();
	};

	template<class ...T>
	class Event : public prop_handler
	{
	public:
		using func_type = std::function<void(T...)>;

	private:
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
					helper->owner = this;
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

		void operator()(T...args)
		{
			std::lock_guard<std::mutex> g(m);

				for (auto& p : i_helpers)
					p->run(std::forward<T>(args)...);
		}

		// Any listeners currently registered? Lets emitters skip work that
		// only exists to feed the event (e.g. debug thumbnail capture).
		bool has_handlers()
		{
			std::lock_guard<std::mutex> g(m);
			return !i_helpers.empty();
		}

		virtual ~Event()
		{
			std::lock_guard<std::mutex> g(m);
			for (auto& p : i_helpers)
			{
				if (p)p->clear_remove();
			}
		}
	};




	// A value plus change notification, built directly on top of Event
	// instead of reimplementing its own listener bookkeeping and dispatch:
	// on_change already gets owner-scoped auto-unregister (including correct
	// teardown if `this` is destroyed before a subscriber -- Event's own
	// destructor disarms every helper it owns), Runner-aware marshaling, and
	// the "replay current value to a new subscriber" hook (default_state) for
	// free. prop no longer needs to be a prop_handler itself: Event already
	// handles both destruction orders on its own.
	//
	// `m` protects `value` only (reads and writes alike -- the previous
	// version guarded the listener list but left `value` itself unlocked, a
	// real race for any prop touched from more than one thread). It is never
	// held while invoking a subscriber callback, to avoid a callback that
	// re-enters this same prop (sets it again, registers another listener)
	// deadlocking on a non-recursive mutex.
	template<class T>
	class prop
	{
	public:
		using function_type = void(const T&);
		using event_type = Event<const T&>;

	private:
		mutable std::mutex m;
		T value;
		event_type on_change;

		prop<T>& operator =(const prop<T>&) = delete;

	public:
		prop() = default;
		prop(const T& t) : value(t) {}

		void register_change(prop_handler* owner, std::function<function_type> func)
		{
			on_change.register_handler(owner, func);
			func(get());
		}

		void register_change(event_type& event)
		{
			on_change.register_handler(&event, [&event](const T& d) { event(d); });

			event.default_state = [this](std::function<function_type> f2)
			{
				f2(get());
			};

			event(get());
		}

		operator T() const
		{
			return get();
		}

		T operator=(const T& r)
		{
			bool changed;
			{
				std::lock_guard<std::mutex> g(m);
				changed = (value != r);
				if (changed) value = r;
			}

			if (changed)
				on_change(r);

			return r;
		}

		// Unconditional -- unlike operator=, always notifies even if the
		// value didn't change. Existing behavior, kept as-is.
		void set(const T& r)
		{
			{
				std::lock_guard<std::mutex> g(m);
				value = r;
			}

			on_change(r);
		}

		bool operator==(const T& r) const
		{
			return get() == r;
		}

		T get() const
		{
			std::lock_guard<std::mutex> g(m);
			return value;
		}

		T operator*() const
		{
			return get();
		}

		// operator->() used to exist here but returned `value` (type T) where
		// the signature demanded `const T*` -- did not compile for any real T,
		// so it was silently dead code, never instantiated. Not resurrected:
		// with `value` now behind a mutex, a raw T* into it would be exactly
		// as unsound as the old version was uncompilable -- get()/operator*
		// return a safely-locked copy instead, which is what every actual
		// caller in the codebase already uses.

	private:
		SERIALIZE()
		{
			{
				std::lock_guard<std::mutex> g(m);
				ar& NVP(value);
			}

			IF_LOAD()
			{
				on_change(get());
			}
		}
	};

}
