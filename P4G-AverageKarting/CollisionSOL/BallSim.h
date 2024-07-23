#ifndef BALLSIM_H
#define BALLSIM_H


#include "Mesh.h"
#include "Model.h"
#include "Input.h"
#include "FX.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "Timer.h"
#include <vector>

#include <string>
#include <iomanip>
#include "AudioMgr.h"

#include "D3D.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

class BallSim
{
public:
	int laps;
	Model mCar,mCar2,mCar3,mCar4;
	Model mCube;
	int carNo = 0;
	DirectX::SimpleMath::Vector3 mVel = DirectX::SimpleMath::Vector3(0, 0, 0);
	DirectX::SimpleMath::Vector3 mGrav = DirectX::SimpleMath::Vector3(0, 0, 0);
	DirectX::SimpleMath::Vector3 mAccel = DirectX::SimpleMath::Vector3(0, 0, 0);
	float mCOR = 1;
	float mRadius = 0.1f;
	float turnSpeed = 0.0f;
	bool onTrack = true;
	DirectX::SimpleMath::Vector3 positionInc = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);

	enum class Mode { WAITING, LINEAR, LINEAR_ACCEL, BOUNCE_INF, BOUNCE, CUBE, CUBE_MOUSE, MULTI };
	Mode mMode, mLastMode;

	void Initialise(MeshManager& mgr, int carNo);
	void Start(int carNo);

	void Update(float dTime, int groundType, const DirectX::SimpleMath::Vector3& camPos, MouseAndKeys& input);
	void Render(FX::MyFX& fx, float dTime);
	void RenderText(DirectX::SpriteFont *pF, DirectX::SpriteBatch *pBatch);
	bool BallSim::GetPoliceCar();
	bool BallSim::GetCarShrink();
	bool BallSim::GetCarGrow();
	DirectX::SimpleMath::Vector3 GetRotation();
	DirectX::SimpleMath::Vector3 GetPos();
	void SetCarPosition(DirectX::SimpleMath::Vector3 dPos);
	void TurnCar(float rotation);
	Model GetCar(), GetCar2(), GetCar3(), GetCar4();

	void Crash();
	void PickUpObtained(BallSim& car);
	std::vector<bool>& GetCheckPointsPassed();
	bool doesntCount = true;
	float lapTimer = 0;
	unsigned int shrinkHdl;
	unsigned int speedHdl;
	unsigned int policeHdl;
	unsigned int growHdl;
	int place;
	int helpfulCounter = 0;
	float currentBestTime = 0;
private:
	DirectX::SimpleMath::Vector3 camPos = DirectX::SimpleMath::Vector3(0, 0, 0);
	//----------------------------------------------------------------------------------------------------
	std::vector<Model> mCubes;
	MeshManager mMeshMgr;
	Gamepad mGamepad;
	float k = 0.0f;
	FX::MyFX mFX;
	// Movement Values -----------------------------------------------------------------------------------
	float enginePower;
	float inverseEP;
	const float epY1 = 0.75f;
	const float epY2 = 1.0F;
	const float epY3 = 0.5f;
	const float epX1 = 2.0f;
	const float epX2 = 4.0f;
	const float epX3 = 6.0f;
	const float epGrad1 = (epY1 - epY3) / epX1;
	const float epGrad2 = (epY2 - epY1) / (epX3 - epX1);
	const float epGrad3 = -epY2 / epX2;
	const float epIntersect2 = epY1 - (epX1 * epGrad2);

	//float acceleration;
	const float acY1 = 5.0f;
	//const float acY2 = 12.0f;
	const float acY3 = 50.0f;//15.0f;
	const float acX1 = 50.0f;//90.0f;
	const float acX2 = 130.0f;
	const float acX3 = 150.0f;
	//const float acGrad1 = (acY2 - acY3) / acX1;
	const float acGrad2 = (acY1 - acY3) / (acX2 - acX1);
	const float acGrad3 = -acY1 / (acX3 - acX2);
	const float acIntersect2 = acY3 - (acX1 * acGrad2);
	const float acIntersect3 = acY1 - (acX2 * acGrad3);

	//float deceleration;
	const float dcY1 = 15.0f;
	const float dcY2 = 25.0f;
	const float dcY3 = 40.0f;
	const float dcX1 = 20.0f;
	const float dcX2 = 60.0f;
	const float dcX3 = 150.0f;
	const float dcGrad1 = (dcY1 - dcY3) / dcX1;
	const float dcGrad2 = (dcY2 - dcY1) / (dcX2 - dcX1);
	const float dcIntersect2 = dcY1 - (dcX1 * dcGrad2);

	bool prevAc;		// True if car was accelerating in the last frame
	float epTimer;

	const float maxSpeed = 150.0f;
	float NFSpeed = 0.0f;		// No Friction Speed
	float speed = 0.0f;

	float frictionMultiplier = 1.0f;
	float mAngle = 45;
	bool speedBoost = false;
	bool policeCar = false;
	bool carShrink = false;
	bool carGrow = false;
	bool exhaustFumes = false;

	float speedCooldown = 2.0f;
	float policeCooldown = 6.0f;
	float shrinkCooldown = 2.0f;
	float growCooldown = 5.0f;
	const float maxSpeedCooldown = 2.0f;
	const float maxPoliceCooldown = 6.0f;
	const float maxShrinkCooldown = 2.0f;
	const float maxGrowCooldown = 2.0f;
	const float accAboveMaxMult = 7.5f;


	int prevGroundType = 1;

	bool isDrifting = false;

	bool ChangeEnginePower(float dTime, bool accelerating);
	float CalculateAcceleration(float s, float fM, float iFM);
	float CalculateDeceleration(float s, float fM, float iFM);
	void Accelerate(float dTime, float fM, float iFM);
	void Decelerate(float dTime, float fM, float iFM);

	Timer pickUpTimer;
	std::vector<bool> checkPointsPassed;

	const int MAX_CUBES = 8;
	const float CUBE_SCALE = 0.2f;
};
class Gamepads
{
public:
	//gamepad data, one per pad
	struct State
	{
		int port = -1;
		float leftStickX = 0;
		float leftStickY = 0;
		float rightStickX = 0;
		float rightStickY = 0;
		float leftTrigger = 0;
		float rightTrigger = 0;
		float deadzoneX = 0.1f;
		float deadzoneY = 0.1f;
		XINPUT_STATE state;
	};
	//as a mechanical device the pad may generate data even when
	//nobody is touching it, especially if it's old. The deadzone
	//specifies a small range of input that will be ignored.
	//idx=which pad
	//x+y=range to ignore
	void SetDeadZone(int idx, float x, float y) {
		assert(idx >= 0 && idx < XUSER_MAX_COUNT);
		mPads[idx].deadzoneX = x;
		mPads[idx].deadzoneY = y;
	}
	//look at the state of a pad
	//idx=pad 0,1,2 or 3
	const State& GetState(int idx) const {
		assert(idx >= 0 && idx < XUSER_MAX_COUNT);
		return mPads[idx];
	}
	//called once at startup
	//check that a specific pad is still plugged in
	bool IsConnected(int idx) {
		assert(idx >= 0 && idx < XUSER_MAX_COUNT);
		return mPads[idx].port != -1;
	}
	//called every update
	void Update();

	bool IsPressed(int idx, unsigned short buttonId);
	//get a list of pad activity - motion of sticks and triggers
	void GetAnalogueNames(int idx, std::wstring& mssg);
	//get a list of pad activity - button presses
	void GetDigitalNames(int idx, std::wstring& mssg);
private:
	//a copy of state for each of 4 pads
	State mPads[XUSER_MAX_COUNT];

};
#endif
