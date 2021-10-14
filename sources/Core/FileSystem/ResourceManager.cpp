#include "pch_core.h"
#include <io.h>
import Scheduler;
#include "ResourceManager.h"

import Utils;
import stl.core;
import windows;

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
