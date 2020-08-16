#pragma once

/**
 * Describe base system in engine (AI, Render, Physics, Animation, etc.)
 */
class base_system abstract
{
public:
	virtual void execute_task() = 0;
	uscene::ptr get_scene() { return pScene; }

	typedef std::shared_ptr<base_system> ptr;
protected:
	uscene::ptr pScene;
};