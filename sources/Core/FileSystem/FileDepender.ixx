export module Core:FileDepender;

import :Utils;
import :Log;
import :Singleton;
import :Scheduler;

import stl.filesystem;
import stl.memory;
import windows;

import :serialization;
import :FileSystem;

export
{


	class resource_file_depender
	{
		struct depender
		{
			std::filesystem::path file_name;
			std::filesystem::file_time_type modify_time;

			bool need_update() const;

            bool operator<(const  depender& r) const
            {
	            return file_name<r.file_name;
            }
		private:

			inline const std::wstring get_name() const { return file_name.wstring(); }
			SERIALIZE()
			{
				ar& NVP(file_name)& NVP(modify_time);
			}
		};
		std::set<depender> files;
	public:

		inline const std::set<depender>& get_files() const { return files; }

		void add_depend(std::shared_ptr<file> _file);
		bool need_update() const;
		void clear();
		bool depends_on(std::string v) const;
	private:
		SERIALIZE()
		{
			ar& NVP(files);
		}
	};

}




template <typename TP>
std::time_t to_time_t(TP tp)
{
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
        + system_clock::now());
    return system_clock::to_time_t(sctp);
}
bool resource_file_depender::depender::need_update() const
{
    auto file = FileSystem::get().get_file(file_name);

    if (file)
    {
        auto a = to_time_t(modify_time);
        auto b = to_time_t(file->edit_time);
        if (a < b)
        {

            Log::get() << "Current " << to_time_t(modify_time) << " file: " << to_time_t(file->edit_time) << Log::endl;

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
    files.insert(d);
}

void resource_file_depender::clear()
{
    files.clear();
}
bool resource_file_depender::depends_on(std::string v) const
{

    auto wstr = convert(v);
    for (auto &f : files)
    {
        if (f.file_name.wstring().find(wstr) != std::string::npos)
        {
            return true;
        }
    }

    return false;
}
bool resource_file_depender::need_update() const
{
    for (auto& d : files)
        if (d.need_update())
            return true;

    return false;
}
