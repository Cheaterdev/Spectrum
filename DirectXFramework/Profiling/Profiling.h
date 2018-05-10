#pragma once
/*
class CPUTimer
{

public:

	void start(DX12::CommandList*  list)
	{
		Singleton<GPUTimeManager>::get().start(*this, list);
	}

	void end(DX12::CommandList*  list)
	{
		Singleton<GPUTimeManager>::get().end(*this, list);
		list = nullptr;
	}

	float get_time()
	{
		return  Singleton<GPUTimeManager>::get().get_time(*this);
	}
};
*/
class Profiler;
class TimedBlock : public tree<TimedBlock, my_unique_vector<std::shared_ptr<TimedBlock>>>
{
	friend class TimedRoot;
	friend class Render::Eventer;
	friend class Profiler;

	std::mutex m;
public:
	using ptr = std::shared_ptr<TimedBlock>;


	virtual  TimedBlock& get_child(std::wstring name);
	// TimedBlock* parent = nullptr;

	bool enabled = true;
	template<class T>
	void iterate_childs(T f);

	float2 get_time();

	std::wstring get_name() const override;


	Events::prop<float2>::event_type on_time;

	std::chrono::time_point<std::chrono::high_resolution_clock>  start_time;
	std::chrono::time_point<std::chrono::high_resolution_clock>  end_time;

	Render::GPUTimer& get_timer()
	{
		return timer;
	}
protected:
	Events::prop<float2> time;

	std::chrono::duration<double> elapsed_time;
	TimedBlock(std::wstring name);

	bool test = false;
	void begin_timings(Render::Eventer* list = nullptr);

	void end_timings(Render::Eventer* list = nullptr);

	void update();
private:
	std::wstring name;
	//    std::vector<TimedBlock::ptr> childs;
	Render::GPUTimer timer;
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
	friend class Render::Eventer;
	friend class Profiler;

	TimedBlock& block;
	TimedRoot* root;

	Timer(TimedBlock& block, TimedRoot* root);
public:
	Timer(Timer&& t);
	~Timer();
	TimedBlock& get_block()
	{
		return block;
	}
};



class Profiler : public Singleton<Profiler>, public TimedBlock, public TimedRoot
{
	std::mutex m;
	std::map<std::thread::id, TimedBlock*> blocks;
public:
	Events::Event<TimedBlock*> on_cpu_timer_start;
	Events::Event<TimedBlock*> on_cpu_timer_end;
	Events::Event<TimedBlock*> on_gpu_timer;
	Events::Event<UINT64> on_frame;

	Profiler() : TimedBlock(L"")
	{
		Render::GPUTimeManager::create();
	}

	virtual  TimedBlock& get_child(std::wstring name) override
	{
		std::lock_guard<std::mutex> g(m);
		return TimedBlock::get_child(name);
	}

	using TimedBlock::update;



	virtual Timer start(const wchar_t* name) override;

private:
	virtual void on_start(Timer* timer) override;


	virtual void on_end(Timer* timer) override;

};
