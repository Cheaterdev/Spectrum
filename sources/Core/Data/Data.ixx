export module Core:Data;

import :serialization;

import stl.core;
import :Utils;
import :Exceptions;

import zlib;

import :shared_ptr;
export
{

	namespace DataPacker
	{

		std::string pack(std::string unpacked);

		std::string unpack(std::string str);

		

		std::string load_all(std::istream& s);

	};


	class MyVariant
	{

		class var_base
		{
			friend class MyVariant;
		protected:

			var_base(std::reference_wrapper<const std::type_info> r);

			virtual   std::unique_ptr<var_base> clone() = 0;

			virtual bool Equals(var_base* other) = 0;
		public:
			std::reference_wrapper<const std::type_info> type;
			virtual ~var_base() = default;
		};





		template<class T>
		class var_typed : public var_base
		{
		public:
			T value;

			template<class ...Args>
			var_typed(Args...args) : var_base(typeid(T)), value(args...)
			{

			}

			var_typed() : var_base(typeid(T))
			{

			}

			virtual bool Equals(var_base* other)
			{
				auto otyped = dynamic_cast<var_typed<T>*>(other);

				if (otyped)
				{
					if constexpr (HaveEqual<T>)
						return value == otyped->value;
					else
						return false;
				}

				return false;
			}


			virtual std::unique_ptr<var_base> clone() override
			{
				if constexpr (std::is_copy_constructible_v<T>)
					return std::make_unique<var_typed<T>>(value);
				else
					return nullptr;// std::make_unique<var_typed<T>>();
			}

			virtual ~var_typed()
			{
			};
		};
		std::unique_ptr<var_base> typed;

	public:
		MyVariant() = default;
		MyVariant(const MyVariant& r);
		virtual ~MyVariant();

		template<class T>
		void operator = (const T& value)
		{
			typed.reset(new var_typed<T>(value));
		}

		template<class T, class ...Args>
		void create(Args...args)
		{
			typed.reset(new var_typed<T>(args...));
		}
		void operator = (MyVariant value);


		bool operator!=(const MyVariant& other) const
		{
			if (!typed) return true;

			return !typed->Equals(other.typed.get());
		}
		template<class T>
		bool operator!=(const T& other) const
		{
			var_typed<T>* var = dynamic_cast<var_typed<T>*>(typed.get());

			if (var)
				return var->value != other;

			return true;
		}



		template<class T>
		T& get() const
		{
			var_typed<T>* var = dynamic_cast<var_typed<T>*>(typed.get());

			if (var)
				return var->value;

			ASSERT(false);
			throw Exceptions::Exception("wrong cast type");
		}

		template<class T>
		T* try_get() const
		{
			var_typed<T>* var = dynamic_cast<var_typed<T>*>(typed.get());

			if (var)
				return &var->value;

			return nullptr;
		}

		bool exists();
		void clear();
	};



	template<class I, class V>
	class Cache
	{

		std::mutex m;

	public:
		std::map<I, V> table;

		std::function<V(const I&)> create_func;

		size_t size()
		{
			return table.size();
		}


		void clear()
		{			m.lock();
				table.clear();
				m.unlock();
		}
		Cache() = default;

		Cache(std::function<V(const I&)> create_func) : create_func(create_func) {}

		V& operator[](const I& i)
		{
			m.lock();
			bool first = table.find(i) == table.end();
			auto&& elem = table[i];
			m.unlock();

			if (!elem)
			{
				if (first)
					elem = create_func(i);
				else
				{
					while (!elem)
						std::this_thread::yield();
				}
			}

			return elem;
		}

	private:
		SERIALIZE()
		{
			ar& NVP(table);
		}

	};

	
	template< class V>
	class Pool
	{

		std::mutex m;

	public:
		std::deque<V> table;

		std::function<V()> create_func;

		size_t size()
		{
			return table.size();
		}
		Pool() = default;

		Pool(std::function<V()> create_func) : create_func(create_func) {}

		V get()
		{
			std::lock_guard<std::mutex> g(m);
		
			if(table.empty())
			{
				return create_func();
			}


			auto elem = table.front();
			table.pop_front();

			return elem;
		}


		void put(const V& v)
		{

		std::lock_guard<std::mutex> g(m);
		table.push_back(v);
		}
	private:
		SERIALIZE()
		{
			ar& NVP(table);
		}

	};


	template<class T>
	class Void
	{
		std::shared_ptr<T> object;
		std::mutex m;
		bool loading = false;
	public:
		std::function<T()> create_func;



		Void() = default;

		Void(std::function<T()> create_func) : create_func(create_func) {}

		operator bool()
		{
			return object.valid();
		}

		void set(const T& obj)
		{
			object.reset(new T(obj));
		}

		std::shared_ptr<T>& get()
		{
			if (!object)
			{
				m.lock();
				bool me_loading = !loading;
				loading = true;
				m.unlock();

				if (me_loading)
					set(create_func());
				else
					while (!object)
						std::this_thread::yield();
			}

			return object;
		}
		T* operator->()
		{
			return get().get();
		}
	};


	template<class E, class T, std::size_t N = magic_enum::enum_count<E>()>
	class enum_array : public std::array<T, N> {
	public:
		T& operator[] (E e) {
			return std::array<T, N>::operator[]((std::size_t)e);
		}

		const T& operator[] (E e) const {
			return std::array<T, N>::operator[]((std::size_t)e);
		}
	};


	// Vector whose emplace_back/push_back is safe to call from many threads at
	// once: each caller claims a unique slot via an atomic counter. Requires
	// reserve() to have sized the backing storage up front (single-threaded) so
	// concurrent appends never reallocate or exceed capacity. Every other
	// operation (iteration, erase, front/back, indexing) is single-threaded and
	// meant to run after the concurrent-append phase has joined.
	//
	// The atomic counter only guarantees non-overlapping slots — visibility of
	// concurrently-appended elements to a later reader relies on the caller's
	// own join barrier (e.g. a thread-pool wait) establishing happens-before.
	template<class T>
	class concurrent_vector
	{
		std::vector<T>      storage;
		std::atomic<size_t> count{ 0 };

	public:
		concurrent_vector() = default;

		concurrent_vector(const concurrent_vector& r)
			: storage(r.storage), count(r.count.load(std::memory_order_relaxed)) {}

		concurrent_vector(concurrent_vector&& r) noexcept
			: storage(std::move(r.storage)), count(r.count.load(std::memory_order_relaxed))
		{
			r.count.store(0, std::memory_order_relaxed);
		}

		concurrent_vector& operator=(const concurrent_vector& r)
		{
			storage = r.storage;
			count.store(r.count.load(std::memory_order_relaxed), std::memory_order_relaxed);
			return *this;
		}

		concurrent_vector& operator=(concurrent_vector&& r) noexcept
		{
			storage = std::move(r.storage);
			count.store(r.count.load(std::memory_order_relaxed), std::memory_order_relaxed);
			r.count.store(0, std::memory_order_relaxed);
			return *this;
		}

		// Single-threaded: pre-size backing storage so concurrent emplace_back
		// never reallocates. Does not change the logical size (count).
		void reserve(size_t n)
		{
			if (storage.size() < n)
				storage.resize(n);
		}

		// Thread-safe append. Requires a free reserved slot (see reserve()).
		T& emplace_back(const T& v)
		{
			size_t i = count.fetch_add(1, std::memory_order_relaxed);
			ASSERT(i < storage.size());
			storage[i] = v;
			return storage[i];
		}
		T& push_back(const T& v) { return emplace_back(v); }

		size_t size()  const { return count.load(std::memory_order_relaxed); }
		bool   empty() const { return size() == 0; }

		T*       begin()       { return storage.data(); }
		T*       end()         { return storage.data() + size(); }
		const T* begin() const { return storage.data(); }
		const T* end()   const { return storage.data() + size(); }

		T&       operator[](size_t i)       { return storage[i]; }
		const T& operator[](size_t i) const { return storage[i]; }

		T&       front()       { return storage[0]; }
		const T& front() const { return storage[0]; }
		T&       back()        { return storage[size() - 1]; }
		const T& back()  const { return storage[size() - 1]; }

		void clear() { count.store(0, std::memory_order_relaxed); }

		// Single-threaded compacting erase, matching std::vector::erase(first,last).
		T* erase(T* first, T* last)
		{
			T* new_end = std::move(last, end(), first);
			count.store((size_t)(new_end - storage.data()), std::memory_order_relaxed);
			return first;
		}
	};


	class SharedObjectBase :public std::enable_shared_from_this<SharedObjectBase>
	{
	protected:

	public:
		virtual ~SharedObjectBase() = default;
	};

	template<class T>
	class SharedObject :public virtual SharedObjectBase
	{
	public:
		virtual ~SharedObject() = default;

		template <class M = T>
		std::shared_ptr<M> get_ptr()
		{
			return std::dynamic_pointer_cast<M>(shared_from_this());
		}
	};


}

