#include "pch.h"
export module Tasks;

import EventsProps;
import Singletons;


export class Task
{
public:
    Task(std::wstring);
    virtual ~Task();
    using ptr = std::shared_ptr<Task>;

    std::wstring name;
    bool active = true;
    Events::prop<float> percent;
};


export class TaskInfoManager : public Singleton<TaskInfoManager>
{
    friend class Task;
public:

    Events::Event<Task*> on_start_task;
    Events::Event<Task*> on_end_task;
    Events::Event<Task*> on_change_task;

    std::shared_ptr<Task> create_task(std::wstring name);
};




std::shared_ptr<Task> TaskInfoManager::create_task(std::wstring name)
{
    auto task = std::make_shared<Task>(name);
    return task;
}

Task::Task(std::wstring name)
{
    this->name = name;
    TaskInfoManager::get().on_start_task(this);
}

Task::~Task()
{
    TaskInfoManager::get().on_end_task(this);
}
