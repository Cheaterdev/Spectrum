module Core:FileSystem;

import stl.core;
import stl.filesystem;
import :Log;
import :Utils;
import :Scheduler;
import :Application;
import windows;

// ---- file -------------------------------------------------------------------

file::file(file_provider* _provider, std::filesystem::path _file_name)
	: provider(_provider), file_name(_file_name)
{
}

std::shared_ptr<std::istream> file::get_new_stream()
{
	return provider->create_stream(file_name);
}

std::string file::load_all()
{
	return provider->load_all(this);
}

// ---- FileSystem -------------------------------------------------------------

void FileSystem::register_provider(file_provider::ptr provider)
{
	providers.push_back(provider);
}

std::shared_ptr<file> FileSystem::get_file(std::filesystem::path name)
{
	std::shared_ptr<file> result;

	for (decltype(providers.size()) i = 0; i < providers.size(); i++)
	{
		result = providers[i]->get_file(name);

		if (result)
			break;
	}

	return result;
}

bool FileSystem::save_data(std::filesystem::path file_name, std::string data)
{
	for (auto& p : providers)
	{
		if (p->save_data(file_name, data))
			return true;
	}

	return false;
}

void FileSystem::iterate(std::filesystem::path path, std::function<void(file::ptr)> f, bool recursive)
{
	for (auto& p : providers)
		p->iterate(path, f, recursive);
}

void FileSystem::iterate_dirs(std::filesystem::path path, std::function<void(std::filesystem::path)> f, bool recursive)
{
	for (auto& p : providers)
		p->iterate_dirs(path, f, recursive);
}

// ---- native_file_provider ---------------------------------------------------

native_file_provider::~native_file_provider()
{
}

std::string native_file_provider::load_all(file* info)
{
	std::string result;

	std::filesystem::path file_path(info->file_name);
	std::filesystem::path abs_path = std::filesystem::absolute(file_path);

	std::ifstream file(abs_path.c_str(), std::ios::in | std::ios::binary);

	if (!file.is_open())
	{
		char data[256];
		strerror_s(data, 256, current_errno());
		Log::get() << "No file found: " << info->file_name << " " << data << Log::endl;
		return result;
	}

	std::streampos current_position = file.tellg();
	file.seekg(0, std::ios::end);
	std::streampos ending_position = file.tellg();
	file.seekg(current_position);
	result.resize(static_cast<size_t>(ending_position - current_position));
	file.read(const_cast<char*>(result.data()), result.size());
	file.close();
	return result;
}

// A single dedicated thread polls every registered file/dir for mtime changes.
// Previously each on_change() spun up its own infinite-loop task on the shared
// thread_pool; with enough watchers that exhausts the pool and later watchers
// (and any other pool work) never start. One thread scales to any count.
namespace
{
	struct FileWatcher
	{
		struct Entry
		{
			std::filesystem::path            parent;
			std::filesystem::path            filename;   // empty ⇒ directory watch
			bool                             is_dir = false;
			std::function<void()>            callback;
			std::filesystem::file_time_type  last = std::filesystem::file_time_type::min();
		};

		std::mutex          mutex;
		std::vector<Entry>  entries;
		std::atomic<bool>   started{ false };

		static FileWatcher& get()
		{
			static FileWatcher* instance = new FileWatcher(); // intentionally leaked: outlives statics
			return *instance;
		}

		void add(const std::filesystem::path& path, std::function<void()> f)
		{
			Entry e;
			e.callback = std::move(f);

			std::error_code ec;
			if (std::filesystem::is_directory(path, ec) && !ec)
			{
				e.is_dir = true;
				e.parent = path;
			}
			else
			{
				e.parent   = path.parent_path();
				e.filename = path.filename();

				std::error_code ec2;
				auto t = std::filesystem::last_write_time(e.parent / e.filename, ec2);
				if (!ec2)
					e.last = t;
			}

			Log::get() << "watching " << path << Log::endl;

			{
				std::lock_guard<std::mutex> g(mutex);
				entries.push_back(std::move(e));
			}

			// Lazily start the single polling thread on first registration.
			if (!started.exchange(true))
				std::thread(&FileWatcher::run, this).detach();
		}

		void run()
		{
			using namespace std::chrono_literals;

			// NOTE: watchers are registered from inside the Application constructor,
			// which runs *before* Singleton sets its ptr — so is_good() is false at
			// startup. Don't exit on that transient false: only stop once the app has
			// come up and then been torn down (is_good() false again == real shutdown).
			bool ever_good = false;

			while (true)
			{
				bool good = Application::is_good();
				if (good)
					ever_good = true;
				else if (ever_good)
					break; // app shut down

				if (!good)
				{
					std::this_thread::sleep_for(200ms); // still starting up — idle
					continue;
				}

				std::vector<std::function<void()>> fire;

				{
					std::lock_guard<std::mutex> g(mutex);
					for (auto& e : entries)
					{
						std::error_code ec;
						if (e.is_dir)
						{
							// Directory watch: fire if any regular file changed mtime.
							auto newest = e.last;
							bool changed = false;
							for (auto& it : std::filesystem::directory_iterator(e.parent, ec))
							{
								if (ec) break;
								if (!it.is_regular_file()) continue;
								std::error_code ect;
								auto t = std::filesystem::last_write_time(it.path(), ect);
								if (ect) continue;
								if (t > newest) { newest = t; changed = true; }
							}
							if (changed) { e.last = newest; fire.push_back(e.callback); }
						}
						else
						{
							auto t = std::filesystem::last_write_time(e.parent / e.filename, ec);
							if (!ec && t != e.last) { e.last = t; fire.push_back(e.callback); }
						}
					}
				}

				// Fire outside the lock so a callback may register new watches.
				for (auto& cb : fire)
				{
					try { cb(); }
					catch (const std::exception& ex) { Log::get() << Log::LEVEL_ERROR << "on_change callback: " << ex.what() << Log::endl; }
					catch (...) { Log::get() << Log::LEVEL_ERROR << "on_change callback threw" << Log::endl; }
				}

				std::this_thread::sleep_for(200ms); // polling interval
			}
		}
	};
}

void native_file_provider::on_change(const std::filesystem::path& path, std::function<void()> f)
{
	FileWatcher::get().add(path, std::move(f));
}

std::shared_ptr<file> native_file_provider::get_file(std::filesystem::path file_name)
{
	std::shared_ptr<file> result;
	std::error_code ec;

	if (std::filesystem::exists(file_name, ec))
	{
		result.reset(new file(this, file_name));
		result->edit_time = std::filesystem::last_write_time(file_name);
	}

	if (ec)
		Log::get() << Log::LEVEL_ERROR << "file: " << file_name << " msg:" << ec.message() << Log::endl;

	return result;
}

std::shared_ptr<std::istream> native_file_provider::create_stream(std::filesystem::path file_name)
{
	return std::shared_ptr<std::istream>(new std::ifstream(file_name.generic_wstring(), std::ios::binary), [](std::istream* str) {
		((std::ifstream*)str)->close();
		delete str;
	});
}

bool native_file_provider::save_data(std::filesystem::path file_name, std::string data)
{
	std::filesystem::path dir(file_name);
	std::filesystem::path parent_dir = dir.parent_path();

	if (!parent_dir.empty())
		std::filesystem::create_directories(parent_dir);

	std::ofstream file;
	file.open(file_name.generic_wstring(), std::ios::out | std::ios::binary);

	if (!file.is_open())
	{
		Log::get() << Log::LEVEL_ERROR << "wtf" << Log::endl;
		return false;
	}

	file.write(data.data(), data.size());
	file.close();
	return true;
}

void native_file_provider::iterate(std::filesystem::path path, std::function<void(file::ptr)> f, bool recursive)
{
	if (recursive)
	{
		using namespace std::filesystem;
		std::error_code ec;
		recursive_directory_iterator dir(path, ec), end;

		if (!ec)
			while (dir != end)
			{
				if (!is_directory(*dir))
					f(get_file(dir->path().wstring()));

				++dir;
			}
		else
			Log::get() << Log::LEVEL_DEBUG << ec.message() << Log::endl;
	}
	else
	{
		using namespace std::filesystem;
		std::error_code ec;
		directory_iterator dir(path, ec), end;

		if (!ec)
			while (dir != end)
			{
				if (!is_directory(*dir))
					f(get_file(dir->path().wstring()));

				++dir;
			}
		else
			Log::get() << Log::LEVEL_DEBUG << ec.message() << Log::endl;
	}
}

void native_file_provider::iterate_dirs(std::filesystem::path path, std::function<void(std::filesystem::path)> f, bool recursive)
{
	if (recursive)
	{
		using namespace std::filesystem;
		std::error_code ec;
		recursive_directory_iterator dir(path, ec), end;

		if (!ec)
			while (dir != end)
			{
				if (is_directory(*dir))
					f(dir->path());

				++dir;
			}
		else
			Log::get() << Log::LEVEL_DEBUG << ec.message() << Log::endl;
	}
	else
	{
		using namespace std::filesystem;
		std::error_code ec;
		directory_iterator dir(path, ec), end;

		if (!ec)
			while (dir != end)
			{
				if (is_directory(*dir))
					f(dir->path().wstring());

				++dir;
			}
		else
			Log::get() << Log::LEVEL_DEBUG << ec.message() << Log::endl;
	}
}
