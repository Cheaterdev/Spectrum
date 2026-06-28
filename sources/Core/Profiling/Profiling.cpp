module Core:Profiling;

import stl.core;
import :Log;

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
	if (auto* l = cpu_listener.load(std::memory_order_relaxed)) l->on_cpu_start(timer->block);
}

void Profiler::on_end(Timer* timer)
{
	auto* blk = timer->block;
	if (blk >= s_block_pool.data() && blk < s_block_pool.data() + POOL_SIZE)
		--pool_head;
	current_block = blk->parent;
	blk->cpu_counter.end_time = std::chrono::high_resolution_clock::now();
	if (auto* l = cpu_listener.load(std::memory_order_relaxed)) l->on_cpu_end(blk);
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
