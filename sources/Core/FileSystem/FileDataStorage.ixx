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
		{			 			uint64 pos = ostream_ptr->tellp();
			try
			{
				  
			serialization_oarchive oa(*ostream_ptr);
			oa << data;
			}
			catch (const std::exception &e)
			{
			 Log::get() << e.what()<<Log::endl;				
			}


			h.offsets[partition] = pos;
		}

		void start_save();

		// base_context lets callers pre-populate the UniversalContext before the
		// archive is opened (e.g. push Device*, RenderContext*, etc.).
		// File-specific entries (fstream*, path) are always set here, overwriting
		// any stale values the caller may have left in those slots.
		template<class T>
		bool get(std::string partition, T& target, UniversalContext* base_context = nullptr)
		{
			std::fstream stream(path, std::ios::binary | std::ios::in);

			if(!stream.is_open()) return false;

			stream.seekg(h.offsets[partition], std::ios::beg);

			UniversalContext local;
			UniversalContext& context = base_context ? *base_context : local;

			context.get_context<std::fstream*>() = &stream;
			context.get_context<std::filesystem::path>() = path;

			cereal::UserDataAdapter<UniversalContext, serialization_iarchive> oa(context, stream);

			oa >> target;

			stream.close();
			return true;
		}

		template<class T>
		T get(std::string partition, UniversalContext* base_context = nullptr)
		{
			T t;
			get(partition, t, base_context);
			return t;
		}

		bool has(std::string partition);

		void save();
	};

}
