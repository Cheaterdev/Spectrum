#include "pch_dx.h"
import Log;
HRESULT test(HRESULT hr, std::string str)
{
		if (FAILED(hr))
			Log::get().crash_error(hr, str);

	return hr;
}
