export module Core:FileDataStorage;

import :Utils;
import :Log;
import :Singleton;
import :Scheduler;
import :FileSystem;
import :Holdable;

import stl.filesystem;
import stl.memory;
import windows;

import :Serializer;

export
{
	class FileDataStorage
	{

		std::filesystem::path path;

		struct header
		{
			std::map<std::string, uint64> offsets;

		private:
			SERIALIZE()
			{
				ar& NVP(offsets);
			}
		};

		header h;

		std::shared_ptr<std::fstream> ostream_ptr;
	public:

		FileDataStorage(std::filesystem::path path);

		template<class T>
		void put(std::string partition, const T& data)
		{
			uint64 pos = ostream_ptr->tellp();
			serialization_oarchive oa(*ostream_ptr);
			oa << data;

			h.offsets[partition] = pos;
		}

		void start_save();

		template<class T>
		bool get(std::string partition, T& target)
		{
			std::fstream stream(path, std::ios::binary | std::ios::in);

			if(!stream.is_open()) return false;

			stream.seekg(h.offsets[partition], std::ios::beg);
			UniversalContext context;

			context.get_context<std::fstream*>() = &stream;
			context.get_context<std::filesystem::path>() = path;

			cereal::UserDataAdapter<UniversalContext, serialization_iarchive> oa(context, stream);

			oa >> target;

			stream.close();
			return true;
		}

		template<class T>
		T get(std::string partition)
		{
			T t;
			get(partition, t);
			return t;
		}

		bool has(std::string partition);

		void save();
	};

}
