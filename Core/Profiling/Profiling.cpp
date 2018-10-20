#include "pch.h"
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

/*
float2 TimedBlock::get_time()
{
	return time.get();
}*/

std::wstring TimedBlock::get_name() const
{
	return name;
}

TimedBlock::TimedBlock(std::wstring name) : name(name)
{
	
}
/*
void TimedBlock::begin_timings(Render::Eventer* list)
{
	enabled = true;
	test = !!list;
	if (list)
		timer.start(list);
	start_time = std::chrono::high_resolution_clock::now();
	Profiler::get().on_cpu_timer_start(this);
	std::lock_guard<std::mutex> g(m);

	for (auto& c : childs)
		c->enabled = false;

	//   childs.clear();
}

void TimedBlock::end_timings(Render::Eventer * list)
{
	assert(test == !!list);
	end_time = std::chrono::high_resolution_clock::now();
	elapsed_time = end_time - start_time;
	if (list)
		timer.end(list);

	Profiler::get().on_cpu_timer_end(this);
}
*/
void TimedBlock::update()
{
	std::lock_guard<std::mutex> g(m);

	Profiler::get().on_gpu_timer(this);
	
	for (auto& c : childs)
		c->update();
}

Timer Profiler::start(const wchar_t* name)
{
	m.lock();
	TimedBlock * block= blocks[std::this_thread::get_id()];
	if (!block)
		block = this;
	m.unlock();
	if(enabled)
	return Timer(&block->get_child(name), this);
	else return Timer(nullptr,nullptr);
}

void Profiler::on_start(Timer* timer)
{
	m.lock();
	blocks[std::this_thread::get_id()] =timer->block;
	timer->block->cpu_counter.start_time = std::chrono::high_resolution_clock::now();
	on_cpu_timer_start(timer->block);
	m.unlock();
}

void Profiler::on_end(Timer* timer)
{
	m.lock();
	blocks[std::this_thread::get_id()] = timer->block->parent;
	timer->block->cpu_counter.end_time = std::chrono::high_resolution_clock::now();
	timer->block->cpu_counter.elapsed_time = timer->block->cpu_counter.end_time - timer->block->cpu_counter.start_time;
	on_cpu_timer_end(timer->block);
	m.unlock();
}
