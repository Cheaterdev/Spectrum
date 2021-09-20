#pragma once

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