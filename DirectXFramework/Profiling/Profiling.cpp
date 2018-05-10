#include "pch.h"
#include "Profiling.h"


Timer::Timer(Timer&& t) : block(t.block), root(t.root)
{
	t.root = nullptr;
}

Timer::Timer(TimedBlock& block, TimedRoot* root) : block(block), root(root)
{
	root->on_start(this);
}

Timer::~Timer()
{
	if (root)
		root->on_end(this);
}

TimedBlock& TimedBlock::get_child(std::wstring name)
{
	std::lock_guard<std::mutex> g(m);
	auto it = std::find_if(childs.begin(), childs.end(), [name](const TimedBlock::ptr & p)->bool
	{
		return p->get_name() == name;
	});

	if (it == childs.end())
	{
		auto c = std::shared_ptr<TimedBlock>(new TimedBlock(name));
		add_child(c);
		return *c;
	}

	// it.reset(new TimedBlock(name, this));
	return **it;
}

float2 TimedBlock::get_time()
{
	return time.get();
}

std::wstring TimedBlock::get_name() const
{
	return name;
}

TimedBlock::TimedBlock(std::wstring name) : name(name)
{
	time.register_change(on_time);
}

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

void TimedBlock::update()
{
	std::lock_guard<std::mutex> g(m);

	Profiler::get().on_gpu_timer(this);

	time = float2(timer.get_time(), elapsed_time.count());

	for (auto& c : childs)
		c->update();
}

Timer Profiler::start(const wchar_t* name)
{
	//TimedBlock * block = this;
	m.lock();
	TimedBlock * block= blocks[std::this_thread::get_id()];
	if (!block)
		block = this;
	m.unlock();

	return Timer(block->get_child(name), this);
}

void Profiler::on_start(Timer* timer)
{
	m.lock();
	blocks[std::this_thread::get_id()] =&timer->block;
	m.unlock();
	timer->block.begin_timings(nullptr);
}

void Profiler::on_end(Timer* timer)
{
	m.lock();
	blocks[std::this_thread::get_id()] = timer->block.parent;
	m.unlock();
	timer->block.end_timings(nullptr);
}
