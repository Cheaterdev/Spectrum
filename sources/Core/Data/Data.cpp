#include "pch.h"


#include "Data.h"
namespace DataPacker
{

	std::string pack(std::string unpacked)
	{
		std::string packed;
		packed.resize(int(1.01 * unpacked.size()) + 12 + sizeof(unsigned long));
		unsigned long T = (unsigned long)packed.size() - sizeof(unsigned long);
		compress((Bytef*)packed.data() + sizeof(unsigned long), &T, (Bytef*)unpacked.data(), (unsigned long)unpacked.size());
		packed.resize(T + sizeof(unsigned long));
		T = (unsigned long)unpacked.size();
		memcpy((void*)packed.data(), &T, sizeof(T));
		return std::move(packed);
	}

	std::string unpack(std::string str)
	{
		std::string result;
		unsigned long T = *reinterpret_cast<unsigned long*>(const_cast<char*>(str.data()));
		result.resize(T);
		uncompress((Bytef*)result.data(), &T, (Bytef*)str.data() + sizeof(unsigned long), (unsigned long)str.size() - sizeof(unsigned long));
		return std::move(result);
	}


	 ZipArchiveEntry::Ptr create_entry(ZipArchive::Ptr archive, std::string name, std::string data, bool pack)
	{
		ZipArchiveEntry::Ptr entry = archive->CreateEntry(name);
		std::istringstream sss(data);

		if (pack)
			entry->SetCompressionStream(sss, DeflateMethod::Create(), ZipArchiveEntry::CompressionMode::Immediate);
		else
			entry->SetCompressionStream(sss, StoreMethod::Create(), ZipArchiveEntry::CompressionMode::Immediate);

		return entry;
	}


	 std::string zip_to_string(ZipArchive::Ptr archive)
	{
		std::ostringstream s;
		archive->WriteToStream(s);
		return s.str();
	}

	std::string load_all(istream& s)
	{
		std::string result;
		s.seekg(0, ios::end);
		auto size = UINT(s.tellg());
		result.resize(size);
		s.seekg(0, ios::beg);
		s.read(const_cast<char*>(result.data()), result.size());
		//memcpy(reinterpret_cast<char*>(result.data()),s.)
		return std::move(result);
	}


};



MyVariant::var_base::var_base(std::reference_wrapper<const std::type_info> r) : type(r) {}
	


MyVariant::MyVariant(const MyVariant& r)
	{
	if(r.typed)
		typed = r.typed->clone();
	}

MyVariant::~MyVariant()
	{
		typed.reset();
	}




	void MyVariant::operator = (MyVariant value)
	{
		typed = value.typed->clone();

		if (!typed)
			typed = nullptr;
	}

	bool MyVariant::exists()
	{
		return !!typed;
	}
	void MyVariant::clear()
	{
		typed.reset();
	}



	std::strong_ordering operator<=>(const std::string& l, const std::string& r) {

		if (auto cmp = l.size() <=> r.size(); cmp != 0) return cmp;


		for (size_t i = 0; i < l.size(); i++)
			if (auto cmp = (l[i] <=> r[i]); cmp != 0) return cmp;

		return  std::strong_ordering::equal;
	}


#include <Log/Log.h>
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
