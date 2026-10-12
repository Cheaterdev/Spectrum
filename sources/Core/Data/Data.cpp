module Core:Data;

import zlib;

namespace DataPacker
{

	std::string pack(std::string unpacked)
	{
		std::string packed;
		// The length header is a fixed 32 bits (see Serializer::get_stream); zlib's own lengths
		// are `unsigned long`, so T stays that width and only the stored header is narrowed.
		constexpr size_t header = sizeof(uint32_t);
		packed.resize(int(1.01 * unpacked.size()) + 12 + header);
		unsigned long T = (unsigned long)(packed.size() - header);
		compress((unsigned char *)packed.data() + header, &T, (unsigned char *)unpacked.data(), (unsigned long)unpacked.size());
		packed.resize(T + header);
		const uint32_t unpacked_size = (uint32_t)unpacked.size();
		std::memcpy((void*)packed.data(), &unpacked_size, sizeof(unpacked_size));
		return std::move(packed);
	}

	std::string unpack(std::string str)
	{
		constexpr size_t header = sizeof(uint32_t);
		std::string result;
		unsigned long T = *reinterpret_cast<uint32_t*>(const_cast<char*>(str.data()));
		result.resize(T);
		uncompress((unsigned char *)result.data(), &T, (unsigned char *)str.data() + header, (unsigned long)(str.size() - header));
		return std::move(result);
	}

	std::string load_all(std::istream& s)
	{
		std::string result;
		s.seekg(0, std::ios::end);
		auto size = static_cast<unsigned int>(s.tellg());
		result.resize(size);
		s.seekg(0, std::ios::beg);
		s.read(const_cast<char*>(result.data()), result.size());
		//std::memcpy(reinterpret_cast<char*>(result.data()),s.)
		return std::move(result);
	}

}


MyVariant::var_base::var_base(std::reference_wrapper<const std::type_info> r) : type(r) {}

MyVariant::MyVariant(const MyVariant& r)
{
	if (r.typed)
		typed = r.typed->clone();
}

MyVariant::~MyVariant()
{
	typed.reset();
}

void MyVariant::operator=(MyVariant value)
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


std::strong_ordering operator<=>(const std::string& l, const std::string& r)
{
	if (auto cmp = l.size() <=> r.size(); cmp != 0) return cmp;

	for (size_t i = 0; i < l.size(); i++)
		if (auto cmp = (l[i] <=> r[i]); cmp != 0) return cmp;

	return std::strong_ordering::equal;
}
