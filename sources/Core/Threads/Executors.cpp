module Core:Executors;

import stl.core;
import stl.threading;
import ppl;

using namespace concurrency;

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
	PROFILE(L"SingleThreadExecutor::run()");

	std::function<void()> n;
	while ((n = receive(*this)) != nullptr)
	{
		PROFILE(L"work");

		n();
	}

	done();
}
