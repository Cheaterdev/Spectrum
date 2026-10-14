export module Core:Executors;

import :Profiling;
import :Threading;
import :Singleton;
import stl.core;
import stl.threading;
import ppl;

using namespace concurrency;

export
{

class SingleThreadExecutor : public concurrency::agent, public concurrency::unbounded_buffer<std::function<void()>>

{
public:
	explicit SingleThreadExecutor(bool start = true) : stopped(!start)
	{
		if (start)
			concurrency::agent::start();
	}

	// The worker blocks on its queue until told to stop. Destroying the executor without stopping it
	// (an exception unwinding through its owner) would otherwise wait forever on the agent's join, which
	// the ppl shim does where Windows' agent destructor does not.
	// Defined in Executors.cpp: an in-class body here is emitted again by every module that imports Core
	// (MSVC: LNK2005 against hal.lib).
	~SingleThreadExecutor();

	void stop_and_wait();

protected:
	void run();

private:
	bool stopped = false;
};


//rewrite this :(
using Batch = std::vector<std::function<void()>>;
class SingleThreadExecutorBatched: public SingleThreadExecutor

{
//	concurrency::task<void> task;

	std::mutex m;
	std::queue<int> q;

	//boost::lockfree::spsc_queue<int, boost::lockfree::capacity<7> > spsc_queue;
public:
	explicit SingleThreadExecutorBatched(int count = 16): SingleThreadExecutor(false)
	{
		concurrency::agent::start();
	}

	void flush();

	void stop_and_wait();

	void add(std::function<void()>&& f);

protected:
	Batch current_batch;

	void process_one(Batch& batch);

};

}
