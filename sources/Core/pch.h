#pragma once
/*#define CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>*/

#define SPECTRUM_ENABLE_EXEPTIONS
// support windows 7
#include <WinSDKVer.h>
//#define _WIN32_WINNT _WIN32_WINNT_WIN7
#include <SDKDDKVer.h>

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN

// WINDOWS includes
#include <windows.h>
#include <windowsx.h>

// STD includes
#include <string>
#include <array>
#include <vector>
#include <typeinfo>
#include <algorithm>
#include <fstream>
#include <map>
#include <any>
#include <unordered_map>
#include <set>
#include <assert.h>
#include <memory>
#include <thread>
#include <sstream>
#include <mutex>
#include <atomic>
#include <type_traits>
#include <queue>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <queue>
#include <optional>
#include <compare>
#include <span>
#include <stack>
#include <ranges>
#include <numeric>
#include <typeindex>
#include <comdef.h>
#include <string>
#include <locale>
#include <codecvt>
#include <cmath>


// BOOST includes
//#define BOOST_DECL_EXPORTS
//#define BOOST_WARCHIVE_SOURCE
//#define BOOST_ARCHIVE_DECL
using namespace std;

#include <ppltasks.h>
using namespace Concurrency;

#pragma warning(disable:4512)
#pragma warning(disable:4100)
#pragma warning(disable:4310)
#include <magic_enum.hpp>

#include <boost/predef/other/endian.h>
#define BOOST_PREDEF_DETAIL_ENDIAN_COMPAT_H

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>


#define NVP(name) boost::serialization::make_nvp(BOOST_PP_STRINGIZE(name), name)
#define NP(name, param) boost::serialization::make_nvp(name, param)

#define GEN_DEF_COMP(x) \
	bool operator==(const x& r) const = default;\
	std::strong_ordering  operator<=>(const  x& r) const = default;



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

#include "serialization/shared_ptr.h"

#pragma warning(default:4512)
#pragma warning(default:4100)
#pragma warning(default:4310)

using serialization_oarchive = boost::archive::binary_oarchive;
using portable_iarchive = boost::archive::binary_iarchive;

using serialization_oarchive = boost::archive::binary_oarchive;
using serialization_iarchive = boost::archive::binary_iarchive;

template<class T> using s_ptr = std::shared_ptr<T>;
template<class T> using w_ptr = std::weak_ptr<T>;

#define STRINGIZE(x) #x
#include <type_traits>
#define USE_MATH_DEFINES




#define LESS(x) {if(l.x<r.x) return true; if(r.x<l.x) return false;}
#define OP_E(x)\
	if(!(l.x==r.x)) return false;

// CORE includes

#include "../3rdparty/ZipLib/ZipFile.h"
#include "../3rdparty/ZipLib/streams/memstream.h"
#define GUID_WINDOWS
HRESULT test(HRESULT hr, std::string str = "");


class serialization_istream
{
	std::string data;
	std::shared_ptr<std::istringstream> stream;
	std::shared_ptr<serialization_iarchive> archive;

public:
	serialization_istream(std::string data, unsigned int offset)
	{
		this->data = data.substr(offset);
		stream.reset(new std::istringstream(this->data, std::ios::binary | std::ios::in));
		//stream->seekg(offset);
		archive.reset(new serialization_iarchive(*stream));
	}

	serialization_istream(std::istream& stream, unsigned int offset)
	{
		stream.seekg(offset, std::ios::beg);
		archive.reset(new serialization_iarchive(stream));
	}
	serialization_istream(std::istream& stream)
	{
		archive.reset(new serialization_iarchive(stream));
	}
	template<class T>
	serialization_istream& operator>>(T& data)
	{
		(*archive) >> data;
		return *this;
	}
};


class serialization_ostream
{
	std::ostringstream stream;
	serialization_oarchive archive;

public:
	serialization_ostream() : stream(std::ios::binary | std::ios::out), archive(stream) {}

	template<class T>
	serialization_ostream& operator<<(const T& data)
	{
		archive << data;
		return *this;
	}

	std::string str()
	{
		return stream.str();
	}
};
