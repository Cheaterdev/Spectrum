module;
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>

export module serialization;

export
{

		namespace serialization
		{
			using boost::serialization::access;
			using boost::serialization::make_nvp;
			using boost::serialization::base_object;

		}
	
}

export template<typename T> concept PrettyArchive = requires () { T::PRETTY; };
