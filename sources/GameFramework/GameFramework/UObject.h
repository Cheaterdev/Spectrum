#pragma once

enum class ETickGroups
{
	ETG_PreSync, // Before pre synchronization phase

	ETG_TickPreDefaultTick2,
	ETG_TickPreDefaultTick1,
	ETG_TickDefault,
	ETG_TickPostDefaultTick1,
	ETG_TickPostDefaultTick2,

	ETG_PostSync, // After post synchronization phase

	ETG_MaxTickGroup
};

enum ESyncType
{
	EST_PreSyncData = 1,
	EST_PostSyncData = 2,
	EST_SyncAll = EST_PreSyncData | EST_PostSyncData,
};

class UObject
{
public:
	UObject();
	virtual ~UObject() {}

	typedef std::shared_ptr<UObject> Ptr;

//	virtual void Destroy();

	// Tick block
	void TickGroup(ETickGroups value) { CurrentTickGroup = value; }
	ETickGroups TickGroup() const { return CurrentTickGroup; }

	virtual void Tick(float DeltaTime);

	// Synchronization block
	void Priority(short value) { priority = value; }
	short Priority() const { return priority; }

	virtual void Synchronize(ESyncType type);
	virtual void SetGlobalResourceObject(std::weak_ptr<UObject> pGlobalObject);

	// Transform block
	void Position(vec3 value) { position = value; }
	vec3 Position() const { return position; }

	void Rotation(quat value) { rotation = value; }
	quat Rotation() const { return rotation; }
	void SetEulerAngles(vec3 eulerAngles);

	mat4x4 GetTransform();
protected:
	vec3 position;
	quat rotation;

	ETickGroups CurrentTickGroup;
	short priority; // Object data priority, used by sync functions

	//bool DestroyMark;

	// Link to global resource object
	std::weak_ptr<UObject> pGlobalResourceObject;
};