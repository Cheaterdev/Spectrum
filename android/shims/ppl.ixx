// Android stand-in for Modules/ppl/ppl.ixx: the Concurrency Runtime (PPL / Agents
// library) exists only on Windows. Core uses just concurrency::agent and
// unbounded_buffer (SingleThreadExecutor), so that is all this provides.
export module ppl;

import stl.core;
import stl.threading;

export namespace concurrency {

template<class T>
class unbounded_buffer
{
public:
    bool enqueue(const T& item)
    {
        {
            std::lock_guard lock(mutex_);
            queue_.push_back(item);
        }
        ready_.notify_one();
        return true;
    }

    T dequeue()
    {
        std::unique_lock lock(mutex_);
        ready_.wait(lock, [this] { return !queue_.empty(); });
        T item = std::move(queue_.front());
        queue_.pop_front();
        return item;
    }

private:
    std::mutex mutex_;
    std::condition_variable ready_;
    std::deque<T> queue_;
};

template<class T>
T receive(unbounded_buffer<T>& source) { return source.dequeue(); }

// ---- tasks: the part of ppltasks.h the engine uses (create_task / then / wait / when_all).
// Each task runs on its own detached thread (PPL uses a pool); results are shared so a
// task can be copied and waited on from several places, like PPL's.
template<class T> class task;

template<class F>
auto create_task(F f);

template<class T>
class task
{
public:
    using result_type = T;

    task() = default;
    explicit task(std::shared_future<T> future) : future_(std::move(future)) {}

    T get() const { return future_.get(); }
    // Like PPL's, rethrows an exception the task died with; std::future::wait() alone would
    // swallow it and leave whatever the task was filling in (an engine PSO) silently null.
    void wait() const { future_.get(); }

    template<class F>
    auto then(F f) const
    {
        auto previous = future_;
        if constexpr (std::is_void_v<T>)
            return create_task([previous, f = std::move(f)]() mutable { previous.get(); return f(); });
        else
            return create_task([previous, f = std::move(f)]() mutable { return f(previous.get()); });
    }

private:
    std::shared_future<T> future_;
};

template<class F>
auto create_task(F f)
{
    using R = std::invoke_result_t<F>;
    auto job = std::make_shared<std::packaged_task<R()>>(std::move(f));
    std::shared_future<R> future = job->get_future().share();
    std::thread([job] { (*job)(); }).detach();
    return task<R>(std::move(future));
}

template<class It>
auto when_all(It first, It last)
{
    using T = typename std::iterator_traits<It>::value_type::result_type;
    std::vector<task<T>> tasks(first, last);
    if constexpr (std::is_void_v<T>)
        return create_task([tasks] { for (auto& t : tasks) t.wait(); });
    else
        return create_task([tasks] {
            std::vector<T> results;
            results.reserve(tasks.size());
            for (auto& t : tasks) results.push_back(t.get());
            return results;
        });
}

class agent
{
public:
    agent() = default;
    agent(const agent&) = delete;
    agent& operator=(const agent&) = delete;
    virtual ~agent() { join(); }

    bool start()
    {
        thread_ = std::thread([this] { run(); });
        return true;
    }

    static void wait(agent* a) { a->join(); }

protected:
    virtual void run() = 0;
    bool done() { return true; }

private:
    void join()
    {
        if (thread_.joinable() && thread_.get_id() != std::this_thread::get_id())
            thread_.join();
    }

    std::thread thread_;
};

}
