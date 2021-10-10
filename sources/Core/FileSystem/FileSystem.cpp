#include "pch.h"
#include <io.h>
#include "Threads/Scheduler.h"
#include "FileSystem.h"

import Utils;
import stl.core;



std::string native_file_provider::load_all(file* info)
{
    std::string result;

	std::filesystem::path file_path(info->file_name);
	std::filesystem::path abs_path = std::filesystem::absolute(file_path);

	std::ifstream file(abs_path.c_str(), std::ios::in | std::ios::binary);

	
	try {
   // file.open(abs_path.c_str(), std::ios::in | std::ios::binary);
	}
	catch (std::ios_base::failure& e) {
		Log::get() << e.what() << Log::endl;
		return result;
	}

    if (!file.is_open())
    {
		char data[256];
		strerror_s(data, 256, errno);
        Log::get() << "No file found: " << info->file_name <<" "<< data <<Log::endl;
        return result;
    }

    std::streampos current_position = file.tellg();
    file.seekg(0, std::ios::end);
    std::streampos ending_position = file.tellg();
    file.seekg(current_position);
    //    result.reset(new std::string());
    result.resize(static_cast<size_t>(ending_position - current_position));
    file.read(const_cast<char*>(result.data()), result.size());
    file.close();
    return result;
}

void native_file_provider::on_change(const std::filesystem::path& path, std::function<void()> f)
{
	auto s2 = path.parent_path().generic_wstring();
    thread_pool::get().enqueue([s2, f]()
    {
        while (thread_pool::is_good())
        {
            auto handle = FindFirstChangeNotificationW(
                              s2.c_str(),                       // directory to watch
                              false,                          // watch the subtree
                              FILE_NOTIFY_CHANGE_LAST_WRITE);  // watch dir name changes
			
			if (handle == INVALID_HANDLE_VALUE)
			{
				std::this_thread::sleep_for(2000_ms);
				continue;
			}

            while (thread_pool::is_good() && WAIT_OBJECT_0 == WaitForSingleObject(handle, 200))
            {
                // wait while os make file accessable
                std::this_thread::sleep_for(100_ms);
                f();
                break;
            }
        }

        Log::get() << "on_change ended" << Log::endl;
    });
}


std::shared_ptr<file> native_file_provider::get_file(std::filesystem::path file_name)
{
    std::shared_ptr<file> result;
    std::error_code ec;

    if (std::filesystem::exists(file_name, ec))
        //  if (_access(file_name.c_str(), 0) == 0)
    {
        result.reset(new file(this, file_name));
        result->edit_time = std::filesystem::last_write_time(file_name);
    }

    //  else
    //     Log::get() << "NO FILE " << file_name << Log::endl;

    if (ec)
        Log::get() << Log::LEVEL_ERROR << "file: " << file_name << " msg:" <<   ec.message() << Log::endl;

    return result;
}
std::shared_ptr<std::istream> native_file_provider::create_stream(std::filesystem::path file_name)
{
	return  std::shared_ptr<std::istream>(new std::ifstream(file_name.generic_wstring(), std::ios::binary), [](std::istream* str) {
	//	Log::get() << "istream deleter" << Log::endl;
		((std::ifstream*)str)->close();
		delete str; 
	}); //std::shared_ptr<istream> stream;
}


bool native_file_provider::save_data(std::filesystem::path file_name, std::string data)
{
    std::filesystem::path dir(file_name);
    std::filesystem::path parent_dir = dir.parent_path();

    if (!parent_dir.empty())
        if (std::filesystem::create_directories(parent_dir))
            std::cout << "Success" << "\n";

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
        std::error_code  ec;
        recursive_directory_iterator dir(path, ec), end;

        if (!ec)
            while (dir != end)
            {
                // do other stuff here.
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
        std::error_code  ec;
        directory_iterator dir(path, ec), end;

        if (!ec)
            while (dir != end)
            {
                // do other stuff here.
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
    //////////////////////////////////////////////////////////////////////////
    /////////////////////////////////OPTIMIZE/////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    if (recursive)
    {
        using namespace std::filesystem;
        std::error_code  ec;
        recursive_directory_iterator dir(path, ec), end;

        if (!ec)
            while (dir != end)
            {
                // do other stuff here.
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
        std::error_code  ec;
        directory_iterator dir(path, ec), end;

        if (!ec)
            while (dir != end)
            {
                // do other stuff here.
                if (is_directory(*dir))
                    f(dir->path().wstring());

                ++dir;
            }

        else
            Log::get() << Log::LEVEL_DEBUG << ec.message() << Log::endl;
    }
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
};

void FileSystem::iterate_dirs(std::filesystem::path path, std::function<void(std::filesystem::path)> f, bool recursive)
{
    for (auto& p : providers)
        p->iterate_dirs(path, f, recursive);
};


std::string file::load_all()
{
    //PROFILE(L"load_all");
    return provider->load_all(this);
}
template <typename TP>
std::time_t to_time_t(TP tp)
{
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
        + system_clock::now());
    return system_clock::to_time_t(sctp);
}
bool resource_file_depender::depender::need_update()
{
    auto file = FileSystem::get().get_file(file_name);

    if (file)
    {
        auto a = to_time_t(modify_time);
        auto b = to_time_t(file->edit_time);
    	if(a<b)
    	{
   
            Log::get() << "Current " << to_time_t(modify_time) << " file: " << to_time_t(file->edit_time) <<Log::endl;

                return true;
    	}
    }
       

    return false;
}

void resource_file_depender::add_depend(std::shared_ptr<file> _file)
{
    if (!_file) return;

    depender d;
    d.file_name = _file->file_name.generic_wstring();
    d.modify_time = _file->edit_time;
    files.push_back(d);
}

void resource_file_depender::clear()
{
    files.clear();
}
bool resource_file_depender::depends_on(std::string v)
{

    auto wstr = convert(v);
    for (auto f : files)
    {
        if (f.file_name.find(wstr) != std::string::npos)
        {
            return true;
        }
    }

    return false;
}
bool resource_file_depender::need_update()
{
    for (auto& d : files)
        if (d.need_update())
            return true;

    return false;
}
