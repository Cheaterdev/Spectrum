#include "pch.h"

export module app;

import Singletons;
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