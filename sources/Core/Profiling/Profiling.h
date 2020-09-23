#pragma once

#define PROFILING

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

	virtual Timer start(const wchar_t* name) = 0;
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
	 TimedBlock& get_child(std::wstring name)
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
	
	TimedBlock(std::wstring name);

	bool test = false;
	void update();
private:
	std::wstring name;
/*	Render::GPUTimer timer;*/
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
	/*friend class Render::Eventer;*/
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


static thread_local TimedBlock* current_block = nullptr;

class Profiler : public Singleton<Profiler>, public TimedBlock, public TimedRoot
{
	std::mutex m;
	//std::map<std::thread::id, TimedBlock*> blocks;

	
public:
	Events::Event<TimedBlock*> on_cpu_timer_start;
	Events::Event<TimedBlock*> on_cpu_timer_end;
	Events::Event<TimedBlock*> on_gpu_timer;
	Events::Event<UINT64> on_frame;

	Profiler() : TimedBlock(L"")
	{
	/*	Render::GPUTimeManager::create();*/
	}

	bool enabled = true;

	using TimedBlock::update;



	virtual Timer start(const wchar_t* name) override;

private:
	virtual void on_start(Timer* timer) override;


	virtual void on_end(Timer* timer) override;

};