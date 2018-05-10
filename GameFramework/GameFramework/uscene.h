#pragma once

class uscene
{
public:
	uscene();
	virtual ~uscene();

	void AddObject(UObject::Ptr obj);
	void RemoveObject(UObject::Ptr obj);

	void ProcessScene();

	void Synchronize(ESyncType type);

	typedef std::shared_ptr<uscene> ptr;
protected:
	void TickObjectGroup(std::vector<UObject::Ptr>& Objects, float DeltaTime, ETickGroups TickGroup);

	CRITICAL_SECTION csObjects;
	std::vector<UObject::Ptr> objects;
};