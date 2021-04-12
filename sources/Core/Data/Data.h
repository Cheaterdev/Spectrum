namespace DataPacker
{

	std::string pack(std::string unpacked);

	std::string unpack(std::string str);

	ZipArchiveEntry::Ptr create_entry(ZipArchive::Ptr archive, std::string name, std::string data, bool pack = false);

	std::string zip_to_string(ZipArchive::Ptr archive);

	std::string load_all(istream& s);

};


inline size_t HashIterate(size_t Next, size_t CurrentHash = 2166136261U)
{
	return 16777619U * CurrentHash ^ Next;
}

template <typename T> inline size_t HashRange(const T* Begin, const T* End, size_t InitialVal = 2166136261U)
{
	size_t Val = InitialVal;

	while (Begin < End)
		Val = HashIterate((size_t)* Begin++, Val);

	return Val;
}

template <typename T> inline size_t HashStateArray(const T* StateDesc, size_t Count, size_t InitialVal = 2166136261U)
{
	static_assert((sizeof(T) & 3) == 0, "State object is not word-aligned");
	return HashRange((UINT*)StateDesc, (UINT*)(StateDesc + Count), InitialVal);
}

template <typename T> inline size_t HashState(const T* StateDesc, size_t InitialVal = 2166136261U)
{
	static_assert((sizeof(T) & 3) == 0, "State object is not word-aligned");
	return HashRange((UINT*)StateDesc, (UINT*)(StateDesc + 1), InitialVal);
}

template<typename T> concept HaveEqual =
requires (T a, T b) {
	a == b;
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

	/*	var_typed(const T& v) : var_base(typeid(T)), value(v)
		{
		}
		*/
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
	//      LEAK_TEST
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
	/*
	template<class T>
	void create()
	{
		typed.reset(new var_typed<T>());
	}

	*/
	void operator = (MyVariant value);


	//template<class T>
	bool operator!=(const MyVariant& other) const
	{
		//var_typed<T>* var = dynamic_cast<var_typed<T>*>(typed.get());
		if (!typed) return true;
	
		return !typed->Equals(other.typed.get());

	//	return true;
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

		assert(false);
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
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
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
	/*template<class T>
	void get(T f)
	{

	}*/

	T* operator->()
	{
		return get().get();
	}
};



std::strong_ordering operator<=>(const std::string& l, const std::string& r);


template<class type, size_t size>
std::strong_ordering operator<=>(const std::array<type, size>& l, const std::array<type, size>& r)
{
	for (size_t i = 0; i < l.size(); i++)
		if (auto cmp = (l[i] <=> r[i]); cmp != 0) return cmp;

	return  std::strong_ordering::equal;
}




template<class type>
std::strong_ordering operator<=>(const std::vector<type>& l, const std::vector<type>& r)
{

	if (auto cmp = l.size() <=> r.size(); cmp != 0) return cmp;


	for (size_t i = 0; i < l.size(); i++)
		if (auto cmp = (l[i] <=> r[i]); cmp != 0) return cmp;

	return  std::strong_ordering::equal;
}




template<class type>
bool operator==(const std::vector<type>& l, const std::vector<type>& r)
{

	if (l.size() != r.size()) return false;


	for (size_t i = 0; i < l.size(); i++)
		if (l[i] != r[i])  return false;

	return  true;
}




/*


template<class type>
std::strong_ordering operator<=>(const std::shared_ptr<type>& l, const std::shared_ptr<type>& r)
{
	//if (auto cmp = l.get() <=> r.get(); cmp != 0) return cmp;
	return  *l <=> *r;
}




template<class type>
bool operator==(const std::shared_ptr<type>& l, const std::shared_ptr<type>& r)
{

	return  *l == *r;
}
*/

#define GENERATE_OPS __GENERATE_OPS__

template<typename T> concept EnumType =
requires (T t) {
	{T::__GENERATE_OPS__ } ;
};

template<EnumType Enum>
Enum operator ~(const Enum l)
{
	using underlying = typename std::underlying_type<Enum>::type;
	return static_cast<Enum> (
		~static_cast<underlying>(l)
		);
}

template<EnumType Enum>
Enum operator |(const Enum lhs, const Enum rhs)
{
	using underlying = typename std::underlying_type<Enum>::type;
	return static_cast<Enum> (
		static_cast<underlying>(lhs) |
		static_cast<underlying>(rhs)
		);
}



template<EnumType Enum>
Enum operator &(const Enum lhs, const Enum rhs)
{
	using underlying = typename std::underlying_type<Enum>::type;
	return static_cast<Enum> (
		static_cast<underlying>(lhs) &
		static_cast<underlying>(rhs)
		);
}


template<EnumType Enum>
bool operator &&(const Enum lhs, const Enum rhs)
{
	using underlying = typename std::underlying_type<Enum>::type;
	return static_cast<Enum> (
		static_cast<underlying>(lhs) &&
		static_cast<underlying>(rhs)
		);
}



template<EnumType Enum>
bool operator ==(const Enum lhs, const Enum rhs)
{
	using underlying = typename std::underlying_type<Enum>::type;
	return static_cast<Enum> (
		static_cast<underlying>(lhs) ==
		static_cast<underlying>(rhs)
		);
}

template<EnumType Enum>
bool operator !=(const Enum lhs, const Enum rhs)
{
	using underlying = typename std::underlying_type<Enum>::type;
	return static_cast<Enum> (
		static_cast<underlying>(lhs) !=
		static_cast<underlying>(rhs)
		);
}

template<EnumType Enum>
bool operator <=(const Enum lhs, const Enum rhs)
{
	using underlying = typename std::underlying_type<Enum>::type;
	return static_cast<Enum> (
		static_cast<underlying>(lhs) <=
		static_cast<underlying>(rhs)
		);
}

template<EnumType Enum>
 bool check(Enum lhs)
{
	using underlying = typename std::underlying_type<Enum>::type;
	return  (
		static_cast<underlying>(lhs)
		) != 0;
}


 template<EnumType E, class T, std::size_t N = std::size_t(E::__GENERATE_OPS__)>
class enum_array : public std::array<T, N> {
public:
    T & operator[] (E e) {
        return std::array<T, N>::operator[]((std::size_t)e);
    }

    const T & operator[] (E e) const {
        return std::array<T, N>::operator[]((std::size_t)e);
    }
};
///////////////////////////////////////////////
///
///
/*

class A
{
	bool used_by(helper*)
	{
		
	}
};


class B: public helper
{
	std::set<A*> inserts;
};*/