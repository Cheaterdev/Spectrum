module Core:Application;

Application::Application()
{
	alive = true;
	tick_interval = 50_ms;
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
