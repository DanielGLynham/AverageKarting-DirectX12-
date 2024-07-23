#include "PickUp.h"


PickUp::PickUp()
{
	pickUpActive = true;
}
void PickUp::Initialise(MeshManager& mgr)
{
	Mesh& ms = mMeshMgr.CreateMesh("questionMark");
	ms.CreateFrom("../bin/data/QuestionMark/questionMark.obj", gd3dDevice, mFX.mCache);
	mCube.Initialise(ms);

	MaterialExt *pMat = &mCube.GetMesh().GetSubMesh(0).material;
	pMat->gfxData.Set(Vector4(1, 1, 0, 0), Vector4(1, 0, 0, 0), Vector4(1, 0, 0, 0));
	mCube.SetOverrideMat(pMat);
	mCube.GetScale() = Vector3(0.1f, 0.1f, 0.1f);
}
void PickUp::Render(FX::MyFX& fx, float dTime)
{
	fx.Render(mCube, gd3dImmediateContext);
}
//game logic, called constantly, elapsed time passed in
void PickUp::Update(float dTime)
{
	if (pickUpActive == false)
	{
		respawnTimer -= dTime;
	}
	if (respawnTimer <= 0)
	{
		respawnTimer = 20.0f;
		pickUpActive = true;
	}
	if (mCube.GetPosition().y > 1 && movingUp) // toggles to go down
	{
		moveForce *= -1;
		movingUp = false;
	}
	if (mCube.GetPosition().y < 0.5f && !movingUp) // toggles to go up
	{
		moveForce *= -1;
		movingUp = true;
	}
	mCube.GetPosition().y += moveForce; // moves the pickUp up and down
	mCube.GetRotation().y += 0.005; // rotates the pickUp
}
void PickUp::Collect()
{
	pickUpActive = false;
}
void PickUp::SetActivity(bool toggle)
{
	pickUpActive = toggle;
}
bool PickUp::GetActivity()
{
	return pickUpActive;
}