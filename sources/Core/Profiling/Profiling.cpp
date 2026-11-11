module Core:Profiling;

import stl.core;
import :Log;
import :Utils;

static constexpr unsigned POOL_SIZE = 2048;  // must be power of two

thread_local TimedBlock root_block(L"thread", nullptr);
thread_local TimedBlock* Profiler::current_block = &root_block;
thread_local unsigned    Profiler::pool_head      = 0;
thread_local std::array<TimedBlock, POOL_SIZE> s_block_pool;


// ── Timer constructors ────────────────────────────────────────────────────

Timer::Timer() : owned_block(nullptr), block(nullptr), root(nullptr) {}

Timer::Timer(TimedBlock::ptr blk, TimedRoot* r)
	: owned_block(blk), block(blk.get()), root(r)
{
	if (block)
		root->start(this);
}

Timer::Timer(TimedBlock* blk, TimedRoot* r)
	: owned_block(nullptr), block(blk), root(r)
{
	if (block)
		root->start(this);
}

Timer::Timer(Timer&& t) noexcept
	: owned_block(std::move(t.owned_block)), block(t.block), root(t.root)
{
	t.block = nullptr;
	t.root  = nullptr;
}

Timer::~Timer()
{
	if (root)
		root->end(this);
}


// ── TimedBlock ────────────────────────────────────────────────────────────

std::wstring_view TimedBlock::get_name() const
{
	return name ? std::wstring_view{name} : std::wstring_view{};
}

TimedBlock::TimedBlock(LiteralWStr name, TimedBlock* parent)
	: name(name.ptr), parent(parent), level(parent ? parent->level + 1 : 0)
{
}


// ── Profiler ──────────────────────────────────────────────────────────────

TimedBlock* Profiler::get_current() const
{
	return current_block;
}

Timer Profiler::start(LiteralWStr name)
{
	if (!enabled)
		return Timer();

	if (current_block && current_block->level >= max_level)
		return Timer();

	auto& block  = s_block_pool[pool_head++ & (POOL_SIZE - 1)];
	block.name   = name.ptr;
	block.parent = current_block;
	block.level  = current_block ? current_block->level + 1 : 0;
	block.id     = -1;
	return Timer(&block, this);
}

void Profiler::on_start(Timer* timer)
{
	current_block = timer->block;
	timer->block->cpu_counter.start_time = std::chrono::high_resolution_clock::now();
	for (auto& slot : cpu_listeners)
		if (auto* l = slot.load(std::memory_order_relaxed)) l->on_cpu_start(timer->block);
}

void Profiler::on_end(Timer* timer)
{
	auto* blk = timer->block;
	if (blk >= s_block_pool.data() && blk < s_block_pool.data() + POOL_SIZE)
		--pool_head;
	current_block = blk->parent;
	blk->cpu_counter.end_time = std::chrono::high_resolution_clock::now();
	for (auto& slot : cpu_listeners)
		if (auto* l = slot.load(std::memory_order_relaxed)) l->on_cpu_end(blk);
}

bool Profiler::add_cpu_listener(CPUTimerListener* l)
{
	for (size_t i = 1; i < MaxCpuListeners; i++)
	{
		CPUTimerListener* expected = nullptr;
		if (cpu_listeners[i].compare_exchange_strong(expected, l, std::memory_order_acq_rel))
			return true;
	}
	return false;
}

void Profiler::remove_cpu_listener(CPUTimerListener* l)
{
	for (size_t i = 1; i < MaxCpuListeners; i++)
	{
		CPUTimerListener* expected = l;
		cpu_listeners[i].compare_exchange_strong(expected, nullptr, std::memory_order_acq_rel);
	}
}


// ── ScopedCounter ─────────────────────────────────────────────────────────

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


// ── TimingCapture ─────────────────────────────────────────────────────────

namespace
{
	int64_t to_ns(std::chrono::time_point<std::chrono::high_resolution_clock> t)
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(t.time_since_epoch()).count();
	}

	int64_t now_ns()
	{
		return to_ns(std::chrono::high_resolution_clock::now());
	}

	// A runaway capture (a scope left open over a long run) stops growing instead of eating the memory.
	constexpr size_t MaxEventsPerThread = 4 * 1024 * 1024;

	std::string ms_text(int64_t ns)
	{
		std::ostringstream out;
		out.setf(std::ios::fixed);
		out.precision(2);
		out << double(ns) / 1e6;
		return out.str();
	}

	std::string padded(std::string text, size_t width)
	{
		if (text.size() < width)
			text.insert(0, width - text.size(), ' ');
		return text;
	}

	std::string json_escape(const std::string& in)
	{
		std::string out;
		for (char c : in)
		{
			if (c == '"' || c == '\\')
			{
				out += '\\';
				out += c;
			}
			else if (static_cast<unsigned char>(c) < 0x20)
				out += ' ';
			else
				out += c;
		}
		return out;
	}

	std::string file_safe(const std::string& in)
	{
		std::string out = in;
		for (char& c : out)
			if (!std::isalnum(static_cast<unsigned char>(c)))
				c = '_';
		return out.empty() ? std::string("capture") : out;
	}
}

TimingCapture::TimingCapture()
{
	Profiler::get().on_frame.register_handler(this, [this](std::uint64_t)
	{
		// fetch_sub returns the old value: 1 means this frame was the last one.
		if (frames_left.load(std::memory_order_acquire) > 0 && frames_left.fetch_sub(1) == 1)
			end();
	});
}

TimingCapture::~TimingCapture() = default;

TimingCapture::ThreadBuffer& TimingCapture::local_buffer()
{
	thread_local ThreadBuffer* cached = nullptr;
	if (!cached)
	{
		auto buffer = std::make_shared<ThreadBuffer>();
		buffer->owner = std::this_thread::get_id();

		std::lock_guard<std::mutex> g(registry);
		buffer->id = next_thread_id++;
		buffers.push_back(buffer);
		cached = buffer.get();   // the registry keeps it alive for the process
	}
	return *cached;
}

bool TimingCapture::begin(std::string name)
{
	std::lock_guard<std::mutex> g(state);
	if (running.load(std::memory_order_acquire))
		return false;

	auto& profiler = Profiler::get();
	if (!profiler.add_cpu_listener(this))
		return false;

	label = std::move(name);
	begin_thread = std::this_thread::get_id();
	previous_enabled = profiler.enabled;
	frames_left.store(-1);
	generation.fetch_add(1);
	begin_ns = now_ns();

	profiler.enabled = true;
	running.store(true, std::memory_order_release);
	return true;
}

void TimingCapture::end_after_frames(unsigned int frames)
{
	if (frames == 0)
		end();
	else if (running.load(std::memory_order_acquire))
		frames_left.store(static_cast<int>(frames), std::memory_order_release);
}

void TimingCapture::end()
{
	std::string name;
	int64_t begun;
	std::thread::id caller;
	{
		std::lock_guard<std::mutex> g(state);
		if (!running.exchange(false))
			return;

		Profiler::get().remove_cpu_listener(this);
		Profiler::get().enabled = previous_enabled;
		name = label;
		begun = begin_ns;
		caller = begin_thread;
	}
	frames_left.store(-1);

	write_report(name, begun, now_ns(), caller);
}

void TimingCapture::on_cpu_end(TimedBlock* block)
{
	if (!running.load(std::memory_order_acquire))
		return;

	ThreadBuffer& buffer = local_buffer();
	std::lock_guard<std::mutex> g(buffer.mutex);

	const uint64_t current = generation.load(std::memory_order_acquire);
	if (buffer.generation != current)
	{
		buffer.events.clear();
		buffer.dropped = 0;
		buffer.generation = current;
	}

	if (buffer.events.size() >= MaxEventsPerThread)
	{
		buffer.dropped++;
		return;
	}
	buffer.events.push_back({ block->name, to_ns(block->cpu_counter.start_time), to_ns(block->cpu_counter.end_time) });
}

void TimingCapture::write_report(const std::string& name, int64_t begun, int64_t ended, std::thread::id caller)
{
	struct ThreadEvents
	{
		uint32_t id;
		bool is_caller;
		uint64_t dropped;
		std::vector<Event> events;
	};
	std::vector<ThreadEvents> threads;

	{
		const uint64_t current = generation.load();
		std::lock_guard<std::mutex> g(registry);
		for (auto& buffer : buffers)
		{
			std::lock_guard<std::mutex> bg(buffer->mutex);
			if (buffer->generation != current || buffer->events.empty())
				continue;

			threads.push_back({ buffer->id, buffer->owner == caller, buffer->dropped, std::move(buffer->events) });
			buffer->events.clear();
			buffer->dropped = 0;
		}
	}

	auto event_name = [](const Event& e) { return e.name ? std::wstring(e.name) : std::wstring(L"?"); };

	// Self time: a scope's duration minus the scopes nested in it, found per thread by interval containment.
	struct Aggregate
	{
		uint64_t count = 0;
		int64_t total = 0;
		int64_t self = 0;
		int64_t longest = 0;
	};
	std::map<std::wstring, Aggregate> aggregates;

	struct Long
	{
		std::wstring name;
		int64_t duration;
		int64_t start;
		uint32_t thread;
	};
	std::vector<Long> longest;

	size_t event_count = 0;
	uint64_t dropped = 0;

	for (auto& thread : threads)
	{
		event_count += thread.events.size();
		dropped += thread.dropped;

		std::sort(thread.events.begin(), thread.events.end(), [](const Event& a, const Event& b)
		{
			return a.start_ns != b.start_ns ? a.start_ns < b.start_ns : a.end_ns > b.end_ns;
		});

		struct Open
		{
			size_t index;
			int64_t children;
		};
		std::vector<Open> stack;

		auto close = [&](const Open& open)
		{
			const Event& e = thread.events[open.index];
			const int64_t duration = e.end_ns - e.start_ns;
			Aggregate& a = aggregates[event_name(e)];
			a.count++;
			a.total += duration;
			a.self += duration - open.children;
			a.longest = std::max(a.longest, duration);
		};

		for (size_t i = 0; i < thread.events.size(); i++)
		{
			const Event& e = thread.events[i];
			while (!stack.empty() && thread.events[stack.back().index].end_ns <= e.start_ns)
			{
				close(stack.back());
				stack.pop_back();
			}

			const int64_t duration = e.end_ns - e.start_ns;
			if (!stack.empty())
				stack.back().children += duration;
			stack.push_back({ i, 0 });

			if (duration >= 1000000)
				longest.push_back({ event_name(e), duration, e.start_ns - begun, thread.id });
		}
		while (!stack.empty())
		{
			close(stack.back());
			stack.pop_back();
		}
	}

	// The Chrome trace.
	std::string path;
	try
	{
		std::filesystem::create_directories("captures");
		path = "captures/" + file_safe(name) + "_" + std::to_string(capture_count++) + ".json";

		std::ofstream out(path);
		out.setf(std::ios::fixed);
		out.precision(3);
		out << "{\"traceEvents\":[{\"name\":\"process_name\",\"ph\":\"M\",\"pid\":1,\"args\":{\"name\":\"Spectrum\"}}";

		for (auto& thread : threads)
			out << ",{\"name\":\"thread_name\",\"ph\":\"M\",\"pid\":1,\"tid\":" << thread.id << ",\"args\":{\"name\":\""
				<< (thread.is_caller ? std::string("capture thread") : "thread " + std::to_string(thread.id)) << "\"}}";

		for (auto& thread : threads)
			for (const Event& e : thread.events)
			{
				const int64_t start = std::max<int64_t>(e.start_ns - begun, 0);
				out << ",{\"name\":\"" << json_escape(convert(event_name(e))) << "\",\"ph\":\"X\",\"pid\":1,\"tid\":" << thread.id
					<< ",\"ts\":" << double(start) / 1000.0 << ",\"dur\":" << double(e.end_ns - e.start_ns) / 1000.0 << "}";
			}
		out << "]}";
	}
	catch (const std::exception& e)
	{
		Log::get() << "capture: cannot write the trace: " << e.what() << Log::endl;
		path.clear();
	}

	// The summary.
	const std::string prefix = "capture '" + name + "': ";
	Log::get() << prefix << ms_text(ended - begun) << " ms, " << event_count << " scopes on " << threads.size() << " threads"
		<< (dropped ? ", " + std::to_string(dropped) + " dropped" : std::string())
		<< (path.empty() ? std::string() : ", trace " + path) << Log::endl;

	std::vector<std::pair<std::wstring, Aggregate>> by_self(aggregates.begin(), aggregates.end());
	std::sort(by_self.begin(), by_self.end(), [](const auto& a, const auto& b) { return a.second.self > b.second.self; });

	Log::get() << prefix << "by self time (ms):      self    total    count      max" << Log::endl;
	for (size_t i = 0; i < std::min<size_t>(by_self.size(), 25); i++)
	{
		const Aggregate& a = by_self[i].second;
		Log::get() << prefix << "  " << padded(ms_text(a.self), 9) << padded(ms_text(a.total), 9)
			<< padded(std::to_string(a.count), 9) << padded(ms_text(a.longest), 9) << "  " << convert(by_self[i].first) << Log::endl;
	}

	std::sort(longest.begin(), longest.end(), [](const Long& a, const Long& b) { return a.duration > b.duration; });
	Log::get() << prefix << "longest single scopes (start ms, duration ms, thread):" << Log::endl;
	for (size_t i = 0; i < std::min<size_t>(longest.size(), 15); i++)
		Log::get() << prefix << "  " << padded(ms_text(longest[i].start), 9) << padded(ms_text(longest[i].duration), 9)
			<< padded(std::to_string(longest[i].thread), 4) << "  " << convert(longest[i].name) << Log::endl;
}

TimingCapture::Scope::Scope(std::string label, unsigned int frames_after)
	: owner(TimingCapture::get().begin(std::move(label))), frames_after(frames_after)
{
}

TimingCapture::Scope::~Scope()
{
	if (!owner)
		return;

	auto& capture = TimingCapture::get();
	if (frames_after)
		capture.end_after_frames(frames_after);
	else
		capture.end();
}
