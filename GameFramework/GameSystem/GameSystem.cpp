#include "pch.h"

GameSystem::GameSystem()
{
	//pScene = shared_ptr<UScene>(new UScene);
}

void GameSystem::AddObject(UObject::Ptr pObject)
{
	/*pScene->AddObject(pObject);

	auto pPlayerController = dynamic_pointer_cast<Controller>(pObject);
	if (pPlayerController)
		pController = pPlayerController;*/
}

void GameSystem::ExecuteTask()
{
/*	// Get delta time
	timer.updateState();
	float DeltaTime = (float)timer.getDeltaTime();

	// get objects
	auto objects = pScene->GetObjects();

	TickObjectGroup(objects, DeltaTime, ETG_PreSync);	
	PreSynchronizeData();

	// Default tick groups
	TickObjectGroup(objects, DeltaTime, ETG_TickPreDefaultTick2);
	TickObjectGroup(objects, DeltaTime, ETG_TickPreDefaultTick1);
	TickObjectGroup(objects, DeltaTime, ETG_TickDefault);
	TickObjectGroup(objects, DeltaTime, ETG_TickPostDefaultTick1);
	TickObjectGroup(objects, DeltaTime, ETG_TickPostDefaultTick2);

	PostSynchronizeData();
	TickObjectGroup(objects, DeltaTime, ETG_PostSync);*/
}