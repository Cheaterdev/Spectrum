#include "pch.h"

void uscene::AddObject(UObject::Ptr obj)
{
	EnterCriticalSection(&csObjects);

	auto iter = find(objects.begin(), objects.end(), obj);
	if (iter == objects.end())
		objects.push_back(obj);

	LeaveCriticalSection(&csObjects);
}

void uscene::RemoveObject(UObject::Ptr obj)
{
	EnterCriticalSection(&csObjects);

	auto iter = find(objects.begin(), objects.end(), obj);
	if (iter != objects.end())
		objects.erase(iter);

	LeaveCriticalSection(&csObjects);
}

void uscene::Synchronize(ESyncType type)
{
	EnterCriticalSection(&csObjects);

	for (auto pObj : objects)
		pObj->Synchronize(type);

	LeaveCriticalSection(&csObjects);
}

void uscene::TickObjectGroup(std::vector<UObject::Ptr>& Objects, float DeltaTime, ETickGroups TickGroup)
{
	for (auto pObject : Objects)
	{
		if (pObject->TickGroup() == TickGroup)
			pObject->Tick(DeltaTime);
	}
}