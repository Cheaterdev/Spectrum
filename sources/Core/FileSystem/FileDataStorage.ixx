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

		FileDataStorage(std::filesystem::path path):path(path)
		{
			std::fstream stream(path,std::ios::binary | std::ios::in);

			if(!stream.is_open()) return;

		
					  stream.seekg( 0, std::ios::end );
					  uint64 file_size = stream.tellg();

					  stream.seekg( file_size-sizeof(uint64), std::ios::beg );

				uint64 header_offset = 0 ;

				stream.read(reinterpret_cast<char*>(&header_offset), sizeof(uint64));

				stream.seekg( header_offset, std::ios::beg );

				serialization_iarchive oa(stream);

				oa>>h;
				stream.close();
			
		}


		template<class T>
		void put(std::string partition, const T&data)
		{

			uint64 pos = ostream_ptr->tellp();
			serialization_oarchive oa(*ostream_ptr);
			oa << data;

			h.offsets[partition] = pos;
		}
		void start_save()
		{

			std::filesystem::create_directories(path.parent_path());
			ostream_ptr = std::make_shared<std::fstream>(path,std::ios::binary|std::ios::out);
		}

			template<class T>
		void get(std::string partition, T& target)
		{
	
			std::fstream stream(path,std::ios::binary | std::ios::in);
			stream.seekg( h.offsets[partition], std::ios::beg );
			UniversalContext context;

			context.get_context<std::fstream*>() = &stream;
			context.get_context<std::filesystem::path>() = path;



			cereal::UserDataAdapter<UniversalContext, serialization_iarchive> oa(context,stream);
		
			oa>>target;

			stream.close();

		}
		template<class T>
		T get(std::string partition)
		{
			T t;
			get(partition,t);
			return t;
		}

		bool has(std::string partition)
		{
			return h.offsets.find(partition) != h.offsets.end();
		}

		void save()
		{
			uint64 pos = ostream_ptr->tellp();
			serialization_oarchive oa(*ostream_ptr);
			oa << h;
			(*ostream_ptr).write(reinterpret_cast<char*>(&pos), sizeof(uint64));
			ostream_ptr->close();
			ostream_ptr = nullptr;
		}
	};

}