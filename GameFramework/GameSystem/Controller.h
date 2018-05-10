#pragma once

class Controller : public GameObject
{
public:
	void Tick(float DeltaTime);

	std::shared_ptr<Pawn> GetPawn() { return pPawn.lock(); }
//	std::shared_ptr<Camera> GetCamera() { return pCamera.lock(); }
protected:
	std::weak_ptr<Pawn> pPawn;
//	std::weak_ptr<Camera> pCamera;
};