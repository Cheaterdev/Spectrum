#include "pch_core.h"
#include "Profiling.h"


Timer::Timer(Timer&& t) : block(t.block), root(t.root)
{
	t.root = nullptr;
}

Timer::Timer(TimedBlock* block, TimedRoot* root) : block(block), root(root)
{
	if(block)
	root->start(this);
}

Timer::~Timer()
{
	if (root)
		root->end(this);
}


std::wstring TimedBlock::get_name() const
{
	return name;
}

TimedBlock::TimedBlock(std::wstring_view name) : name(name)
{
	
}

void TimedBlock::update()
{
	std::lock_guard<std::mutex> g(m);

	Profiler::get().on_gpu_timer(this);
	
	for (auto& c : childs)
		c->update();
}

Timer Profiler::start(std::wstring_view name)
{
	if (!enabled) 
	return Timer(nullptr, nullptr);

	m.lock();
	TimedBlock* block = current_block;
	if (!block)
		block = this;
	m.unlock();
	
	return Timer(&block->get_child(name), this);
}

void Profiler::on_start(Timer* timer)
{
	current_block = timer->block;
	timer->block->cpu_counter.start_time = std::chrono::high_resolution_clock::now();
	on_cpu_timer_start(timer->block);
}

void Profiler::on_end(Timer* timer)
{
	current_block = timer->block->parent;
	timer->block->cpu_counter.end_time = std::chrono::high_resolution_clock::now();
	timer->block->cpu_counter.elapsed_time = timer->block->cpu_counter.end_time - timer->block->cpu_counter.start_time;
	on_cpu_timer_end(timer->block);
}
