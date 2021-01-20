#include "pch.h"
import Utils;

import Logs;
import Singletons;
import EventsProps;

HRESULT test(HRESULT hr, std::string str)
{
	if (FAILED(hr))
		Log::get().crash_error(hr, str);

	return hr;
}
