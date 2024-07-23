#include "BallSim.h"





void Gamepads::Update()
{
	for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
	{
		State& s = mPads[i];
		s.port = -1;
		ZeroMemory(&s.state, sizeof(XINPUT_STATE));
		if (XInputGetState(i, &s.state) == ERROR_SUCCESS)
		{
			float normLX = fmaxf(-1, (float)s.state.Gamepad.sThumbLX / 32767);
			float normLY = fmaxf(-1, (float)s.state.Gamepad.sThumbLY / 32767);

			s.leftStickX = (abs(normLX) < s.deadzoneX ? 0 : (abs(normLX) - s.deadzoneX) * (normLX / abs(normLX)));
			s.leftStickY = (abs(normLY) < s.deadzoneY ? 0 : (abs(normLY) - s.deadzoneY) * (normLY / abs(normLY)));

			if (s.deadzoneX > 0)
				s.leftStickX *= 1 / (1 - s.deadzoneX);
			if (s.deadzoneY > 0)
				s.leftStickY *= 1 / (1 - s.deadzoneY);

			float normRX = fmaxf(-1, (float)s.state.Gamepad.sThumbRX / 32767);
			float normRY = fmaxf(-1, (float)s.state.Gamepad.sThumbRY / 32767);

			s.rightStickX = (abs(normRX) < s.deadzoneX ? 0 : (abs(normRX) - s.deadzoneX) * (normRX / abs(normRX)));
			s.rightStickY = (abs(normRY) < s.deadzoneY ? 0 : (abs(normRY) - s.deadzoneY) * (normRY / abs(normRY)));

			if (s.deadzoneX > 0)
				s.rightStickX *= 1 / (1 - s.deadzoneX);
			if (s.deadzoneY > 0)
				s.rightStickY *= 1 / (1 - s.deadzoneY);

			s.leftTrigger = (float)s.state.Gamepad.bLeftTrigger / 255;
			s.rightTrigger = (float)s.state.Gamepad.bRightTrigger / 255;

			s.port = i;
		}
	}
}
void BallSim::Initialise(MeshManager& mgr, int carNo)
{
	laps = 0;
	//*****************************************************************************

	Mesh& ms = mMeshMgr.CreateMesh("Car");
	ms.CreateFrom("../bin/data/CarModel/CarModel.obj", gd3dDevice, mFX.mCache);
	mCar.Initialise(ms);
	mCar.GetScale() = Vector3(0.01f, 0.01f, 0.01f);
	mCar.GetPosition() = Vector3(0, 0, 0);
	mCar.GetRotation() = Vector3(0, 0, 0);

	this->carNo = carNo;

	vector<Vector4> colours;
	colours.push_back(Colours::Blue);
	colours.push_back(Colours::Red);
	colours.push_back(Colours::Green);
	colours.push_back(Colours::Magenta);
	MaterialExt mat = mCar.GetMesh().GetSubMesh(0).material;
	mat.gfxData.Set(colours[carNo], mat.gfxData.Ambient, mat.gfxData.Specular);
	mCar.SetOverrideMat(&mat);
	mLastMode = mMode = Mode::WAITING;

	mCubes.insert(mCubes.begin(), MAX_CUBES, mCar);
	//*****************************************************************************
	Start(carNo);
}

void BallSim::Start(int carNo)
{
	// Set-Up Movement Values
	enginePower = 0.5f;
	inverseEP = 1.0f;
	prevAc = false;
	epTimer = 0.0f;
	speed = 0.0f;

	switch (mMode)
	{
	case Mode::WAITING:
		mCar.GetRotation().y = PI;
		break;

	}
}

bool BallSim::GetPoliceCar()
{
	return policeCar;
}
bool BallSim::GetCarShrink()
{
	return carShrink;
}
bool BallSim::GetCarGrow()
{
	return carGrow;
}
void BallSim::Update(float dTime, int groundType, const Vector3& camPos, MouseAndKeys& input)
{
	float iFM;

	mGamepad.Update();
	mAngle += dTime * 1.8f;

	switch (groundType)
	{
	case 0:
		frictionMultiplier = 0.05f;
		prevGroundType = 0;
		break;
	case 1:
	case 3:
		frictionMultiplier = 1.0f;
		prevGroundType = 1;
		break;
	case 2:
		frictionMultiplier = 0.4f;
		prevGroundType = 2;
		break;
	}

	if (speedBoost)	
	{
		speedCooldown -= dTime;
		if (speedCooldown <= 0.0f)
			speedBoost = false;
		else
			frictionMultiplier += 1.0f;
	}
	
	if (GetPoliceCar() == true)
	{
		policeCooldown -= dTime;
		if (policeCooldown <= 0.0f)
			policeCar = false;
		Matrix mx = Matrix::CreateRotationY(mAngle);
		Vector3 dir(1, 0, 0);
		dir = Vector3::TransformNormal(dir, mx);
		Vector3 pos = GetPos() + Vector3(0.0,0.1, -0.5);
		FX::SetupSpotLight(carNo + 1, GetPoliceCar(), pos, dir, Vector3(1, 0, 0), Vector3(0.1, 0.1, 0.1), Vector3(0, 0, 0));
		dir *= -1;
		pos = GetPos() + Vector3(0.0, 0.1, -0.5);
		FX::SetupSpotLight((carNo + 5), GetPoliceCar(), pos, dir, Vector3(0, 0, 1), Vector3(0.1, 0.1, 0.1), Vector3(0, 0, 0));
	}
	
	float count = 0;
	if (GetCarGrow() == true && GetCarShrink() == false)
	{
		count -= dTime;
		if(count <= 0.0f && growCooldown > 0.0f)
		{
			count = 1.f;
			k = 0.0001;
			mCar.GetScale() = Vector3(mCar.GetScale().x + k, mCar.GetScale().y + k, mCar.GetScale().z + k );
			mCar.GetPosition() = Vector3(mCar.GetPosition().x, mCar.GetPosition().y - k, mCar.GetPosition().z);
		}

		growCooldown -= dTime;
		if (growCooldown <= 0.0f)
		{
			count -= dTime;
			if (count <= 0.0f)
			{
				count = 1.f;
				k = -0.0001;
				mCar.GetScale() = Vector3(mCar.GetScale().x + k, mCar.GetScale().y + k, mCar.GetScale().z + k);
				mCar.GetPosition() = Vector3(mCar.GetPosition().x, mCar.GetPosition().y + k, mCar.GetPosition().z);
			}
		}
	}
	if(mCar.GetScale().x <  0.01f && GetCarShrink() == false)
	{
		growCooldown = 5.0f;
		carGrow = false;
		mCar.GetScale() = Vector3(0.01, 0.01, 0.01);
		mCar.GetPosition() = Vector3(mCar.GetPosition().x, 0.2f, mCar.GetPosition().z);
		k = 0.01f;
	}
	if (mCar.GetScale().x > 0.01f && GetCarGrow() == false)
	{
		growCooldown = 5.0f;
		carShrink = false;
		mCar.GetScale() = Vector3(0.01, 0.01, 0.01);
		mCar.GetPosition() = Vector3(mCar.GetPosition().x, 0.2f, mCar.GetPosition().z);
		k = 0.01f;
	}
	if (GetCarShrink() == true && GetCarGrow() == false)
	{
		count -= dTime;
		if (count <= 0.0f && growCooldown > 0.0f && mCar.GetScale().x > 0.0005)
		{
			count = 1.f;
			k = -0.00005;
			mCar.GetScale() = Vector3(mCar.GetScale().x + k, mCar.GetScale().y + k, mCar.GetScale().z + k);
			mCar.GetPosition() = Vector3(mCar.GetPosition().x, mCar.GetPosition().y + k, mCar.GetPosition().z);
		}

		growCooldown -= dTime;
		if (growCooldown <= 0.0f)
		{
			count -= dTime;
			if (count <= 0.0f)
			{
				count = 1.f;
				k = 0.00005;
				mCar.GetScale() = Vector3(mCar.GetScale().x + k, mCar.GetScale().y + k, mCar.GetScale().z + k);
				mCar.GetPosition() = Vector3(mCar.GetPosition().x, mCar.GetPosition().y - k, mCar.GetPosition().z);
			}
		}
	}

	iFM = 2.0f - frictionMultiplier;

	if (ChangeEnginePower(dTime, mGamepad.IsPressed(carNo, XINPUT_GAMEPAD_A)))
		Accelerate(dTime, frictionMultiplier, iFM);
	else if(ChangeEnginePower(dTime, mGamepad.IsPressed(carNo, XINPUT_GAMEPAD_B)))
		Decelerate(dTime * 2, frictionMultiplier, iFM);
	else
		Decelerate(dTime, frictionMultiplier, iFM);
	Vector3 dPos(0.0f, 0.0f, speed * 0.1f * dTime);
	SetCarPosition(dPos);
	TurnCar((mGamepad.GetState(carNo).leftStickX));

	Vector3 pos = mCar.GetPosition();
	mCar.GetRotation().y += turnSpeed * dTime;						//Causes car to turn 
	pos += positionInc;
	mCar.GetPosition() = pos;
}

void BallSim::Render(FX::MyFX& fx, float dTime)
{
	fx.Render(mCar, gd3dImmediateContext);
}

void BallSim::RenderText(SpriteFont *pF, SpriteBatch *pBatch)
{

}

DirectX::SimpleMath::Vector3 BallSim::GetRotation()
{
	return mCar.GetRotation();
}

DirectX::SimpleMath::Vector3 BallSim::GetPos()
{
	return mCar.GetPosition();
}

void BallSim::SetCarPosition(DirectX::SimpleMath::Vector3 dPos)
{
	positionInc.y = dPos.y;
	positionInc.x = dPos.z * sinf(mCar.GetRotation().y);
	positionInc.z = dPos.z * cosf(mCar.GetRotation().y);
}

void BallSim::TurnCar(float rotation)
{
	turnSpeed = rotation;
}
Model BallSim::GetCar()
{
	return mCar;
}
Model BallSim::GetCar2()
{
	return mCar2;
}
Model BallSim::GetCar3()
{
	return mCar3;
}
Model BallSim::GetCar4()
{
	return mCar4;
}

void BallSim::Crash()
{
	speed = -10.0f;
	enginePower = 0.5f;
	NFSpeed = -10.0f;
	prevAc = false;
	epTimer = 0.0f;
}
void BallSim::PickUpObtained(BallSim& car)
{
	int RandomNo = rand() % 4+ 1;
	switch (RandomNo)
	{
	default:
		break;
	case 1:
		speedBoost = true;		
		speedCooldown = maxSpeedCooldown;
		if (!GetIAudioMgr()->GetSfxMgr()->IsPlaying(speedHdl))
		{
			GetIAudioMgr()->GetSfxMgr()->Play("SpeedPickUp", false, false, &speedHdl, 0.7f);
		}
		break;
	case 2:
		policeCar = true;
		policeCooldown = maxPoliceCooldown;
		if (!GetIAudioMgr()->GetSfxMgr()->IsPlaying(policeHdl))
		{
			GetIAudioMgr()->GetSfxMgr()->Play("PoliceSirenClipped", false, false, &policeHdl, 0.7f);
		}
		break;
	case 3:
		if (GetCarGrow() == false)
		{
			carShrink = true;
			shrinkCooldown = maxShrinkCooldown;
			if (!GetIAudioMgr()->GetSfxMgr()->IsPlaying(shrinkHdl))
			{
				GetIAudioMgr()->GetSfxMgr()->Play("ShrinkSound", false, false, &shrinkHdl, 0.7f);
			}
		}
		break;
	case 4:
		if (GetCarShrink() == false)
		{
			carGrow = true;
			growCooldown = maxGrowCooldown;
			if (!GetIAudioMgr()->GetSfxMgr()->IsPlaying(growHdl))
			{
				GetIAudioMgr()->GetSfxMgr()->Play("GrowSoundOGG", false, false, &growHdl, 0.7f);
			}
		}
		break;
	}
}


bool BallSim::ChangeEnginePower(float dTime, bool accelerating)
{
	epTimer += dTime;
	if (accelerating)
		if (epTimer < epX1)
			enginePower = epGrad1 * epTimer + epY3;
		else
		{
			enginePower = (epGrad2 * epTimer) + epIntersect2;
			if (enginePower > epY2)
				enginePower = epY2;
		}
	else
	{
		enginePower = (epGrad3 * epTimer) + epY2;
		if (enginePower < epY3)
			enginePower = epY3;
	}

	if (prevAc != accelerating)
	{
		enginePower = 0.5f;
		epTimer = 0.0f;
	}
	prevAc = accelerating;
	return accelerating;
}

float BallSim::CalculateAcceleration(float s, float fM, float iFM)
{
	float acceleration;
	if (s < acX1 * fM)
		acceleration = acY3 * fM;
	else
		if (s < acX2 * fM)
			acceleration = (acGrad2 * s) + acIntersect2 * fM;
		else
			if (s < acX3 * fM)
				acceleration = (acGrad3 * s) + acIntersect3 * fM;
			else
				acceleration = (accAboveMaxMult * acGrad3 * s) - (accAboveMaxMult * acGrad3 * acX3 * fM);
	return acceleration;
}

float BallSim::CalculateDeceleration(float s, float fM, float iFM)
{
	float deceleration;
	if (s < dcX1 * fM)
		deceleration = (dcGrad1 * s) + dcY3 * iFM;
	else
		if (s < dcX2 * fM)
			deceleration = (dcGrad2 * s) + dcIntersect2 * iFM;
		else
		{
			deceleration = dcY2 * iFM;
			if (s > dcX3 * fM)
				deceleration *= accAboveMaxMult;
		}
	return deceleration;
}

void BallSim::Accelerate(float dTime, float fM, float iFM)
{
	NFSpeed += CalculateAcceleration(NFSpeed, 1.0f, 1.0f) * enginePower * dTime;
	if (NFSpeed > maxSpeed)
		NFSpeed = maxSpeed;

	speed += CalculateAcceleration(speed, fM, iFM) * enginePower * dTime;
}

void BallSim::Decelerate(float dTime, float fM, float iFM)
{
	NFSpeed -= CalculateDeceleration(NFSpeed, 1.0f, 1.0f) * dTime;
	if (NFSpeed < 0.0f)
		NFSpeed = 0.0f;

	speed -= CalculateDeceleration(speed, fM, iFM) * dTime;
	if (speed < 0.0f)
		speed = 0.0f;
}

std::vector<bool>& BallSim::GetCheckPointsPassed()
{
	return checkPointsPassed;
}

bool Gamepads::IsPressed(int idx, unsigned short buttonId)
{
	assert(idx >= 0 && idx < XUSER_MAX_COUNT);
	assert(mPads[idx].port != -1);
	return (mPads[idx].state.Gamepad.wButtons & buttonId) != 0;
}

void Gamepads::GetAnalogueNames(int idx, std::wstring& mssg)
{
	
}
void Gamepads::GetDigitalNames(int idx, std::wstring& mssg)
{
	
}
