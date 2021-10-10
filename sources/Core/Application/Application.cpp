#include "pch_core.h"
#include "Application.h"

Application::Application()
{
    alive = true;
    tick_interval = std::chrono::milliseconds(50);
    //  Engine::create();
}

Application::~Application()
{
    alive = false;
//   Engine::reset();
}


void Application::on_tick()
{
}

void Application::run()
{
    while (alive)
    {
        on_tick();
        //  std::this_thread::sleep_for(tick_interval);
    }
}

void Application::shutdown()
{
    alive = false;
}