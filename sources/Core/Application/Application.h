#pragma once
#include "patterns/Singleton.h"

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

};