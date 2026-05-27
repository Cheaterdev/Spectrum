module Core:Profiling;

import stl.core;
import :Log;

thread_local TimedBlock root_block(L"thread", nullptr);
thread_local TimedBlock* Profiler::current_block = &root_block;


Timer::Timer(Timer&& t) : block(t.block), root(t.root)
{
	t.root = nullptr;
}

Timer::Timer(TimedBlock::ptr block, TimedRoot* root) : block(block), root(root)
{
	if (block)
		root->start(this);
}

Timer::~Timer()
{
	if (root)
		root->end(this);
}


std::wstring_view TimedBlock::get_name() const
{
	return name;
}

TimedBlock::TimedBlock(std::wstring_view name, TimedBlock* parent)
	: name(name), parent(parent), level(parent ? parent->level + 1 : 0)
{
}

TimedBlock* Profiler::get_current() const
{
	return current_block;
}

Timer Profiler::start(std::wstring_view name)
{
	if (!enabled)
		return Timer(nullptr, nullptr);

	return Timer(std::make_shared<TimedBlock>(name, current_block), this);
}

void Profiler::on_start(Timer* timer)
{
	current_block = timer->block.get();
	timer->block->cpu_counter.start_time = std::chrono::high_resolution_clock::now();
	on_cpu_timer_start(timer->block.get());
}

void Profiler::on_end(Timer* timer)
{
	current_block = timer->block->parent;
	timer->block->cpu_counter.end_time = std::chrono::high_resolution_clock::now();
	on_cpu_timer_end(timer->block.get());
}

ScopedCounter::ScopedCounter(std::string name) : name(name)
{
	start = std::chrono::high_resolution_clock::now();
}

ScopedCounter::~ScopedCounter()
{
	auto end = std::chrono::high_resolution_clock::now();
	double time = std::chrono::duration<double>(end - start).count();
	Log::get() << name << ":" << std::to_string(int(time * 1000)) << Log::endl;
}
