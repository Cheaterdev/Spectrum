#include "pch.h"
#include <io.h>

std::string native_file_provider::load_all(file* info)
{
    std::string result;

	boost::filesystem::path file_path(info->file_name);
	boost::filesystem::path abs_path = boost::filesystem::absolute(file_path);

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

void native_file_provider::on_change(std::wstring s, std::function<void()> f)
{
    auto s2 = s.substr(0, s.find_last_of(L"\\/"));
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
				std::this_thread::sleep_for(2000ms);
				continue;
			}

            while (thread_pool::is_good() && WAIT_OBJECT_0 == WaitForSingleObject(handle, 200))
            {
                // wait while os make file accessable
                std::this_thread::sleep_for(100ms);
                f();
                break;
            }
        }

        Log::get() << "on_change ended" << Log::endl;
    });
}


std::shared_ptr<file> native_file_provider::get_file(std::wstring file_name)
{
    std::shared_ptr<file> result;
    boost::system::error_code ec;

    if (boost::filesystem::exists(file_name, ec))
        //  if (_access(file_name.c_str(), 0) == 0)
    {
        result.reset(new file(this, file_name));
        result->edit_time = boost::filesystem::last_write_time(file_name);
    }

    //  else
    //     Log::get() << "NO FILE " << file_name << Log::endl;

    if (ec)
        Log::get() << Log::LEVEL_ERROR << "file: " << file_name << " msg:" <<   ec.message() << Log::endl;

    return result;
}
std::shared_ptr<istream> native_file_provider::create_stream(std::wstring file_name)
{
	return  std::shared_ptr<istream>(new ifstream(file_name, ios::binary), [](istream* str) {
	//	Log::get() << "istream deleter" << Log::endl;
		((ifstream*)str)->close();
		delete str; 
	}); //std::shared_ptr<istream> stream;
}


bool native_file_provider::save_data(std::wstring file_name, std::string data)
{
    boost::filesystem::path dir(file_name);
    boost::filesystem::path parent_dir = dir.parent_path();

    if (!parent_dir.empty())
        if (boost::filesystem::create_directories(parent_dir))
            std::cout << "Success" << "\n";

    std::ofstream file;
    file.open(file_name, std::ios::out | std::ios::binary);

    if (!file.is_open())
    {
        Log::get() << Log::LEVEL_ERROR << "wtf" << Log::endl;
        return false;
    }

    file.write(data.data(), data.size());
    file.close();
    return true;
}

void native_file_provider::iterate(std::wstring path, std::function<void(file::ptr)> f, bool recursive)
{
    if (recursive)
    {
        using namespace boost::filesystem;
        boost::system::error_code  ec;
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
        using namespace boost::filesystem;
        boost::system::error_code  ec;
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


void native_file_provider::iterate_dirs(std::wstring path, std::function<void(std::wstring)> f, bool recursive)
{
    //////////////////////////////////////////////////////////////////////////
    /////////////////////////////////OPTIMIZE/////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    if (recursive)
    {
        using namespace boost::filesystem;
        boost::system::error_code  ec;
        recursive_directory_iterator dir(path, ec), end;

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

    else
    {
        using namespace boost::filesystem;
        boost::system::error_code  ec;
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


std::shared_ptr<file> FileSystem::get_file(std::string name)
{
    return get_file(convert(name));
}
std::shared_ptr<file> FileSystem::get_file(std::wstring name)
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

bool FileSystem::save_data(std::wstring file_name, std::string data)
{
    for (auto& p : providers)
    {
        if (p->save_data(file_name, data))
            return true;
    }

    return false;
}

void FileSystem::iterate(std::wstring path, std::function<void(file::ptr)> f, bool recursive)
{
    for (auto& p : providers)
        p->iterate(path, f, recursive);
};

void FileSystem::iterate_dirs(std::wstring path, std::function<void(std::wstring)> f, bool recursive)
{
    for (auto& p : providers)
        p->iterate_dirs(path, f, recursive);
};


std::string file::load_all()
{
    return provider->load_all(this);
}

bool resource_file_depender::depender::need_update()
{
    auto file = FileSystem::get().get_file(file_name);

    if (file)
        return modify_time < file->edit_time;

    return false;
}

void resource_file_depender::add_depend(std::shared_ptr<file> _file)
{
    if (!_file) return;

    depender d;
    d.file_name = _file->file_name;
    d.modify_time = _file->edit_time;
    files.push_back(d);
}

void resource_file_depender::clear()
{
    files.clear();
}

bool resource_file_depender::need_update()
{
    for (auto& d : files)
        if (d.need_update())
            return true;

    return false;
}
