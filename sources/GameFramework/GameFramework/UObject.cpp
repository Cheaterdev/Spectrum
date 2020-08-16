#include "pch.h"

UObject::UObject()
{
	position = vec3(0, 0, 0);
	rotation = quat(0, 0, 0, 1);

	CurrentTickGroup = ETickGroups::ETG_TickDefault;
	priority = 0;

	//DestroyMark = false;
}
/*
void UObject::Destroy()
{
	DestroyMark = true;
}*/

void UObject::Synchronize(ESyncType type)
{
	auto pObj = pGlobalResourceObject.lock();
	if (pObj)
	{
		if (priority > pObj->priority)
		{
			// Save data
			if (type & ESyncType::EST_PostSyncData)
			{
				pObj->position = position;
				pObj->rotation = rotation;
			}
		}
		else if (priority < pObj->priority)
		{
			// Get data
			if (type & ESyncType::EST_PreSyncData)
			{
				position = pObj->position;
				rotation = pObj->rotation;
			}
		}
	}
}

void UObject::SetGlobalResourceObject(std::weak_ptr<UObject> pGlobalObject)
{
	pGlobalResourceObject = pGlobalObject;
}

void UObject::SetEulerAngles(vec3 eulerAngles)
{
	quat qx(vec3(1, 0, 0), eulerAngles.x);
	quat qy(vec3(0, 1, 0), eulerAngles.y);
	quat qz(vec3(0, 0, 1), eulerAngles.z);

	rotation = qz*qy*qx;
}

mat4x4 UObject::GetTransform()
{
	mat4x4 resultTransfrom;

	rotation.toMatrix(resultTransfrom);

	resultTransfrom.a41 = position.x;
	resultTransfrom.a42 = position.y;
	resultTransfrom.a43 = position.z;

	return resultTransfrom;
}