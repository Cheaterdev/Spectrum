#include "pch_core.h"
#include "Executors.h"

 void SingleThreadExecutorBatched::flush()
{
	while (!spsc_queue.push(index));

	index = (index + 1) % datas.size();
}

 void SingleThreadExecutorBatched::stop_and_wait()
{
	add(nullptr);
	flush();
	task.wait();

	spsc_queue.consume_all([&](int id)
		{
			for (auto& f : datas[id])
				if (f)
					f();

			datas[id].clear();
		});
}

 void SingleThreadExecutorBatched::add(std::function<void()>&& f)
{

	datas[index].emplace_back(std::move(f));

	if (datas[index].capacity() == datas[index].size())
	{
		flush();
	}

}

 void SingleThreadExecutorBatched::run()
{
	bool alive = true;
	while (alive)
		while (alive && spsc_queue.consume_one([&](int id)
			{
				for (auto& f : datas[id])
					if (f)
						f();
					else
						alive = false;

				datas[id].clear();
			}));
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
