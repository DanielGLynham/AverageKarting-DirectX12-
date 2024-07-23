#ifndef Obstacle_H
#define Obstacle_H
#define _USE_MATH_DEFINES

#include <vector>
#include <future>
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
class Obstacle
{
public:
	Obstacle();
	void Initialise(MeshManager& mgr, bool wantToMove);
	void Render(FX::MyFX& fx, float dTime);
	void Update(float dTime);
	void SetWantToMove(bool wantToMove);
	Model mCube;
private:
	float moveUpTimer = 20.0f, moveDownTimer = 10.0f;
	float maxYPos = 5, minYPos = 0.5f;
	float movement = 0.01f;
	float randomWaitTime = rand() % 30;
	bool waitOver = false, wantToMove;
};

#endif




