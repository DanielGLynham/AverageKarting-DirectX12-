#ifndef MENU_H
#define MENU_H

#define _USE_MATH_DEFINES

#include <vector>
#include <future>
#include <math.h>
#include "Mesh.h"
#include "Model.h"
#include "FX.h"
#include "SpriteFont.h"
#include "Input.h"
#include "ModeManager.h"
#include <sstream>
#include <thread>
#include <iomanip>
#include "WindowUtils.h"
#include "D3D.h"
#include "GeometryBuilder.h"
#include "CommonStates.h"




using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

class Game;

//wrap up comon behaviours, initialisation+shutdown
class Menu
{
public:
	//start up and shut down
	Menu() {}
	~Menu() { Release(); }
	//game logic, called constantly, elapsed time passed in
	void Update(float dTime);
	//render images, called constantly, elapsed time passed in
	void Render(float dTime);
	//called when ALT+ENTER or drag
	void OnResize(int screenWidth, int screenHeight);
	void Initialise(ModeManager* mmPtr, Game& game);
	void Release();

	//message handling
	LRESULT WindowsMssgHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//camera
	const DirectX::SimpleMath::Vector3 mDefCamPos = DirectX::SimpleMath::Vector3(0, 0.5f, -5);
	DirectX::SimpleMath::Vector3 mCamPos = DirectX::SimpleMath::Vector3(0, 0.5f, -5);

	//ensure each mesh is loaded and release exactly once
	MeshManager mMeshMgr;
	//game models that reference meshes
	Model mQuad, mTorch, mCube, mRock, mSphere, mOption, mCar, raceCarOne, raceCarTwo;
	float raceCarOneVel = (rand() % 4) + 5;
	float raceCarTwoVel = (rand() % 4) + 5;
	float racingcarsTimer = 5;
	bool raceCarOneFinished = true;
	bool raceCarTwoFinished = true;
	//handy rendering state
	FX::MyFX mFX;

	MouseAndKeys& GetMouseAndKeys() {
		return mMKInput;
	}

	//load meshes
	void Load();


private:
	Menu& operator=(const Menu&) = delete;
	Menu(const Menu& m) = delete;

	//display a message
	void LoadDisplay(float dTime);

	//light position
	DirectX::SimpleMath::Vector3 mLightPos = DirectX::SimpleMath::Vector3(0, 0, 0);
	//text
	DirectX::SpriteBatch *mpSpriteBatch = nullptr;
	DirectX::SpriteFont *mpFont = nullptr, *mpFont2 = nullptr;

	float rotationCarY = 0.0f;

	MouseAndKeys mMKInput;
	Gamepad mGamepad;

	bool changeState = false;
	int nextState = 0;
	bool moveObstaclesClicked = false;

	ModeManager* mmPtr = nullptr;
	Game *mpGame = nullptr;
};
#endif