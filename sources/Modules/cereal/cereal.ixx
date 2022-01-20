export module cereal;

export import "cereal.h";

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

export
{

	template<typename T>
	concept loggable = std::is_enum_v<T> || std::is_arithmetic_v<T> || std::is_same_v<std::string, T> || std::is_same_v<std::wstring, T>;

	template<typename T>
	concept Enum = std::is_enum_v<T>;
	template <typename T> concept arithmetic = std::is_arithmetic_v<T>;// && !std::is_enum_v<T>;

	class simple_log_archive : public cereal::OutputArchive<simple_log_archive>, public cereal::traits::TextArchive
	{
		std::ostream& m_os;
		unsigned int m_depth;
		unsigned int t_depth;
		bool serial = false;
	public:

		template<Enum T>
		void save(const T& t)
		{
			if (serial)	m_os << ", ";
			m_os <<magic_enum::enum_name(t);
			serial = true;
		}
		template<class T>
		void save(const cereal::NameValuePair<T>& t);

		template<arithmetic T>
		void save(const T& t)
		{
			if (serial)	m_os << ", ";
			m_os <<t;
			serial = true;
		}



		void save(const std::string& t)
		{
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
		if (strcmp(m.name, "cereal_class_version") == 0) return;
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
	concept can_save = requires(C c,T t)
	{
	
	c.save(t);
	};

	template<class T>
	void simple_log_archive::save(const cereal::NameValuePair<T>& t)
	{
		// indent according to object depth
		for (unsigned int i = 0; i < m_depth; ++i)
			m_os << ' ';
		if(serial)	m_os << ", ";
		if constexpr (can_save<simple_log_archive, T>)
		{
			m_os << t.name << " = ";
			serial = false;
			save(t.value);
			m_os << std::endl;
		}
		else
		{
			m_os << t.name << " = {" << std::endl;
			m_depth++;

			serial = false;
			(*this)(t.value);
			m_depth--;

			for (unsigned int i = 0; i < m_depth; ++i)
				m_os << ' ';
			m_os << "}" << std::endl;
		}
		serial = true;
	}

	CEREAL_REGISTER_ARCHIVE(simple_log_archive)

}