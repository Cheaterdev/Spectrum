#pragma once

struct CameraDataStructure
{
	float speed;
	float2 cVerHor;
	float2 aVerHor;
	float2 zNearFar;
};
/*
class Camera : public GameObject
{
public:
	Camera();

	/*void SetViewDirection(Vec3 dir);
	void SetAngles(float2 angleVH);

	void setZNearFar(float2 planeZNearFar) { CameraData->zNearFar = planeZNearFar; }
	void setCVerHor(float2 cVH) { CameraData->cVerHor = cVH; }

	Vec3 GetPosition() const { return objData->position; }
	float GetSpeed() const { return CameraData->speed; }
	float2 GetZNearFar() const { return CameraData->zNearFar; }
	float2 GetCVehHor() const { return CameraData->cVerHor; }
	*
	// movement
//	void angleVerHorChange(float2 angles);
protected:
//	void RegisterAllDataWrappers();

	//ObjDataWrapper<CameraDataStructure> CameraData;
};*/