#ifndef PICKUP_H
#define PICKUP_H

#define _USE_MATH_DEFINES

#include "D3D.h"
#include <vector>
#include <math.h>
#include "GeometryBuilder.h"
#include "SimpleMath.h"
#include "ShaderTypes.h"


#include "Mesh.h"
#include "Model.h"
#include "FX.h"
#include <string>
#include <iomanip>

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//wrap up comon behaviours, initialisation+shutdown
class PickUp
{
public:
	PickUp();
	void Initialise(MeshManager& mgr);
	void Render(FX::MyFX& fx, float dTime);
	void Update(float dTime);
	void Collect();
	Model mCube;
	void SetActivity(bool toggle);
	bool GetActivity();

private:
	MeshManager mMeshMgr;
	FX::MyFX mFX;
	bool pickUpActive;
	bool movingUp = true;
	float moveForce = sinf(0.02f) * 0.05;
	float respawnTimer = 20.0f;
};

#endif




