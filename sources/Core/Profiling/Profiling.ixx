export module Profiling;

export import "macros.h";

import Log;
import Singleton;

import Utils;
import Math;
import Tree;

//#define PROFILING
export
{

class Timer;
class TimedRoot
{
protected:
	TimedRoot * parent = nullptr;
	friend class Timer;



	virtual  void start(Timer* timer)
	{
		on_start(timer);
		if (parent)
			parent->start(timer);
	}
	virtual  void end(Timer* timer)
	{
		on_end(timer);
		if (parent)
			parent->end(timer);
	}


	virtual  void on_start(Timer* timer) = 0;
	virtual  void on_end(Timer* timer) = 0;

public:

	virtual Timer start(std::wstring_view name) = 0;
};



class CPUCounter
{
public:
	//bool enabled = true;

	Events::prop<float2>::event_type on_time;

	std::chrono::time_point<std::chrono::high_resolution_clock>  start_time;
	std::chrono::time_point<std::chrono::high_resolution_clock>  end_time;

	Events::prop<float2> time;

	std::chrono::duration<double> elapsed_time;

	CPUCounter()
	{
		time.register_change(on_time);
	}
};


class Profiler;
class TimedBlock : public tree<TimedBlock, my_unique_vector<std::shared_ptr<TimedBlock>>>
{
	friend class TimedRoot;
	friend class Profiler;

	std::mutex m;
public:
	using ptr = std::shared_ptr<TimedBlock>;

	int id;

	template<class T = TimedBlock>
	 TimedBlock& get_child(std::wstring_view name)
	{
		std::lock_guard<std::mutex> g(m);
		auto it = std::find_if(childs.begin(), childs.end(), [name](const TimedBlock::ptr & p)->bool
		{
			return p->get_name() == name;
		});

		if (it == childs.end())
		{
			auto c = std::shared_ptr<TimedBlock>(new T(name));
			add_child(c);
			return *c;
		}

		// it.reset(new TimedBlock(name, this));
		return **it;
	}
	
	template<class T>
	void iterate_childs(T f);
	std::wstring get_name() const override;

	CPUCounter cpu_counter;

protected:
	
	TimedBlock(std::wstring_view name);

	bool test = false;
	void update();
private:
	std::wstring name;
/*	Graphics::GPUTimer timer;*/
};

template<class T>
void TimedBlock::iterate_childs(T f)
{
	std::lock_guard<std::mutex> g(m);

	for (auto& c : childs)
		f(c.get());
}

class Timer
{
	friend class TimedRoot;
	/*friend class Graphics::Eventer;*/
	friend class Profiler;

	TimedBlock* block;
	TimedRoot* root;
public:
	Timer(TimedBlock* block, TimedRoot* root);

	Timer(Timer&& t);
	~Timer();
	TimedBlock& get_block()
	{
		return *block;
	}

	TimedRoot* get_root()
	{
		return root;
	}
};



class Profiler : public Singleton<Profiler>, public TimedBlock, public TimedRoot
{
	std::mutex m;
	//std::map<std::thread::id, TimedBlock*> blocks;
	static thread_local TimedBlock* current_block;

	
public:
	Events::Event<TimedBlock*> on_cpu_timer_start;
	Events::Event<TimedBlock*> on_cpu_timer_end;
	Events::Event<TimedBlock*> on_gpu_timer;
	Events::Event<std::uint64_t> on_frame;

	Profiler() : TimedBlock(L"")
	{
	/*	Graphics::GPUTimeManager::create();*/
	}

	bool enabled = true;

	using TimedBlock::update;



	virtual Timer start(std::wstring_view name) override;

private:
	virtual void on_start(Timer* timer) override;


	virtual void on_end(Timer* timer) override;

};


class ScopedCounter
{
	std::string name;
	std::chrono::time_point<std::chrono::high_resolution_clock> start;
public:
	ScopedCounter(std::string name):name(name)
	{
		start = std::chrono::high_resolution_clock::now();
	}

	~ScopedCounter()
	{
		auto end = std::chrono::high_resolution_clock::now();

		double time = std::chrono::duration<double>(end- start).count();

		Log::get() << name << ":" << std::to_string(int(time * 1000)) << Log::endl;
	}
};



}

module: private;
thread_local TimedBlock* Profiler::current_block = nullptr;


Timer::Timer(Timer&& t) : block(t.block), root(t.root)
{
	t.root = nullptr;
}

Timer::Timer(TimedBlock* block, TimedRoot* root) : block(block), root(root)
{
	if (block)
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
