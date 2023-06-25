export module Core:Executors;

import :Profiling;
import :Threading;
import :Singleton;
//#include <boost/lockfree/spsc_queue.hpp>
import <stl/core.h>;
import <stl/threading.h>;

import ppl;
using namespace concurrency;

export
{

class SingleThreadExecutor : public concurrency::agent, public concurrency::unbounded_buffer<std::function<void()>>

{
public:
	explicit SingleThreadExecutor(bool start = true)
	{
		if (start)
			concurrency::agent::start();
	}

	void stop_and_wait();

protected:
	void run();

private:

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



void SingleThreadExecutorBatched::process_one(Batch& batch)
{
	for (auto& f : batch)
		if (f)
			f();
}

void SingleThreadExecutorBatched::flush()
{
	std::shared_ptr<Batch> process_batch = std::make_shared<Batch>();


	std::swap(current_batch, *process_batch);

	enqueue([this, process_batch]() {

		process_one(*process_batch);
		});

	current_batch.reserve(8);
}



void SingleThreadExecutorBatched::add(std::function<void()>&& f)
{

	current_batch.emplace_back(std::move(f));

	if (current_batch.capacity() == current_batch.size())
	{
		flush();
	}

}

void SingleThreadExecutorBatched::stop_and_wait()
{
	flush();
	SingleThreadExecutor::stop_and_wait();
}


void SingleThreadExecutor::stop_and_wait()
{
	enqueue(nullptr);
	concurrency::agent::wait(this);
}

void SingleThreadExecutor::run()
{
	std::function<void()> n;
	while ((n = receive(*this)) != nullptr)
	{
		n();
	}

	done();
}
