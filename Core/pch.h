#pragma once
/*#define CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>*/

#define SPECTRUM_ENABLE_EXEPTIONS
// support windows 7
#include <WinSDKVer.h>
#define _WIN32_WINNT _WIN32_WINNT_WIN7
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
#include <comdef.h>
// BOOST includes
#define BOOST_DECL_EXPORTS

using namespace std;

#pragma warning(disable:4512)
#pragma warning(disable:4100)
#pragma warning(disable:4310)

//#include <boost/move/move.hpp>
#include <boost/iostreams/stream.hpp>

#include <boost/format.hpp>
#include <boost/functional/hash.hpp>
//#include <boost/timer.hpp>
//#include <boost/type_traits/is_enum.hpp>
//#include <boost/mpl/bool.hpp>
//#include <boost/mpl/eval_if.hpp>
//#include <boost/mpl/int.hpp>
//#include <boost/mpl/equal_to.hpp>
//#include <boost/algorithm/string.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#define NVP(name) boost::serialization::make_nvp(BOOST_PP_STRINGIZE(name), name)
#define NP(name, param) boost::serialization::make_nvp(name, param)


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

#include "serialization/portable_iarchive.hpp"
#include "serialization/portable_oarchive.hpp"

#pragma warning(default:4512)
#pragma warning(default:4100)
#pragma warning(default:4310)


typedef  eos::portable_oarchive serialization_oarchive;
typedef  eos::portable_iarchive serialization_iarchive;

typedef  eos::portable_oarchive serialization_portable_oarchive;
typedef  eos::portable_iarchive serialization_portable_iarchive;


#include <boost/system/error_code.hpp>
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.

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
	void insert(T& elem)
	{
		push_back(elem);
	}

	void erase(const T& elem)
	{
		auto it = std::find(begin(), end(), elem);

		if (it != end())
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
#define NOMINMAX
#include <cmath>


#define LESS(x) {if(l.x<r.x) return true; if(r.x<l.x) return false;}
#define OP_E(x)\
	if(!(l.x==r.x)) return false;

// CORE includes
#include "../3rdparty/alllibs.h"

#include "Data/Data.h"
#include "patterns/Singleton.h"
#include "patterns/SharedObject.h"
#include "patterns/EditObject.h"
#include "Patterns/IdGenerator.h"
#include "Patterns/Holdable.h"

#include "Data/md5.h"
#include "Serialization/Serializer.h"

#include "Log/Events.h"

#include "Log/Log.h"
#include "Log/Tasks.h"
#include "DebugInfo/Debug.h"
#include "Tree/Tree.h"

#include "Math/MathX.h"
#include "Math/math_serialization.h"

#include "Input/InputSystem.h"
#include "BinaryObjects.h"
#include "Platform.h"

#include "Threads/Threading.h"

#include "Threads/Scheduler.h"

#include "Application/Application.h"

#define GUID_WINDOWS
#include "guid/guid.h"

#pragma comment(lib, "Core")