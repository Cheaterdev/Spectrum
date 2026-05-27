module Core:FileDepender;

import stl.core;
import stl.filesystem;
import :FileSystem;
import :Log;
import :Utils;

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
    for (auto& f : files)
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
