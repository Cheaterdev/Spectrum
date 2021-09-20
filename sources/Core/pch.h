#pragma once

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
#include <locale>
#include <codecvt>
#include <cmath>
#include <ranges>
using namespace std;

namespace ranges = std::ranges;
namespace view = ranges::views;

#include <magic_enum.hpp>

#include <boost/predef/other/endian.h>
#define BOOST_PREDEF_DETAIL_ENDIAN_COMPAT_H

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

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


#include "../3rdparty/ZipLib/ZipFile.h"
#include "../3rdparty/ZipLib/streams/memstream.h"



#define NVP(name) boost::serialization::make_nvp(BOOST_PP_STRINGIZE(name), name)
#define NP(name, param) boost::serialization::make_nvp(name, param)

#define SAVE_PARENT(type) boost::serialization::make_nvp("parent", boost::serialization::base_object<type>(*this))
#define SERIALIZE() friend class boost::serialization::access; template<class Archive> void serialize(Archive& ar, const unsigned int version)

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

using serialization_oarchive = boost::archive::binary_oarchive;
using portable_iarchive = boost::archive::binary_iarchive;

using serialization_oarchive = boost::archive::binary_oarchive;
using serialization_iarchive = boost::archive::binary_iarchive;



std::wstring convert(std::string_view str);
std::string convert(std::wstring_view wstr);
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

#define STRINGIZE(x) #x

// CORE includes

#include "DebugInfo/Exceptions.h"
#include "Data/Data.h"
#include "patterns/Singleton.h"
#include "patterns/SharedObject.h"
#include "patterns/EditObject.h"

#include "Patterns/Holdable.h"

#include "Log/Events.h"
#include "Log/Log.h"

#include "Serialization/Serializer.h"
#include "Data/md5.h"




#include "Log/Tasks.h"
#include "DebugInfo/Debug.h"
#include "Tree/Tree.h"

#include "Math/Constants.h"
#include "Math/Types/Vectors.h"
#include "Math/Types/Quaternions.h"
#include "Math/Types/Matrices.h"

#include "Math/Primitives/Intersections.h"


#include "Math/math_serialization.h"

#include "BinaryObjects.h"
#include "Platform.h"

#include "Threads/Threading.h"

#include "Threads/Scheduler.h"
#include "Patterns/IdGenerator.h"
#include "Application/Application.h"
#include "Profiling/Profiling.h"
#include "Patterns/StateContext.h"
#define GUID_WINDOWS
#include "guid/guid.h"
#include "Allocators/Allocators.h"


#include "patterns/Singleton.hpp"