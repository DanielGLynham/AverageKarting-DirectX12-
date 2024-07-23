#include "Instructions.h"

void Instructions::OnResize(int screenWidth, int screenHeight)
{
	OnResize_Default(screenWidth, screenHeight);
}

void Instructions::Load()
{
	Mesh& ms = mMeshMgr.CreateMesh("car");
	ms.CreateFrom("../bin/data/CarModel/CarModel.obj", gd3dDevice, mFX.mCache);
	mCar.Initialise(ms);
	mCar.GetScale() = Vector3(0.011f, 0.011f, 0.011f);
	mCar.GetPosition() = Vector3(0, 1.0f, 0);
	mCar.GetRotation() = Vector3(90, 0, 90);
	mLoadData.loadedSoFar++;

	mQuad.Initialise(BuildQuad(mMeshMgr));
	MaterialExt *pMat = &mQuad.GetMesh().GetSubMesh(0).material;
	pMat->pTextureRV = mFX.mCache.LoadTexture("racebackground.dds", true, gd3dDevice);
}

void Instructions::LoadDisplay(float dTime)
{
	/*BeginRender(Colours::Black);	*****************************************************************************************DONT NEED?

	mpSpriteBatch->Begin();

	static int pips = 0;
	static float elapsed = 0;
	elapsed += dTime;
	if (elapsed > 0.25f) {
		pips++;
		elapsed = 0;
	}
	if (pips > 10)
		pips = 0;
	wstringstream ss;
	ss << L"Loading meshes(" << (int)(((float)mLoadData.loadedSoFar / (float)mLoadData.totalToLoad)*100.f) << L"%) ";
	for (int i = 0; i < pips; ++i)
		ss << L'.';
	mpFont2->DrawString(mpSpriteBatch, ss.str().c_str(), Vector2(100, 200), Colours::White, 0, Vector2(0, 0), Vector2(1.f, 1.f));

	ss.str(L"");
	ss << L"FPS:" << (int)(1.f / dTime);
	mpFont->DrawString(mpSpriteBatch, ss.str().c_str(), Vector2(10, 550), Colours::White, 0, Vector2(0, 0), Vector2(0.5f, 0.5f));

	mpSpriteBatch->End();

	EndRender();*/
}

void Instructions::Initialise(ModeManager* mmPtr)
{
	this->mmPtr = mmPtr;

	mFX.Init(gd3dDevice);

	//FX::SetupDirectionalLight(0, true, Vector3(-0.7f, -0.7f, 0.7f), Vector3(0.8f, 0.8f, 0.8f), Vector3(0.15f, 0.15f, 0.15f), Vector3(0.25f, 0.25f, 0.25f)); *****************************************************************************************DONT NEED?

	mpSpriteBatch = new SpriteBatch(gd3dImmediateContext);
	assert(mpSpriteBatch);
	mpFont = new SpriteFont(gd3dDevice, L"../bin/data/comicSansMS.spritefont");
	assert(mpFont);

	mpFont2 = new SpriteFont(gd3dDevice, L"../bin/data/algerian.spritefont");
	assert(mpFont2);

	mLoadData.totalToLoad = 4;
	mLoadData.loadedSoFar = 0;
	mLoadData.running = true;
	mLoadData.loader = std::async(launch::async, &Instructions::Load, this);

	/*mMKInput.Initialise(GetMainWnd()); *****************************************************************************************DONT NEED?
	mGamepad.Initialise();*/
}

void Instructions::Release()
{
	mmPtr = nullptr;
	mFX.Release();
	mMeshMgr.Release();
	delete mpFont;
	mpFont = nullptr;
	delete mpSpriteBatch;
	mpSpriteBatch = nullptr;
	delete mpFont2;
	mpFont2 = nullptr;
}

void Instructions::Update(float dTime)
{
	//mGamepad.Update(); *****************************************************************************************DONT NEED?

	//const float camInc = 10.f * dTime;

	//if (mMKInput.IsPressed(VK_A))
	//	mCamPos.y += camInc;
	//else if (mMKInput.IsPressed(VK_Z))
	//	mCamPos.y -= camInc;
	//else if (mMKInput.IsPressed(VK_D))
	//	mCamPos.x -= camInc;
	//else if (mMKInput.IsPressed(VK_F))
	//	mCamPos.x += camInc;
	//else if (mMKInput.IsPressed(VK_W))
	//	mCamPos.z += camInc;
	//else if (mMKInput.IsPressed(VK_S))
	//	mCamPos.z -= camInc;

	////don't update anything that relies on loaded assets until they are loaded
	//if (mLoadData.running)
	//	return;
}

void Instructions::Render(float dTime)
{
	if (mLoadData.running)
	{
		if (!mLoadData.loader._Is_ready())
		{
			//LoadDisplay(dTime); *****************************************************************************************DONT NEED?
			return;
		}
		mLoadData.loader.get();
		mLoadData.running = false;
		return;
	}

	BeginRender(Colours::Black);

	FX::SetPerFrameConsts(gd3dImmediateContext, mCamPos);

	int sw, sh;
	GetClientExtents(sw, sh);

	CreateViewMatrix(FX::GetViewMatrix(), mCamPos, Vector3(0, 0, 0), Vector3(0, 1, 0));
	CreateProjectionMatrix(FX::GetProjectionMatrix(), 0.25f*PI, GetAspectRatio(), 1, 1000.f);
	SetViewportDimensions(gd3dImmediateContext, 0, 0, sw, sh, gScreenViewport);

	mCar.GetRotation() = Vector3(0.4f, 3.4f, 0);
	mCar.GetPosition() = Vector3(0, 0, -3.5);
	

	mFX.Render(mCar, gd3dImmediateContext);

	MaterialExt mat = MaterialExt::default;
	mQuad.GetScale() = Vector3(5.0f, 4.0f, 4.0f);
	mQuad.GetRotation() = Vector3(-1.0f, 0.0f, 0.0f);
	mFX.Render(mQuad, gd3dImmediateContext); //render background

	CommonStates statetwo(gd3dDevice);
	mpSpriteBatch->Begin(SpriteSortMode_Deferred, statetwo.NonPremultiplied());

	//wstring mssg, mssg2; *****************************************************************************************DONT NEED?

	//fire a ray from the camera to the point on the XY plane we mouse clicked on
	Ray ray;
	mMKInput.MousePosToWorldRay(FX::GetViewMatrix(), FX::GetProjectionMatrix(), mCamPos, ray);
	Plane p(Vector3(0, 0, 0), Vector3(0, 0, -1.0f));
	float dist;
	ray.Intersects(p, dist);
	Vector3 pos(ray.position + ray.direction*dist);

	/*wstringstream sstr;
	sstr.precision(3);
	sstr << L"Mouse move 3D x=" << pos.x << L" y=" << pos.y << L" z=" << pos.z;
	mssg = sstr.str();
	mpFont->DrawString(mpSpriteBatch, mssg.c_str(), Vector2(10, 100), DirectX::Colors::White, 0, Vector2(0, 0), Vector2(1, 1));*/

	float swf = sw;
	float shf = sh;

	float textWidth = swf * 0.0018f;
	float textHeight = shf * 0.0032857f;

	//general messages
	wstringstream ss;
	if (dTime > 0)
		ss << L"FPS: " << (int)(1.f / dTime);
	else
		ss << L"FPS: 0";
	mpFont->DrawString(mpSpriteBatch, ss.str().c_str(), Vector2(10, 550), Colours::White, 0, Vector2(0, 0), Vector2(0.5f, 0.5f));

	wstringstream t;
	t << L"Average                 Karting";
	mpFont2->DrawString(mpSpriteBatch, t.str().c_str(), Vector2(sw / 11, sh / 8), Colours::White, 0, Vector2(0, 0), Vector2(textWidth, textHeight));

	wstringstream instructions, back;
	instructions << L"Instructions\n" << " Complete 3 full laps to win the game\n Use the left thumb stick to move\n Use A to accellerate";
	mpFont->DrawString(mpSpriteBatch, instructions.str().c_str(), Vector2(sw * 0.38f, sh * 0.45f), Colours::White, 0, Vector2(0, 0), Vector2(textWidth * 0.3f, textHeight * 0.3f));


	back << L"Press 'B' to go back to main menu";
	mpFont->DrawString(mpSpriteBatch, back.str().c_str(), Vector2(sw * 0.75f, sh * 0.7f), Colours::White, 0, Vector2(0, 0), Vector2(textWidth * 0.4f, textHeight * 0.4f));


	mpSpriteBatch->End();

	EndRender();

	mMKInput.PostProcess();
}

LRESULT Instructions::WindowsMssgHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	const float camInc = 20.f * GetElapsedSec();

	// Check for change in state
	if (changeState)
	{
		changeState = false;
		mmPtr->SetMode(nextState);
	}

	//do something game specific here
	switch (msg)
	{
		// Respond to a keyboard event.
	case WM_CHAR:
		//mCar.Input(wParam);
		switch (wParam)
		{
		case 27:
		case 'q':
		case 'Q':
			PostQuitMessage(0);
			return 0;
		case 'm':
		case 'M':
			mmPtr->SetMode(2);
			break;
		case ' ':
			mCamPos = mDefCamPos;
			break;
		case 'b':
		case 'B':
			mmPtr->SetMode(0);
			break;
		}
	case WM_INPUT:
		mMKInput.MessageEvent((HRAWINPUT)lParam);
		break;
	}
	return DefaultMssgHandler(hwnd, msg, wParam, lParam);
}