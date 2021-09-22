#include "pch.h"
#include "utils.h"

std::wstring convert(std::string_view s)
{
    std::wstring w;
    w.resize(s.size());
    std::transform(s.begin(), s.end(), w.begin(), btowc);
    return w;
}

std::string convert(std::wstring_view s)
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

std::string to_lower(std::string_view str)
{
    std::string lowered;
    lowered.resize(str.size());
    std::transform(str.begin(),
        str.end(),
        lowered.begin(),
        ::tolower);
    return lowered;
}

std::wstring to_lower(std::wstring_view str)
{
    std::wstring lowered;
    lowered.resize(str.size());
    std::transform(str.begin(),
        str.end(),
        lowered.begin(),
        ::towlower);
    return lowered;
}
