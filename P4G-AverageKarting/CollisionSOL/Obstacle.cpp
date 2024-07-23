#include "Obstacle.h"


Obstacle::Obstacle()
{
}
void Obstacle::Initialise(MeshManager& mgr, bool wantToMove)
{
	this->wantToMove = wantToMove;
	Mesh *p = mgr.GetMesh("cube");
	assert(p);
	mCube.Initialise(*p);

	MaterialExt *pMat = &mCube.GetMesh().GetSubMesh(0).material;
	pMat->gfxData.Set(Colours::Red, Vector4(1,0,0,0), Vector4(1, 0, 0, 0));
	mCube.SetOverrideMat(pMat);
	mCube.GetScale() = Vector3(0.1f, 0.1f, 0.1f);

}
void Obstacle::Render(FX::MyFX& fx, float dTime)
{
	fx.Render(mCube, gd3dImmediateContext);
}
//game logic, called constantly, elapsed time passed in
void Obstacle::Update(float dTime)
{
	if (wantToMove)
	{
		if (waitOver) // move
		{
			if (mCube.GetPosition().y < minYPos || mCube.GetPosition().y > maxYPos)
			{
				movement *= -1;
				waitOver = false;
			}
			mCube.GetPosition().y += movement;
		}
		else // wait to move
		{
			randomWaitTime -= dTime;
			if (randomWaitTime <= 0)
			{
				waitOver = true;
				randomWaitTime = rand() % 30;
			}
		}
	}
}

void Obstacle::SetWantToMove(bool wantToMove)
{
	this->wantToMove = wantToMove;
}