#ifndef GAME_H
#define GAME_H

#define _USE_MATH_DEFINES

#include <vector>
#include <future>
#include <math.h>
#include "FPSCamera.h"
#include "Mesh.h"
#include "Model.h"
#include "FX.h"
#include "SpriteFont.h"
#include "Input.h"
#include "BallSim.h"
#include "Timer.h"
#include "PickUp.h"
#include <string>
#include "ModeManager.h"
#include "Obstacle.h"
#include "WindowUtils.h"
#include "D3D.h"
#include "GeometryBuilder.h"
#include "CommonStates.h"
#include <iomanip>
#include <sstream>
#include <thread>

#include "Menu.h"


//wrap up comon behaviours, initialisation+shutdown
class Game
{
public:
	//start up and shut down
	Game() {}
	~Game() {
		Release();
	}
	//game logic, called constantly, elapsed time passed in
	void Update(float dTime);
	//render images, called constantly, elapsed time passed in
	void Render(float dTime);
	//called when ALT+ENTER or drag
	void OnResize(int screenWidth, int screenHeight);
	void Initialise(ModeManager* mmPtr, Menu& menu);
	void Release();
	
	
	//message handling
	LRESULT WindowsMssgHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	
	//camera
	const DirectX::SimpleMath::Vector3 mDefCamPos = DirectX::SimpleMath::Vector3(0, 0.5f, -5);
	DirectX::SimpleMath::Vector3 mCamPos = DirectX::SimpleMath::Vector3(0, 0.5f, -5);
	
	//ensure each mesh is loaded and release exactly once
	MeshManager mMeshMgr;
	//game models that reference meshes
	Model mQuad, mTorch, mCube, mRock, mSphere, mSkybox;
	//handy rendering state
	FX::MyFX mFX;
	
	struct CheckPoint
	{
		int x;
		int y;
		bool used;
	};
	int isOnTrack(Model& car);
	MouseAndKeys& GetMouseAndKeys() {
		return mMKInput;
	}

	float GetHighScores(int pos);
	void SetAHighScore(float highScore);
	void SetHighScores(int pos, float highScore);
	void CheckPointsConnectedCheck(CheckPoint point, vector<CheckPoint>& allCheckPoints);

	void ResetGame();
	void SortPositions();
	float currentBestTime = 0;

	bool FinishedLoading();

	void SetMoveObstacles(bool moveObstacles);

private:

	

	Game& operator=(const Game&) = delete;
	Game(const Game& m) = delete;

	//load meshes
	void Load();
	//display a message
	void LoadDisplay(float dTime);

	DirectX::SimpleMath::Vector3 FPScamPos();
	//light position
	DirectX::SimpleMath::Vector3 mLightPos = DirectX::SimpleMath::Vector3(0, 0, 0);
	//text
	DirectX::SpriteBatch *mpSpriteBatch = nullptr;
	DirectX::SpriteFont *mpFont = nullptr, *mpFont2 = nullptr;

	const float camRadius = 3.0f;
	void LoadMap();
	
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
	BallSim mCar,mCar2,mCar3,mCar4,mBall;
	BallSim cars[4] = 
	{
		mCar,
		mCar2,
		mCar3,
		mCar4
	};

	FPSCamera mCamera, mCamera2,mCamera3,mCamera4;
	FPSCamera cameras[4] =
	{
		mCamera, 
		mCamera2,
		mCamera3,
		mCamera4
	};
	
	float highScores[10];
	
	Matrix gViewMat1, gViewMat2, gViewMat3, gViewMat4;
	Matrix gViews[4] =
	{
		gViewMat1,
		gViewMat2,
		gViewMat3,
		gViewMat4
	};

	struct MapData
	{
		bool track = false;  //red pixels
		bool gravel = false;	//green pixel
		bool checkpoint = false; //blue pixel
		bool obstacle = false; // yellow
		bool pickUp = false; // very light blue?
	};
	static const int MAP_WIDTH = 128;
	MapData mTrack[MAP_WIDTH][MAP_WIDTH];
	bool isOn = false;
	vector<vector<CheckPoint>> checkpoints;
	vector<Obstacle> mObstacles;
	ModeManager* mmPtr = nullptr;

	Matrix scaling = Matrix::CreateScale(2.0f, 1.0f, 1.0f);

	Vector3 cScaleSmall = Vector3(2.0f, 2.0f, 2.0f);
	Vector3 cScaleBig = Vector3(0.5f, 0.5f, 0.5f);

	Menu *mpMenu = nullptr;
	bool mInitialised = false;

	
};

#endif




