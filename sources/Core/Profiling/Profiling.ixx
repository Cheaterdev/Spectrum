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
// Set/clear from any thread before/after recording. A listener's callbacks run on whichever thread
// owns the timer, so they must be thread-safe.
struct CPUTimerListener
{
	virtual void on_cpu_start(TimedBlock*) = 0;
	virtual void on_cpu_end(TimedBlock*)   = 0;
	virtual ~CPUTimerListener()            = default;
};

class Profiler : public Singleton<Profiler>,public TimedRoot
{
	std::mutex m;
	static thread_local unsigned    pool_head;

	// Slot 0 is the profiler window's (set_cpu_listener); the others are add_cpu_listener's, so a capture
	// (TimingCapture) can run while the window is open.
	static constexpr size_t MaxCpuListeners = 4;
	std::array<std::atomic<CPUTimerListener*>, MaxCpuListeners> cpu_listeners{};

public:
	static thread_local TimedBlock* current_block;

	// on_cpu_timer_start / on_cpu_timer_end replaced by set_cpu_listener (lock-free hot path)
	Events::Event<std::pair<TimedBlock*, GPUTimerInterface*>> on_gpu_timer;
	Events::Event<std::uint64_t> on_frame;

	void set_cpu_listener(CPUTimerListener* l) { cpu_listeners[0].store(l, std::memory_order_release); }
	bool add_cpu_listener(CPUTimerListener* l);
	void remove_cpu_listener(CPUTimerListener* l);

	Profiler() {}

	bool enabled = false;
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


// Records every PROFILE(...) scope on every thread between begin() and end(), then writes
//   captures/<label>_<n>.json   a Chrome trace (chrome://tracing or ui.perfetto.dev), one row per thread
//   and a summary in the log: the scopes ordered by self time, then the longest single ones.
// Nothing runs outside a capture. It needs PROFILING (Debug / Profile builds), like PROFILE itself.
//
//   PROFILE_CAPTURE("drop asset");            // the rest of this scope
//   PROFILE_CAPTURE("add asset", 30);         // the rest of this scope and the next 30 frames: the work the scope
//                                             // queues for the render thread lands in the capture too
// A capture started while another is running is ignored (the outer one keeps going).
class TimingCapture : public Singleton<TimingCapture>, public CPUTimerListener, public Events::prop_handler
{
public:
	struct Event
	{
		const wchar_t* name;
		int64_t start_ns;   // steady clock
		int64_t end_ns;
	};

	TimingCapture();
	~TimingCapture();

	bool active() const { return running.load(std::memory_order_acquire); }

	// False when a capture is already running.
	bool begin(std::string label);
	void end();
	// Ends once `frames` frames (Profiler::on_frame) have passed; 0 ends now.
	void end_after_frames(uint frames);

	class Scope
	{
		bool owner;
		uint frames_after;
	public:
		Scope(std::string label, uint frames_after = 0);
		~Scope();
	};

	void on_cpu_start(TimedBlock*) override {}
	void on_cpu_end(TimedBlock*) override;

private:
	struct ThreadBuffer
	{
		std::mutex mutex;
		std::vector<Event> events;
		uint64_t generation = 0;
		uint32_t id = 0;
		uint64_t dropped = 0;
		std::thread::id owner;
	};

	ThreadBuffer& local_buffer();
	void write_report(const std::string& label, int64_t begin_ns, int64_t end_ns, std::thread::id begin_thread);

	std::atomic<bool> running{false};
	std::atomic<uint64_t> generation{0};
	std::atomic<int> frames_left{-1};   // -1: the capture ends by end()

	std::mutex state;   // label, times, the previous Profiler::enabled
	std::string label;
	int64_t begin_ns = 0;
	std::thread::id begin_thread;
	bool previous_enabled = false;

	std::mutex registry;
	std::vector<std::shared_ptr<ThreadBuffer>> buffers;
	uint32_t next_thread_id = 1;
	uint32_t capture_count = 0;
};



}
