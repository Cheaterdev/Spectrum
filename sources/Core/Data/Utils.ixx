#include "pch.h"
export module Utils;

export
{

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



	void com_deleter(IUnknown* pComResource);

	std::wstring convert(const std::string& str);
	std::string convert(const std::wstring& wstr);
	std::string to_lower(const std::string& str);
	std::wstring to_lower(const std::wstring& str);


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



	template <class T>
	void unreferenced_parameter(const T&)
	{
	};

}


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
