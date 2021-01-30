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
#include <comdef.h>
// BOOST includes
//#define BOOST_DECL_EXPORTS
//#define BOOST_WARCHIVE_SOURCE
//#define BOOST_ARCHIVE_DECL
using namespace std;

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



constexpr std::size_t operator "" _t(unsigned long long int x)
{
	return x;
}


constexpr std::size_t operator "" _mb(unsigned long long int x) {
	return x * 1024 * 1024;
}

constexpr std::size_t operator "" _gb(unsigned long long int x) {
	return x * 1024 * 1024 * 1024;
}

constexpr std::size_t operator "" _kb(unsigned long long int x) {
	return x * 1024;
}

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



class SpinLock
{
public:
	void lock()
	{
		while (lck.test_and_set(std::memory_order_acquire))
		{
		}
	}

	void unlock()
	{
		lck.clear(std::memory_order_release);
	}
	SpinLock() = default;
	SpinLock(const SpinLock&) {}
	void operator=(const SpinLock&) {}
private:
	std::atomic_flag lck = ATOMIC_FLAG_INIT;
};
#include "serialization/shared_ptr.h"

//#include "serialization/portable_iarchive.hpp"

//#include "serialization/portable_oarchive.hpp"

#pragma warning(default:4512)
#pragma warning(default:4100)
#pragma warning(default:4310)

/*
typedef  eos::portable_oarchive serialization_oarchive;
typedef  eos::portable_iarchive serialization_iarchive;

typedef  eos::portable_oarchive serialization_portable_oarchive;
typedef  eos::portable_iarchive serialization_portable_iarchive;

*/

using serialization_oarchive = boost::archive::binary_oarchive;
using portable_iarchive = boost::archive::binary_iarchive;

using serialization_oarchive = boost::archive::binary_oarchive;
using serialization_iarchive = boost::archive::binary_iarchive;


//#include <boost/system/error_code.hpp>
//#include <boost/uuid/uuid.hpp>            // uuid class
//#include <boost/uuid/uuid_generators.hpp> // generators
//#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.

#include <string>
#include <locale>
#include <codecvt>

void com_deleter(IUnknown* pComResource);

std::wstring convert(const std::string& str);
std::string convert(const std::wstring& wstr);
std::string to_lower(const std::string& str);
std::wstring to_lower(const std::wstring& str);

template<class T> using s_ptr = std::shared_ptr<T>;
template<class T> using w_ptr = std::weak_ptr<T>;



template <class T>
class my_unique_vector : public std::vector<T>
{
public:
	using 	std::vector<T>::insert;

	void insert(const T& elem)
	{
		std::vector<T>::push_back(elem);
	}
	void erase(const T& elem)
	{
		auto it = std::find(std::vector<T>::begin(), std::vector<T>::end(), elem);

		if (it != std::vector<T>::end())
			std::vector<T>::erase(it);
	}
};


HRESULT test(HRESULT hr, std::string str = "");

template <class T>
void unreferenced_parameter(const T&)
{
};
#define STRINGIZE(x) #x
#include <type_traits>
#define USE_MATH_DEFINES

#include <cmath>


#define LESS(x) {if(l.x<r.x) return true; if(r.x<l.x) return false;}
#define OP_E(x)\
	if(!(l.x==r.x)) return false;

// CORE includes

#include "../3rdparty/ZipLib/ZipFile.h"
#include "../3rdparty/ZipLib/streams/memstream.h"
#include "DebugInfo/Exceptions.h"
#include "Data/Data.h"
#include "patterns/Singleton.h"
#include "patterns/SharedObject.h"
#include "patterns/EditObject.h"
#include "Patterns/IdGenerator.h"
#include "Patterns/Holdable.h"

#include "Log/Events.h"
#include "Log/Log.h"

#include "Serialization/Serializer.h"
#include "Data/md5.h"




#include "Log/Tasks.h"
#include "DebugInfo/Debug.h"
#include "Tree/Tree.h"

#include "Math/Constants.h"
#include "Math/Vectors.h"
#include "Math/Quaternions.h"
#include "Math/Matrices.h"

#include "Math/GeometryPrimitives.h"


#include "Math/math_serialization.h"

#include "BinaryObjects.h"
#include "Platform.h"

#include "Threads/Threading.h"

#include "Threads/Scheduler.h"

#include "Application/Application.h"
#include "Profiling/Profiling.h"

#define GUID_WINDOWS
#include "guid/guid.h"
#include "Allocators/Allocators.h"


#include "patterns/Singleton.hpp"