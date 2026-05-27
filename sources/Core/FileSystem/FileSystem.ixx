export module Core:FileSystem;

import :Utils;
import :Log;
import :Singleton;
import :Scheduler;
import :Application;

import stl.filesystem;
import stl.memory;
import windows;
import :serialization;

export
{

	struct file;

class file_provider
{

public:
	using ptr = s_ptr<file_provider>;

	virtual std::string load_all(file*) = 0;
	virtual std::shared_ptr<file> get_file(std::filesystem::path) = 0;
	virtual std::shared_ptr<std::istream> create_stream(std::filesystem::path) = 0;

	virtual bool save_data(std::filesystem::path, std::string) { return false; };

	virtual void iterate(std::filesystem::path, std::function<void(s_ptr<file>)> f, bool recursive) {};
	virtual void iterate_dirs(std::filesystem::path, std::function<void(std::filesystem::path)> f, bool recursive) {};

	virtual void on_change(const std::filesystem::path&, std::function<void()>) {};
	// TODO: save_file, get_file_info, iterate all files|dirs, streaming?
};

struct file
{
	std::string load_all();
	std::shared_ptr<std::istream> get_new_stream();
	file(file_provider* _provider, std::filesystem::path _file_name);
public:
	using ptr = s_ptr<file>;
	std::filesystem::path file_name;
	std::filesystem::file_time_type edit_time;

	file_provider* provider;
};


class FileSystem : public Singleton<FileSystem>
{
	std::vector<file_provider::ptr> providers;
public:
	void register_provider(file_provider::ptr provider);

	virtual file::ptr get_file(std::filesystem::path);
	virtual bool save_data(std::filesystem::path file_name, std::string data);

	template<class T>
	s_ptr<T> get_provider()
	{
		for (auto& p : providers)
		{
			auto r = std::dynamic_pointer_cast<T>(p);

			if (r)
				return r;
		}

		return nullptr;
	}
	virtual void iterate(std::filesystem::path path, std::function<void(file::ptr)> f, bool recursive);
	virtual void iterate_dirs(std::filesystem::path path, std::function<void(std::filesystem::path)> f, bool recursive);
};

class native_file_provider : public file_provider
{
public:
	virtual std::string load_all(file*);

	virtual std::shared_ptr<file> get_file(std::filesystem::path dir);
	virtual bool save_data(std::filesystem::path dir, std::string data);
	virtual void iterate(std::filesystem::path path, std::function<void(file::ptr)> f, bool recursive);
	virtual void iterate_dirs(std::filesystem::path, std::function<void(std::filesystem::path)> f, bool recursive);
	virtual void on_change(const std::filesystem::path&, std::function<void()>) override;
	virtual std::shared_ptr<std::istream> create_stream(std::filesystem::path);

	virtual ~native_file_provider();
};



}
