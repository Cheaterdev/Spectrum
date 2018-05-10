#include "pch.h"

void com_deleter(IUnknown* pComResource)
{
    if (pComResource)
        pComResource->Release();
}

std::wstring convert(const std::string& s)
{
    /*typedef std::codecvt_utf8<wchar_t> convert_typeX;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.from_bytes(str);*/
    std::wstring w;
    w.resize(s.size());
    std::transform(s.begin(), s.end(), w.begin(), btowc);
    return w;
}

std::string convert(const std::wstring& s)
{
    /*typedef std::codecvt_utf8<wchar_t> convert_typeX;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.to_bytes(wstr);*/
    std::string w;
    w.resize(s.size());
    std::transform(s.begin(), s.end(), w.begin(), wctob);
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
