export module Core:Serializer;

export import :serialization;
using serialization_exception = cereal::Exception;
import :Log;
import :Utils;
import :crc32;

import :Data;
import windows;
//import boost.archives;

template<class Archive>
class SerializationArchive: public Archive
{
public:

	void* user_ptr = nullptr;
	template <class ... Args>
	SerializationArchive(Args && ... args) :
		Archive(std::forward<Args>(args)...)
	{ }
};

using iarchive = serialization_iarchive;
using oarchive = serialization_oarchive;


export
{

class Serializer
{

public:

	class serialization_istream
	{
		std::string data;
		std::shared_ptr<std::istringstream> stream;
		std::shared_ptr<iarchive> archive;

	public:
		serialization_istream(std::string data, unsigned int offset)
		{
			this->data = data.substr(offset);
			stream.reset(new std::istringstream(this->data, std::ios::binary | std::ios::in));
			//stream->seekg(offset);
			archive.reset(new iarchive(*stream));
		}

		serialization_istream(std::istream & stream, unsigned int offset)
		{
			stream.seekg(offset, std::ios::beg);
			archive.reset(new iarchive(stream));
		}
		serialization_istream(std::istream & stream)
		{
			archive.reset(new iarchive(stream));
		}
		template<class T>
		serialization_istream& operator>>(T & data)
		{
			(*archive) >> data;
			return *this;
		}
	};


	class serialization_ostream
	{
		std::ostringstream stream;
		oarchive archive;

	public:
		serialization_ostream() : stream(std::ios::binary | std::ios::out), archive(stream) {}

		template<class T>
		serialization_ostream& operator<<(const T& data)
		{
			archive << data;
			return *this;
		}

		std::string str()
		{
			return stream.str();
		}
	};



	static std::string serialize(serialization_ostream& stream, bool compress = false)
	{
		std::string result;

		if (compress)
			result = DataPacker::pack(stream.str());
		else
		{
			result.resize(sizeof(unsigned long), 0);
			result += stream.str();
		}

		return result;
	}

	template<class T>
	static std::string serialize(const T& object, bool compress = false)
	{
		std::ostringstream stream(std::ios::binary | std::ios::out);
		oarchive oa(stream);

		try
		{
			oa << object;
		}

		catch (const serialization_exception& e)
		{
			Log::get().crash_error("Serialization", e.what());
			return "";
		}

		std::string result;

		if (compress)
			result = DataPacker::pack(stream.str());
		else
		{
			result.resize(sizeof(unsigned long), 0);
			result += stream.str();
		}

		return result;
	}

	template<class T>
	static std::string serialize_simple(const T& object)
	{
		std::ostringstream stream(std::ios::binary | std::ios::out);
		oarchive oa(stream);

		try
		{
			oa << object;
		}

		catch (const serialization_exception& e)
		{
			Log::get().crash_error("Serialization", e.what());
			return "";
		}

		return stream.str();
	}

	template<class T>
	static std::shared_ptr<T> deserialize(const std::string& data)
	{
		if (data.size() < sizeof(unsigned long))
			return std::shared_ptr<T>();

		try
		{
			std::shared_ptr<T> obj(new T());
			get_stream(data) >> *obj;
			return obj;
		}

		catch (const serialization_exception& e)
		{
			Log::get().crash_error("Serialization", e.what());
		}

		return std::shared_ptr<T>();
	}

	template<class T>
	static std::shared_ptr<T> deserialize(std::istream& s)
	{
		try
		{
			std::shared_ptr<T> obj(new T());
			get_stream(s) >> *obj;
			return obj;
		}

		catch (const serialization_exception& e)
		{
			Log::get().crash_error("Serialization", e.what());
		}

		return std::shared_ptr<T>();
	}


	template<class T>
	static void deserialize(const std::string& data, T& obj)
	{
		if (data.size() < sizeof(unsigned long))
			return;

		try
		{
			get_stream(data) >> obj;
		}

		catch (const serialization_exception& e)
		{
			Log::get().crash_error("Serialization", e.what());
		}
	}

	template<class T>
	static void deserialize_simple(std::istream& data, T& obj)
	{
		//    if (data.size() < sizeof(unsigned long))
//return;
		try
		{
			get_stream(data) >> obj;
		}

		catch (const serialization_exception& e)
		{
			Log::get().crash_error("Serialization", e.what());
		}
	}
	template<class T>
	static std::shared_ptr<T> deserialize_ptr(const std::string& data)
	{
		if (data.size() < sizeof(unsigned long))
			return std::shared_ptr<T>();

		try
		{
			std::shared_ptr<T> obj;
			get_stream(data) >> obj;
			return obj;
		}

		catch (const serialization_exception& e)
		{
			Log::get().crash_error("Serialization", e.what());
		}

		return std::shared_ptr<T>();
	}

	static serialization_istream get_stream(const std::string& data)
	{
		unsigned long S = *reinterpret_cast<unsigned long*>(const_cast<char*>(data.data()));

		if (S)
			return serialization_istream(DataPacker::unpack(data), 0);

		return serialization_istream(data, sizeof(unsigned long));
	}
	static serialization_istream get_stream(std::istream & stream)
	{
		unsigned long S = 0;
		stream.read(reinterpret_cast<char*>(&S), sizeof(S));

		return serialization_istream(stream);
	}


	template<class T>
	static std::shared_ptr<T> clone(const T & obj)
	{
		return deserialize<T>(serialize(obj));
	}
};

template<class T>
std::string to_string(const T& elem)
{
	std::stringstream s;
	s << elem;
	return s.str();
}


template<class T>
class BinaryData
{
	std::shared_ptr<T> data;
	std::string packed_data;
private:
	SERIALIZE()
	{
		if (data&& Archive::is_saving::value)
			packed_data = Serializer::serialize(*data);

		ar & NVP(packed_data);
	}

public:
	std::function<void(std::shared_ptr<T>)> on_create;
	BinaryData() = default;
	BinaryData(const std::shared_ptr<T> & _data) : data(_data) {}


	void test()
	{
		if (data)
			packed_data = Serializer::serialize(data);

		data = Serializer::deserialize_ptr<T>(packed_data);
		//  packed_data.clear();
	}

	std::shared_ptr<T> get()
	{
		if (!data)
		{
			data = Serializer::deserialize<T>(packed_data);

			if (data)
			{
				packed_data.clear();

				if (on_create)
					on_create(data);
			}
		}

		/*   else
		   {
			   packed_data = Serializer::serialize(*data);
			   data = Serializer::deserialize<T>(packed_data);
		   }
		*/
		return data;
	}
};


class Hasher
{
public:

	static std::string hash(std::string_view str)
	{
		return to_string(crc32(str));
	}

	template<NonString T>
	static std::string hash(const T& obj)
	{
		return hash(Serializer::serialize(obj));
	}

};

}