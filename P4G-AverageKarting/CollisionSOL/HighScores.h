#ifndef HIGHSCORES_H
#define HIGHSCORES_H

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

//wrap up comon behaviours, initialisation+shutdown
class HighScores
{
public:
	//start up and shut down
	HighScores() {}
	~HighScores() { Release(); }
	//game logic, called constantly, elapsed time passed in
	void Update(float dTime);
	//render images, called constantly, elapsed time passed in
	void Render(float dTime, float highScores1, float highScores2, float highScores3, float highScores4, float highScores5, float highScores6, float highScores7, float highScores8, float highScores9, float highScores10);
	//called when ALT+ENTER or drag
	void OnResize(int screenWidth, int screenHeight);
	void Initialise(ModeManager* mmPtr);
	void Release();


	//message handling
	LRESULT WindowsMssgHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


	//camera
	const DirectX::SimpleMath::Vector3 mDefCamPos = DirectX::SimpleMath::Vector3(0, 0.5f, -5);
	DirectX::SimpleMath::Vector3 mCamPos = DirectX::SimpleMath::Vector3(0, 0.5f, -5);

	//ensure each mesh is loaded and release exactly once
	MeshManager mMeshMgr;
	//game models that reference meshes
	Model mQuad, mTorch, mCube, mRock, mSphere, mOption;
	//handy rendering state
	FX::MyFX mFX;

	MouseAndKeys& GetMouseAndKeys() {
		return mMKInput;
	}
	//void CollisionManager(const BoundingBox& box, const BoundingSphere& sphere, float vel, Vector3& pos, float COR, float dTime);
private:
	HighScores& operator=(const HighScores&) = delete;
	HighScores(const HighScores& m) = delete;

	//load meshes
	void Load();
	//display a message
	void LoadDisplay(float dTime);

	//light position
	DirectX::SimpleMath::Vector3 mLightPos = DirectX::SimpleMath::Vector3(0, 0, 0);
	//text
	DirectX::SpriteBatch *mpSpriteBatch = nullptr;
	DirectX::SpriteFont *mpFont = nullptr, *mpFont2 = nullptr;

	//loading handler
	struct LoadData
	{
		//second thread
		std::future<void> loader;
		//how many meshes to load
		int totalToLoad = 0;
		//tally
		int loadedSoFar = 0;
		//is the loader active
		bool running = false;
	};
	LoadData mLoadData;
	MouseAndKeys mMKInput;
	Gamepad mGamepad;

	bool changeState = false;
	int nextState = 0;

	ModeManager* mmPtr = nullptr;
};
#endif