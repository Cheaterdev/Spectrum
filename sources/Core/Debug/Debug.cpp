module Core:Debug;

import stl.core;
import stl.threading;
import :Log;

bool ObjectTracker::alive = true;

Object::Object()
{
	creation_stack = Exceptions::get_stack_trace();
}

void Object::set_name(std::string_view str)
{
	object_name = str;
}

ObjectTracker::ObjectTracker(std::string_view name) : name(name)
{
	Log::get() << "ObjectTracker created: " << name << Log::endl;
}

ObjectTracker::~ObjectTracker()
{
	alive = false;
	std::lock_guard<std::mutex> g(m);
	for (auto& o : objects)
	{
		Log::get() << "LEAKED OBJECT (" << name << "): '" << o->get_name() << "' stacktrace \n" << o->get_creation_stack() << Log::endl;
	}
}

void ObjectTracker::track(Object* object)
{
	std::lock_guard<std::mutex> g(m);
	objects.insert(object);
}

void ObjectTracker::untrack(Object* object)
{
	std::lock_guard<std::mutex> g(m);
	objects.erase(object);
}

void CounterManager::print()
{
	for (auto& f : print_functions)
		f();
}

Checker::Checker(std::atomic<std::thread::id>& c) : v(c)
{
	auto id = std::this_thread::get_id();
	auto prev = v.exchange(id);
	ASSERT(prev == std::thread::id());
}

Checker::~Checker()
{
	auto id = std::this_thread::get_id();
	auto prev = v.exchange(std::thread::id());
	ASSERT(prev == id);
}
