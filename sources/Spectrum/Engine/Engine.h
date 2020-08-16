#pragma once
/*
class Engine : public Singleton<Engine>
{
public:
	Engine();
	virtual ~Engine();

	Scheduler::ptr GetScheduler() const { return pScheduler; }

	void RegisterSystem(base_system::ptr pSystem);

	void Run();
protected:
	std::vector<base_system::ptr> Systems;

private:
	Scheduler::ptr pScheduler;
};*/
#define GEngine Engine::get()