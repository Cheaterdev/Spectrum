export module Core:Profiling;

export import "macros.h";

import :Log;
import :Singleton;

import :Utils;
import :Math;
import :Tree;
import :shared_ptr;

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
	std::chrono::time_point<std::chrono::high_resolution_clock>  start_time;
	std::chrono::time_point<std::chrono::high_resolution_clock>  end_time;
	std::chrono::duration<double> get_lapsed_time()
	{
	return end_time - start_time;
	}
};


class Profiler;
class TimedBlock: public SharedObject<TimedBlock>
{
	friend class TimedRoot;
	friend class Profiler;

public:
	using ptr = std::shared_ptr<TimedBlock>;
				int id;
	std::wstring get_name() const;
	CPUCounter cpu_counter;
	TimedBlock(std::wstring_view name, TimedBlock* parent);
	 virtual ~TimedBlock() = default;
public:
	std::wstring name;
	uint level;
	TimedBlock* parent;
};


class Timer
{
	friend class TimedRoot;
	/*friend class HAL::Eventer;*/
	friend class Profiler;

	TimedBlock::ptr block;
	TimedRoot* root;
public:
	Timer(TimedBlock::ptr block, TimedRoot* root);

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

class GPUTimerInterface
{
public:
	virtual uint64 get_start() = 0;

	virtual uint64 get_end() = 0;

	virtual ~GPUTimerInterface() = default;
};

class Profiler : public Singleton<Profiler>,public TimedRoot
{
	std::mutex m;
	//std::map<std::thread::id, TimedBlock*> blocks;
	static thread_local TimedBlock* current_block;

public:
	Events::Event<TimedBlock*> on_cpu_timer_start;
	Events::Event<TimedBlock*> on_cpu_timer_end;
	Events::Event<std::pair<TimedBlock*, GPUTimerInterface*>> on_gpu_timer;
	Events::Event<std::uint64_t> on_frame;

	Profiler() 
	{
	/*	HAL::GPUTimeManager::create();*/
	}

	bool enabled = true;


	TimedBlock* get_current() const;
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


std::wstring TimedBlock::get_name() const
{
	return name;
}

TimedBlock::TimedBlock(std::wstring_view name, TimedBlock* parent) : name(name),  parent(parent) , level(parent?parent->level+1:0)
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
