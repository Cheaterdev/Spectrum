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

		// A save goes to `temp` and takes the file's place in save(), and only if every put() worked.
		bool failed = false;
		std::filesystem::path temp;
		// The table of a staged replace, taken over by commit().
		std::shared_ptr<header> staged;

		void make_temp_name();
		bool stage_append(const std::string& partition, const std::string& bytes);

		// Run by commit() once the new file is in place; dropped if it never gets there.
		std::vector<std::function<void()>> commit_actions;

		// The storage whose put() is serializing on this thread, for writing().
		static FileDataStorage*& current();
	public:

		FileDataStorage(std::filesystem::path path);

		template<class T>
		void put(std::string partition, const T& data)
		{			 			uint64 pos = ostream_ptr->tellp();
			FileDataStorage* const outer = current();
			current() = this;
			try
			{

			serialization_oarchive oa(*ostream_ptr);
			oa << data;
			}
			catch (const std::exception &e)
			{
			 Log::get() << e.what()<<Log::endl;
			 failed = true;
			}
			current() = outer;


			h.offsets[partition] = pos;
		}

		// For an object being serialized by put(): the storage it is going into, or null when it is being serialized
		// some other way. With it the object can note where its bytes land (tell(), the position in the file once
		// saved) and ask to be told when that file is really there (on_commit): a save can still fail, and until it
		// has not, the old file is the one that holds the object's bytes.
		static FileDataStorage* writing() { return current(); }
		uint64 tell() { return static_cast<uint64>(ostream_ptr->tellp()); }
		void on_commit(std::function<void()> action) { commit_actions.push_back(std::move(action)); }

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

		// Writes one partition of an existing file again without serializing the rest: the new copy goes after everything
		// in the file, followed by a new partition table. Nothing already in the file moves, so offsets into it held
		// elsewhere (streamed pages) stay right; the old copy is left as unused bytes until the next whole save. Not
		// with start_save().
		//
		// In two steps, because the file is never written in place (see stage_append): stage() builds the new file beside
		// the old one, which costs a copy of it, and commit() puts it in the old one's place. A caller with handles open
		// on the file closes them between the two. False, with the file as it was, if a step fails.
		template<class T>
		bool stage(std::string partition, const T& data)
		{
			std::ostringstream bytes(std::ios::binary);
			try
			{
				serialization_oarchive oa(bytes);
				oa << data;
			}
			catch (const std::exception& e)
			{
				Log::get() << Log::LEVEL_ERROR << "cannot write " << partition << " of " << path.string() << ": " << e.what() << Log::endl;
				return false;
			}
			return stage_append(partition, bytes.str());
		}

		// The temporary file written by stage() (or, inside save(), by the put() calls) takes the place of the old one.
		bool commit();

		template<class T>
		bool replace(std::string partition, const T& data)
		{
			return stage(partition, data) && commit();
		}

		const std::filesystem::path& get_path() const { return path; }

		// False if anything went wrong; the file at the path is then what it was before start_save().
		bool save();

		// Held exclusively from the moment a saved file takes the old one's place until everything that reads ranges of
		// it has been pointed at the new layout (AssetStorage::save). Code that turns an offset stored in memory into a
		// read of an asset file holds it shared, from looking the offset up until the read is issued: otherwise it can
		// read the new file at an offset from the old one.
		static std::shared_mutex& relocation_mutex();
	};

}
