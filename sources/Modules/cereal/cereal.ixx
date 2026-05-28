export module cereal;

export import "_cereal.h";
export import stl.core;

//export cereal::detail::msb_32bit;

export using serialization_oarchive = cereal::BinaryOutputArchive;
export using serialization_iarchive = cereal::BinaryInputArchive;

import magic_enum;

std::string _convert(std::wstring_view s)
{
	std::string w;
	w.resize(s.size());
	std::transform(s.begin(), s.end(), w.begin(), [](wint_t c) {return static_cast<char>(wctob(c)); });
	return w;
}

template<typename T>
concept Enum = std::is_enum_v<T>;

//template<typename T>
//concept loggable = std::is_enum_v<T> || std::is_arithmetic_v<T> || std::is_same_v<std::string, T> || std::is_same_v<std::wstring, T>;

template <typename T> concept arithmetic = std::is_arithmetic_v<T>;// && !std::is_enum_v<T>;


template <typename T> concept printable = requires( std::ostream &s, const T &t) { s << t;};

export
{

		class simple_log_archive : public cereal::OutputArchive<simple_log_archive>, public cereal::traits::TextArchive
	{
		std::ostream& m_os;
		unsigned int m_depth;
		unsigned int t_depth;
		bool serial = false;
		bool first = true;	
		bool equal = false;
		void new_line()
		{
			if (first){
				first = false;
				return;
			}
			  m_os << std::endl;
			for (unsigned int i = 0; i < m_depth; ++i)
				m_os << "        ";
		}
	public:

		template<Enum T>
		void save(const T& t)
		{
				if(!serial&&!equal) new_line();
			if (serial)	m_os << ", ";
			m_os << magic_enum::enum_name(t);
			serial = true;
		}
		template<class T>
		void save(const cereal::NameValuePair<T>& t);

		template<arithmetic  T>
		void save(const T& t)
		{
			if(!serial&&!equal) new_line();
			if (serial)	m_os << ", ";
			m_os << t;
			serial = true;
		}



		void save(const std::string& t)
		{
			if(!serial&&!equal) new_line();
			if (serial)	m_os << ", ";
			m_os << "\"" << t << "\"";
			serial = true;
		}

		void save(const std::wstring& t)
		{
			save(_convert(t));
			//	m_os << " " << convert(t);
		}
		static constexpr bool PRETTY = true;


		// the << operators
		template<class T>
		simple_log_archive& operator<<(const T& t)
		{
			//	m_os << ' ';
			(*this)(t);
			return *this;
		}

		///////////////////////////////////////////////

		simple_log_archive(std::ostream& os) :
			cereal::OutputArchive<simple_log_archive>(this),
			m_os(os),
			m_depth(0), t_depth(0)
		{}
	};


	template<arithmetic T>
	void serialize(simple_log_archive& archive,
		T& m)
	{
		archive.save(m);
	}


	template<class T>
	void serialize(simple_log_archive& archive,
		cereal::NameValuePair<T>& m)
	{
		if (std::strcmp(m.name, "cereal_class_version") == 0) return;
		archive.save(m);

	}

	template<class T>
	void serialize(simple_log_archive& archive,
		cereal::SizeTag<T>& m)
	{
		//archive.save(m);
	}


	void serialize(simple_log_archive& archive,
		std::wstring& m)
	{
		archive.save(m);
	}

	void serialize(simple_log_archive& archive,
		std::string& m)
	{
		archive.save(m);
	}
	template<typename C, typename T>
	concept can_save = requires(C c, T t)
	{

		c.save(t);
	};

	template<class T>
	void simple_log_archive::save(const cereal::NameValuePair<T>& t)
	{
		// indent according to object depth
		 	new_line();



		if (serial)	m_os << ", ";
		if constexpr (can_save<simple_log_archive, T>)
		{

			m_os << t.name << " = ";
			serial = false;
			equal = true;
			save(t.value);
						 equal = false;
		//	new_line();
		}
		else
		{
			std::string type_name = typeid(T).name();

			if (type_name.rfind("class std::variant", 0) == 0)
				type_name = "std::variant<...>";

			m_os << t.name << " = " << type_name;	new_line();
			m_os << "{" ;
			m_depth++;
		//	new_line();
			

		
			serial = false;
			(*this)(t.value);
			m_depth--;

			new_line();
			m_os << "}";	
		
		}
		serial = true;
	}

	CEREAL_REGISTER_ARCHIVE(simple_log_archive)

	// ---- Macro-free polymorphic caster registration ----
	// Forces registration of a Base→Derived cast path in cereal's BFS table.
	// Prefer CEREAL_REGISTER_POLYMORPHIC_RELATION + CEREAL_FORCE_REGISTER_RELATION
	// in .cpp files for reliable static-init-time registration.
	template<class Base, class Derived>
	inline bool cereal_poly_link() noexcept
	{
		cereal::detail::StaticObject<
			cereal::detail::PolymorphicVirtualCaster<Base, Derived>
		>::getInstance();
		return true;
	}

}