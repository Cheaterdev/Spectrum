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
#include <unordered_set>
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
#include <any>
#include <atomic>
#include <typeindex>
using namespace std;

namespace ranges = std::ranges;
namespace view = ranges::views;

#include "crc32c/crc32c.h"

#include <crossguid/guid.hpp>

using Guid = xg::Guid;

#include <magic_enum.hpp>

#include <boost/stacktrace.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/predef/other/endian.h>
#define BOOST_PREDEF_DETAIL_ENDIAN_COMPAT_H


#include "ZipFile.h"
#include "streams/memstream.h"

#include <ppltasks.h>
#include <ppl.h>
#include <agents.h>


HRESULT test(HRESULT hr, std::string str = "");


// CORE includes

#include "Utils/utils.h"

#include "Debug/Exceptions.h"

#include "Serialization/serialization.h"


#include "Data/Data.h"
#include "patterns/SharedObject.h"
#include "patterns/EditObject.h"
#include "Patterns/Holdable.h"

#include "Log/Log.h"

#include "Serialization/Serializer.h"


#include "Events/Tasks.h"



#include "Math/Math.h"

#include "Debug/Debug.h"
#include "Data/BinaryObjects.h"



