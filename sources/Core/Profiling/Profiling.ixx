export module Core:Profiling;

export import "macros.h";

import :Log;
import :Singleton;

import :Utils;
import :Math;
import :Tree;
import :shared_ptr;

export
{

// Compile-time wide string handle. Implicit constructor and _cs UDL both consteval,
// so any non-literal argument is a hard compile error.
struct LiteralWStr
{
	const wchar_t* ptr = nullptr;
	consteval LiteralWStr(const wchar_t* p) noexcept : ptr(p) {}
	LiteralWStr() = default;
};

consteval LiteralWStr operator""_cs(const wchar_t* s, size_t) noexcept { return {s}; }

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
	virtual Timer start(LiteralWStr name) = 0;
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
	int id = -1;
	std::wstring_view get_name() const;
	CPUCounter cpu_counter;
	TimedBlock(LiteralWStr name, TimedBlock* parent);
	TimedBlock() = default;   // for thread-local pool allocation
	virtual ~TimedBlock() = default;
public:
	const wchar_t* name = nullptr;
	uint level = 0;
	TimedBlock* parent = nullptr;
};


class Timer
{
	friend class TimedRoot;
	friend class Profiler;

	TimedBlock::ptr owned_block;  // non-null only for heap-allocated blocks (Eventer/GPUBlock)
	TimedBlock*     block;        // raw ptr; valid for both pool and heap blocks
	TimedRoot*      root;
public:
	Timer();                                          // disabled / null timer
	Timer(TimedBlock::ptr block, TimedRoot* root);   // heap path (Eventer)
	Timer(TimedBlock* block, TimedRoot* root);        // pool path (Profiler)

	Timer(Timer&& t) noexcept;
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

// Lock-free listener interface for the CPU timer hot path.
// At most one listener is supported; set/clear from any thread before/after recording.
struct CPUTimerListener
{
	virtual void on_cpu_start(TimedBlock*) = 0;
	virtual void on_cpu_end(TimedBlock*)   = 0;
	virtual ~CPUTimerListener()            = default;
};

class Profiler : public Singleton<Profiler>,public TimedRoot
{
	std::mutex m;
	static thread_local TimedBlock* current_block;
	static thread_local unsigned    pool_head;

	std::atomic<CPUTimerListener*> cpu_listener{nullptr};

public:
	// on_cpu_timer_start / on_cpu_timer_end replaced by set_cpu_listener (lock-free hot path)
	Events::Event<std::pair<TimedBlock*, GPUTimerInterface*>> on_gpu_timer;
	Events::Event<std::uint64_t> on_frame;

	void set_cpu_listener(CPUTimerListener* l) { cpu_listener.store(l, std::memory_order_release); }

	Profiler() {}

	bool enabled = true;
	int max_level = std::numeric_limits<int>::max();

	TimedBlock* get_current() const;
	virtual Timer start(LiteralWStr name) override;

private:
	virtual void on_start(Timer* timer) override;
	virtual void on_end(Timer* timer) override;
};


class ScopedCounter
{
	std::string name;
	std::chrono::time_point<std::chrono::high_resolution_clock> start;
public:
	ScopedCounter(std::string name);
	~ScopedCounter();
};



}
