#include "pch.h"


#include "Data.h"
import Logs;

void com_deleter(IUnknown* pComResource)
{
if (pComResource)
pComResource->Release();
}

std::wstring convert(const std::string& s)
{
	std::wstring w;
	w.resize(s.size());
	std::transform(s.begin(), s.end(), w.begin(), btowc);
	return w;
}

std::string convert(const std::wstring& s)
{
	std::string w;
	w.resize(s.size());
	std::transform(s.begin(), s.end(), w.begin(), [](wint_t c) {return static_cast<char>(wctob(c)); });
	return w;
}


HRESULT test(HRESULT hr, std::string str)
{
	if (FAILED(hr))
		Log::get().crash_error(hr, str);

	return hr;
}

std::string to_lower(const std::string& str)
{
	std::string lowered;
	lowered.resize(str.size());
	std::transform(str.begin(),
		str.end(),
		lowered.begin(),
		::tolower);
	return lowered;
}


std::wstring to_lower(const std::wstring& str)
{
	std::wstring lowered;
	lowered.resize(str.size());
	std::transform(str.begin(),
		str.end(),
		lowered.begin(),
		::towlower);
	return lowered;
}
