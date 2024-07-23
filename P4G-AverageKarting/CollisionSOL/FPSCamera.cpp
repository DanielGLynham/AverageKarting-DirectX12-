#include "FPSCamera.h"
#include "D3D.h"
#include "D3DUtil.h"
#include "Game.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;


void FPSCamera::Initialise(const Vector3& pos, const DirectX::SimpleMath::Vector3& yawPitchRoll, Matrix& viewSpaceTfm)
{
	mpViewSpaceTfm = &viewSpaceTfm;
	mCamPos = pos;
	yaw = yawPitchRoll.x;
	pitch = yawPitchRoll.y;
	roll = yawPitchRoll.z;

	*mpViewSpaceTfm = Matrix::CreateFromYawPitchRoll(yaw, pitch, roll) * Matrix::CreateTranslation(pos);
}

void FPSCamera::Move(float dTime, DirectX::SimpleMath::Vector3 carPos, DirectX::SimpleMath::Vector3 carRot/*bool forward, bool back, bool left, bool right*/)
{
	Vector3 pos(mCamPos);

	float carYRotation = carRot.y;
	pos.x = carPos.x + camRadius * cos((1.5 * M_PI) - fmodf(carYRotation, 2 * M_PI));
	pos.y = carPos.y + 1.0f;
	pos.z = carPos.z + camRadius * sin((1.5 * M_PI) - fmodf(carYRotation, 2 * M_PI));

	CreateViewMatrix(*mpViewSpaceTfm, pos, Vector3(carPos.x, carPos.y + 1.0f, carPos.z), Vector3(0, 1, 0));
	
	mCamPos = pos;
}

void FPSCamera::Rotate(float dTime, float _yaw, float _pitch, float _roll)
{
	yaw += _yaw * dTime * rspeed;
	pitch += _pitch * dTime * rspeed;
	roll += _roll * dTime * rspeed;
	Matrix ori;
	ori = Matrix::CreateFromYawPitchRoll(yaw, pitch, roll);
	Vector3 dir(0, 0, 1), up(0, 1, 0);
	dir = Vector3::TransformNormal(dir, ori);
	up = Vector3::TransformNormal(up, ori);
	CreateViewMatrix(*mpViewSpaceTfm, mCamPos, mCamPos + dir, up);
}
