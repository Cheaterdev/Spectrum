#include "pch.h"
/*
Camera::Camera()
{
	CameraData->speed = 10;
	CameraData->zNearFar = Vec2(0.1f, 1000);
	CameraData->aVerHor = Vec2(0, 0);
	CameraData->cVerHor = Vec2(0.25f*pi, 0.25f*pi); // for frustum
	CameraData.SetPriority(1);

	objData->position = Vec3(0, 0, 0);
	objData->rotation = Quaternion(0, 0, 0, 1);
	objData.SetPriority(1);

	RegisterAllDataWrappers();
}

void Camera::SetViewDirection(Vec3 dir)
{
	CameraData->aVerHor.x = -asin(dir.y);
	if (abs(dir.y)>0.99f)
		CameraData->aVerHor.y = 0;
	else
	{
		CameraData->aVerHor.y = acos(dir.z / sqrt(1 - sqr(dir.y)));
		CameraData->aVerHor.y *= fsign(dir.x);
	}

	SetEulerAngles(Vec3(CameraData->aVerHor.x, CameraData->aVerHor.y, 0));
}

void Camera::SetAngles(float2 angleVH)
{
	CameraData->aVerHor = angleVH;

	SetEulerAngles(Vec3(CameraData->aVerHor.x, CameraData->aVerHor.y, 0));
}

void Camera::angleVerHorChange(float2 angles)
{
	CameraData->aVerHor += angles;
	CameraData->aVerHor.x = clamp(CameraData->aVerHor.x, -pi, pi);
	if (CameraData->aVerHor.y<0 || CameraData->aVerHor.y>2 * pi)
		CameraData->aVerHor.y -= floorAlign(CameraData->aVerHor.y, 2 * pi);

	SetEulerAngles(Vec3(CameraData->aVerHor.x, CameraData->aVerHor.y, 0));
}

void Camera::RegisterAllDataWrappers()
{
	AllDataWrappers.push_back(pair<int, IObjData*>(CameraData.GetDataId(), &CameraData));
}*/