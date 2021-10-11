export module Application;

import Singleton;
import Utils;

export
{
    class Application : public Singleton<Application>
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

        virtual std::vector<std::string> file_open(const std::string& Name, const std::string& StartPath, const std::string& Extension)
        {
            return std::vector<std::string>();
        }
    };
}


module: private;

Application::Application()
{
    alive = true;
    tick_interval = 50_ms;
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