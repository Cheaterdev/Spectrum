#pragma once
#include "Events.h"
#include "patterns/Singleton.h"

class Task
{
    public:
        Task(std::wstring);
        virtual ~Task();
		using ptr = std::shared_ptr<Task>;

        std::wstring name;
		bool active = true;
		Events::prop<float> percent;
};


class TaskInfoManager: public Singleton<TaskInfoManager>
{
        friend class Task;
    public:

        Events::Event<Task*> on_start_task;
        Events::Event<Task*> on_end_task;
        Events::Event<Task*> on_change_task;

		std::shared_ptr<Task> create_task(std::wstring name);
};
