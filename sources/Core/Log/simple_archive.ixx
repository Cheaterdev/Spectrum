module;
///import Utils;

export module simple_log_archive;

export import cereal;

import stl.core;
import magic_enum;


template<typename T>
concept Enum = std::is_enum_v<T>;


export class simple_log_archive: public cereal::OutputArchive<simple_log_archive>
{
	std::ostream& m_os;
	unsigned int m_depth;
	unsigned int t_depth;

	template<Enum T>
	void save(const T& t)
	{
		m_os << magic_enum::enum_name(t);
	}

	template<class T>
	void save(const T& t)
	{
/*
		if constexpr (std::is_enum_v<T>)
		{
			m_os << magic_enum::enum_name(t);
		}
		else if constexpr (boost::mpl::equal_to<
			boost::serialization::implementation_level<T>,
			boost::mpl::int_<boost::serialization::primitive_type>
		>::value)
		{
			m_os << t;
		}
		else
		{
			// make sure call is routed through the highest interface that might
		   // be specialized by the user.
			boost::serialization::serialize_adl(
				*this,
				const_cast<T&>(t),
				::boost::serialization::version<T>::value
			);
		}
		*/
	}

	void save(const std::wstring& s)
	{
	//	m_os << convert(s);
	}

public:

	static constexpr bool PRETTY = true;


	// this can be a no-op since we ignore pointer polymorphism
	template<class T>
	void register_type(const T* = NULL) {}

	unsigned int get_library_version()
	{
		return 0;
	}

	void
		save_binary(const void*, std::size_t)
	{
		m_os << "save_binary not implemented";
	}

	// the << operators
	template<class T>
	simple_log_archive& operator<<(T const& t)
	{
		m_os << ' ';
		save(t);
		return *this;
	}

	simple_log_archive& operator<<(void* const t)
	{
		m_os << " ->";

		if (NULL == t)
			m_os << " null";
		else
			*this << t;

		return *this;
	}

	template<class T>
	simple_log_archive& operator<<(T t) requires (std::is_pointer_v<T>)
	{
		m_os << " ->";

		if (NULL == t)
			m_os << " null";
		else
			*this << *t;

		return *this;
	}

	/*
	template<class T, int N>
	simple_log_archive& operator<<(const T(&t)[N])
	{
		return *this << boost::serialization::make_array(
			static_cast<const T*>(&t[0]),
			N
		);
	}

	template<class T>
	simple_log_archive& operator<<(const boost::serialization::nvp<T>& t)
	{
		m_os << "\n"; // start line with each named object

		// indent according to object depth
		for (unsigned int i = 0; i < m_depth; ++i)
			m_os << ' ';

		m_os << "{";
		++m_depth;

		if (m_depth > t_depth)
			t_depth = m_depth;

		m_os << t.name() << " ="; // output the name of the object
		*this << t.const_value();

		if (t_depth != m_depth)
		{
			m_os << "\n";

			for (unsigned int i = 0; i < m_depth - 1; ++i)
				m_os << ' ';

			t_depth = m_depth;
		}

		--m_depth;
		m_os << "}";
		return *this;
	}
	*/
	// the & operator
	template<class T>
	simple_log_archive& operator&(const T& t)
	{
		return *this << t;
	}
	///////////////////////////////////////////////

	simple_log_archive(std::ostream& os) :
		cereal::OutputArchive<simple_log_archive>(this),
		m_os(os),
		m_depth(0), t_depth(0)
	{}
};


CEREAL_REGISTER_ARCHIVE(simple_log_archive)