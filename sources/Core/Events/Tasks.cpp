#include "pch.h"
#include "Events/Tasks.h"
#include "Log/Log.h"
#include "Events/Events.h"

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
