#pragma once

class GameSystem : public base_system
{
public:
	GameSystem();

	void AddObject(UObject::Ptr pObject);

	std::shared_ptr<Controller> GetPlayerController() { return pController.lock(); }

	void ExecuteTask();
	void PreSynchronizeData();
	void PostSynchronizeData();
protected:

	std::weak_ptr<Controller> pController;
};