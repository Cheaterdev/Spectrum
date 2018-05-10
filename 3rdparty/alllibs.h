#undef min
#undef max
#define ZEXTERN
//#include "zlib/zlib.h"

#include <algorithm>
#include "DirectXTex/ScreenGrab/ScreenGrab.h"


#include "ZipLib/ZipFile.h"
#include "ZipLib/streams/memstream.h"

//#include "ZipLib/methods/Bzip2Method.h"

#pragma comment(lib, "3rdparty")
#pragma comment(lib, "ZipLib")
//#pragma comment(lib, "zlib")
//#pragma comment(lib, "bzip2")

//#pragma comment(lib, "lzma")