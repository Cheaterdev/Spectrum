#pragma once
#define BOOST_DECL_EXPORTS
#define BOOST_WARCHIVE_SOURCE
#define BOOST_ARCHIVE_DECL
#include <boost/archive/xml_oarchive.hpp>

class simple_log_archive
{
    std::ostream &m_os;
    unsigned int m_depth;
    unsigned int t_depth;

    template<class Archive>
    struct save_enum_type
    {
        template<class T>
        static void invoke(Archive &ar, const T &t)
        {
            ar.m_os << static_cast<int>(t);
        }
    };
    template<class Archive>
    struct save_primitive
    {
        template<class T>
        static void invoke(Archive &ar, const T &t)
        {
            ar.m_os << t;
        }
    };
    template<class Archive>
    struct save_only
    {
        template<class T>
        static void invoke(Archive &ar, const T &t)
        {
            // make sure call is routed through the highest interface that might
            // be specialized by the user.
            boost::serialization::serialize_adl(
                ar,
                const_cast<T &>(t),
                ::boost::serialization::version<T>::value
            );
        }
    };
    template<class T>
    void save(const T &t)
    {
        typedef
        BOOST_DEDUCED_TYPENAME boost::mpl::eval_if < boost::is_enum<T>,
                               boost::mpl::identity<save_enum_type<simple_log_archive> >,
                               //else
                               BOOST_DEDUCED_TYPENAME boost::mpl::eval_if<
                               // if its primitive
                               boost::mpl::equal_to<
                               boost::serialization::implementation_level<T>,
                               boost::mpl::int_<boost::serialization::primitive_type>
                               >,
                               boost::mpl::identity<save_primitive<simple_log_archive> >,
                               // else
                               boost::mpl::identity<save_only<simple_log_archive> >
                               > > ::type typex;
        typex::invoke(*this, t);
    }
#ifndef BOOST_NO_STD_WSTRING
    void save(const std::wstring & s)
    {
        m_os << convert(s);
    }
#endif

public:
    ///////////////////////////////////////////////////
    // Implement requirements for archive concept

    typedef boost::mpl::bool_<false> is_loading;
    typedef boost::mpl::bool_<true> is_saving;

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
    simple_log_archive &insert(T const &t)
    {
        save(t);
        return *this;
    }


    // the << operators
    template<class T>
    simple_log_archive &operator<<(T const &t)
    {
        m_os << ' ';
        save(t);
        return *this;
    }

    simple_log_archive &operator<<(void* const t)
    {
        m_os << " ->";

        if (NULL == t)
            m_os << " null";
        else
            *this << t;

        return *this;
    }

    template<class T>
    simple_log_archive &operator<<(T* const t)
    {
        m_os << " ->";

        if (NULL == t)
            m_os << " null";
        else
            *this << *t;

        return *this;
    }


    template<class T, int N>
    simple_log_archive &operator<<(const T(&t)[N])
    {
        return *this << boost::serialization::make_array(
                   static_cast<const T*>(&t[0]),
                   N
               );
    }
	/*
    template<class T>
    simple_log_archive &operator<<(const std::vector<T> &v)
    {
        m_os << ' [' << v.size() << "]\n{";

		for (int i = 0; i < v.size(); i++)
		{
		//	m_os << i << ':';
			stringstream s;
			s << i;
			*this << NP(s.str().c_str(),v[i]);
			//m_os << '\n';
		}
		m_os << "}";

        return *this;
    }
	*/

    template<class T>
    simple_log_archive &operator<<(const boost::serialization::nvp<T> &t)
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

    // the & operator
    template<class T>
    simple_log_archive &operator&(const T &t)
    {
        return *this << t;
    }
    ///////////////////////////////////////////////

    simple_log_archive(std::ostream &os) :
        m_os(os),
        m_depth(0), t_depth(0)
    {}
};
