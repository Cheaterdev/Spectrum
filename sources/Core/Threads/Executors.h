#pragma once
import Profiling;
import Threading;
import Singleton;
#include <boost/lockfree/spsc_queue.hpp>

import ppl;
using namespace concurrency;


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
using Batch = std::vector<std::function<void()>>;
class SingleThreadExecutorBatched

{
	concurrency::task<void> task;
	boost::lockfree::spsc_queue<int, boost::lockfree::capacity<7> > spsc_queue;
public:
	explicit SingleThreadExecutorBatched(int count = 16)
	{
		for (auto& d : datas)
			d.reserve(count);

		index = 0;

		task = create_task([this]
			{
				run();
			});
	}

	void flush();

	void stop_and_wait();

	void add(std::function<void()>&& f);

protected:
	std::array<Batch, 8> datas;
	int index;

	void run();


};
