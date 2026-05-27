module Core:Events;

import stl.core;

namespace Events
{

// ---- Runner -----------------------------------------------------------------

bool Runner::has_tasks()
{
	return !tasks.empty();
}

void Runner::process_tasks()
{
	std::list<std::function<void()>> copy;
	{
		std::lock_guard<std::mutex> g(m);
		std::swap(copy, tasks);
	}

	for (auto& t : copy)
	{
		t();
	}
}

void Runner::run(std::function<void()> f)
{
	std::lock_guard<std::mutex> g(m);
	tasks.emplace_back(f);
}

// ---- prop_helper ------------------------------------------------------------

void prop_helper::clear_remove()
{
	remove_func = nullptr;
}

void prop_helper::unregister()
{
	if (remove_func)
		remove_func();
	remove_func = nullptr;
}

prop_helper::~prop_helper()
{
	unregister();
}

// ---- prop_handler -----------------------------------------------------------

void prop_handler::add_helper(prop_handler* handler, std::shared_ptr<prop_helper> helper)
{
	handler->helpers.push_back(helper);
}

void prop_handler::clear_remove_funcs()
{
	for (auto& p : helpers)
	{
		if (p) p->remove_func = nullptr;
	}
}

void prop_handler::unregister(prop_handler* owner)
{
	for (auto& helper : owner->helpers)
	{
		if (helper->event == this)
		{
			helper->unregister();
		}
	}
}

prop_handler::~prop_handler()
{
	helpers.clear();
}

} // namespace Events
