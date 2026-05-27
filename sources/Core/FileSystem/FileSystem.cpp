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

void native_file_provider::on_change(const std::filesystem::path& path, std::function<void()> f)
{
	// this function is completely written by copilot

	auto watch_path = path;
	bool is_dir = false;
	std::filesystem::path monitored_parent;
	std::filesystem::path monitored_filename;

	std::error_code ec;
	if (std::filesystem::is_directory(watch_path, ec) && !ec)
	{
		is_dir = true;
		monitored_parent = watch_path;
	}
	else
	{
		monitored_parent = watch_path.parent_path();
		monitored_filename = watch_path.filename();
	}

	// initialize last write time
	std::filesystem::file_time_type last_time = std::filesystem::file_time_type::min();
	if (!is_dir && !monitored_parent.empty())
	{
		auto full = monitored_parent / monitored_filename;
		std::error_code ec2;
		last_time = std::filesystem::last_write_time(full, ec2);
		if (ec2)
			last_time = std::filesystem::file_time_type::min();
	}

	// Use thread pool to run a lightweight polling watcher.
	thread_pool::get().enqueue([monitored_parent, monitored_filename, is_dir, f, last_time]()
	{
		using namespace std::chrono;
		using namespace std::chrono_literals;

		std::filesystem::file_time_type local_last = last_time;
		std::error_code ec_inner;

		while (Application::is_good())
		{
			try
			{
				if (is_dir)
				{
					// For directory watches: scan entries and detect any file modification.
					for (auto& entry : std::filesystem::directory_iterator(monitored_parent, ec_inner))
					{
						if (ec_inner)
							break;

						if (!entry.is_regular_file())
							continue;

						std::error_code ec_time;
						auto t = std::filesystem::last_write_time(entry.path(), ec_time);
						if (ec_time)
							continue;

						if (t != local_last)
						{
							local_last = t;
							std::this_thread::sleep_for(100ms); // debounce / allow file to become accessible
							try { f(); } catch (...) { Log::get() << Log::LEVEL_ERROR << "on_change callback threw" << Log::endl; }
							// after detecting one change, break to avoid multiple callbacks in the same loop
							break;
						}
					}
				}
				else
				{
					// For single-file watches: poll that file only.
					auto full = monitored_parent / monitored_filename;
					std::error_code ec_time;
					auto t = std::filesystem::last_write_time(full, ec_time);
					if (!ec_time && t != local_last)
					{
						local_last = t;
						std::this_thread::sleep_for(100ms); // debounce
						try { f(); } catch (...) { Log::get() << Log::LEVEL_ERROR << "on_change callback threw" << Log::endl; }
					}
				}
			}
			catch (const std::exception& ex)
			{
				Log::get() << Log::LEVEL_ERROR << "on_change exception: " << ex.what() << Log::endl;
			}
			catch (...)
			{
				Log::get() << Log::LEVEL_ERROR << "on_change unknown exception" << Log::endl;
			}

			std::this_thread::sleep_for(200ms); // polling interval
		}
	});
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
