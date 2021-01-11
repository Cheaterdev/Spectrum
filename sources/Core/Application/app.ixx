#include <string>
#include <array>
#include <vector>
#include <typeinfo>
#include <algorithm>
#include <fstream>
#include <map>
#include <unordered_map>
#include <set>
#include <assert.h>
#include <memory>
#include <thread>
#include <sstream>
#include <mutex>
#include <atomic>
#include <type_traits>
#include <queue>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <queue>
#include <optional>
#include <compare>
#include <span>
#include <stack>
#include <ranges>
#include <numeric>

#include "patterns/Singleton.h"

export module app;

export class Application : public Singleton<Application>
{
    std::atomic<bool> alive;
    friend class Singleton<Application>;
protected:
    Application();
    virtual ~Application();

    /** Application events */
    virtual void on_tick();

    std::chrono::steady_clock::duration tick_interval;
public:
    void run();
    void shutdown();
    bool is_alive() const { return alive; }

};



Application::Application()
{
    alive = true;
    tick_interval = std::chrono::milliseconds(50);
}

Application::~Application()
{
    alive = false;
}


void Application::on_tick()
{
}

void Application::run()
{
    while (alive)
    {
        on_tick();
    }
}

void Application::shutdown()
{
    alive = false;
}