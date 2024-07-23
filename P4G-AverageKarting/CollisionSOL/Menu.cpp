
#include "Game.h"
#include "Menu.h"

void Menu::OnResize(int screenWidth, int screenHeight)
{
	OnResize_Default(screenWidth, screenHeight);
}

void Menu::Load()
{
	Mesh& ms = mMeshMgr.CreateMesh("car");
	ms.CreateFrom("../bin/data/CarModel/CarModel.obj", gd3dDevice, mFX.mCache);
	mCar.Initialise(ms);
	mCar.GetScale() = Vector3(0.01f, 0.01f, 0.01f);
	mCar.GetPosition() = Vector3(0, 1.0f, 0);
	mCar.GetRotation() = Vector3(90, 0, 90);

	MaterialExt mat = mCar.GetMesh().GetSubMesh(0).material;
	mat.gfxData.Set(Colours::GetRandomColour(), mat.gfxData.Ambient, mat.gfxData.Specular);
	raceCarOne.SetOverrideMat(&mat);
	raceCarOne.Initialise(ms);
	raceCarOne.GetScale() = Vector3(0.005f, 0.005f, 0.005f);
	raceCarOne.GetPosition() = Vector3(-2, -0.8f, -2);
	raceCarOne.GetRotation() = Vector3(0, 140, 0);

	MaterialExt mat2 = mCar.GetMesh().GetSubMesh(0).material;
	mat2.gfxData.Set(Colours::GetRandomColour(), mat2.gfxData.Ambient, mat2.gfxData.Specular);
	raceCarTwo.SetOverrideMat(&mat2);
	raceCarTwo.Initialise(ms);
	raceCarTwo.GetScale() = Vector3(0.005f, 0.005f, 0.005f);
	raceCarTwo.GetPosition() = Vector3(-2, -0.8f, -1.5);
	raceCarTwo.GetRotation() = Vector3(0, 140, 0);

	mQuad.Initialise(BuildQuad(mMeshMgr));
	MaterialExt *pMat = &mQuad.GetMesh().GetSubMesh(0).material;
	pMat->pTextureRV = mFX.mCache.LoadTexture("data/racebackground.dds", false, gd3dDevice);
}

void Menu::LoadDisplay(float dTime)
{
	BeginRender(Colours::Black);

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
	for (int i = 0; i < pips; ++i)
		ss << L'.';
	mpFont2->DrawString(mpSpriteBatch, ss.str().c_str(), Vector2(100, 200), Colours::White, 0, Vector2(0, 0), Vector2(1.f, 1.f));

	ss.str(L"");
	ss << L"FPS:" << (int)(1.f / dTime);
	mpFont->DrawString(mpSpriteBatch, ss.str().c_str(), Vector2(10, 550), Colours::White, 0, Vector2(0, 0), Vector2(0.5f, 0.5f));

	mpSpriteBatch->End();

	EndRender();
}

void Menu::Initialise(ModeManager* mmPtr, Game& game)
{
	mpGame = &game;
	this->mmPtr = mmPtr;

	mFX.Init(gd3dDevice);

	FX::SetupDirectionalLight(0, true, Vector3(-0.7f, -0.7f, 0.7f), Vector3(0.8f, 0.8f, 0.8f), Vector3(0.15f, 0.15f, 0.15f), Vector3(0.25f, 0.25f, 0.25f));

	mpSpriteBatch = new SpriteBatch(gd3dImmediateContext);
	assert(mpSpriteBatch);
	mpFont = new SpriteFont(gd3dDevice, L"../bin/data/comicSansMS.spritefont");
	assert(mpFont);

	mpFont2 = new SpriteFont(gd3dDevice, L"../bin/data/algerian.spritefont");
	assert(mpFont2);

	mMKInput.Initialise(GetMainWnd());
	mGamepad.Initialise();
}

void Menu::Release()
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

void Menu::Update(float dTime)
{
	if (!mpGame->FinishedLoading())
		return;

	mGamepad.Update();

		rotationCarY += dTime;

		racingcarsTimer -= dTime;

		if (!raceCarOneFinished)
		{
			raceCarOne.GetPosition().x += raceCarOneVel / 1000;
		}
		if (!raceCarTwoFinished)
		{
			raceCarTwo.GetPosition().x += raceCarTwoVel / 1000;
		}
		if (raceCarOne.GetPosition().x > 4)
		{
			raceCarOneFinished = true;
		}
		if (raceCarTwo.GetPosition().x > 4)
		{
			raceCarTwoFinished = true;
		}
		if (raceCarOneFinished && raceCarTwoFinished && racingcarsTimer < 0)
		{
			MaterialExt mat2 = mCar.GetMesh().GetSubMesh(0).material;
			mat2.gfxData.Set(Colours::GetRandomColour(), mat2.gfxData.Ambient, mat2.gfxData.Specular);
			raceCarTwo.SetOverrideMat(&mat2);
			MaterialExt mat = mCar.GetMesh().GetSubMesh(0).material;
			mat.gfxData.Set(Colours::GetRandomColour(), mat.gfxData.Ambient, mat.gfxData.Specular);
			raceCarOne.SetOverrideMat(&mat);

			racingcarsTimer = 5;
			raceCarOneVel = (rand() % 4) + 5;
			raceCarTwoVel = (rand() % 4) + 5;
			raceCarOne.GetPosition().x = -2.5f;
			raceCarTwo.GetPosition().x = -2.5f;
			raceCarOneFinished = false;
			raceCarTwoFinished = false;
		}
}

void Menu::Render(float dTime)
{
	if (!mpGame->FinishedLoading())
	{
		LoadDisplay(dTime);
		return;
	}


	BeginRender(Colours::Black);

	FX::SetPerFrameConsts(gd3dImmediateContext, mCamPos);

	int sw, sh;
	GetClientExtents(sw, sh);

	CreateViewMatrix(FX::GetViewMatrix(), mCamPos, Vector3(0, 0, 0), Vector3(0, 1, 0));
	CreateProjectionMatrix(FX::GetProjectionMatrix(), 0.25f*PI, GetAspectRatio(), 1, 1000.f);
	SetViewportDimensions(gd3dImmediateContext, 0, 0, sw, sh, gScreenViewport);

	MaterialExt mat = MaterialExt::default;
	mQuad.GetScale() = Vector3(5.0f, 4.0f, 4.0f);
	mQuad.GetRotation() = Vector3(-1.0f, 0.0f, 0.0f);
	mFX.Render(mQuad, gd3dImmediateContext); //render background

	mCar.GetRotation() = Vector3(0, rotationCarY, 0);

	mFX.Render(mCar, gd3dImmediateContext);
	if (!raceCarOneFinished)
	{
	mFX.Render(raceCarOne, gd3dImmediateContext);
	}
	if (!raceCarTwoFinished)
	{
		mFX.Render(raceCarTwo, gd3dImmediateContext);
	}
	CommonStates state(gd3dDevice);
	mpSpriteBatch->Begin(SpriteSortMode_Deferred, state.NonPremultiplied());

	wstring mssg, mssg2;

	//fire a ray from the camera to the point on the XY plane we mouse clicked on
	Ray ray;
	mMKInput.MousePosToWorldRay(FX::GetViewMatrix(), FX::GetProjectionMatrix(), mCamPos, ray);
	Plane p(Vector3(0, 0, 0), Vector3(0, 0, -1.0f));
	float dist;
	ray.Intersects(p, dist);
	Vector3 pos(ray.position + ray.direction*dist);


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
	mpFont2->DrawString(mpSpriteBatch, t.str().c_str(), Vector2(sw / 11, sh / 8), Colours::Magenta, 0, Vector2(0, 0), Vector2(textWidth, textHeight));

	wstringstream play, instructions, highscores, quit, moveObstacles;
	play << L"Start Game";
	instructions << L"Instructions";
	highscores << L"Highscores";
	quit << L"Quit";
	moveObstacles << L"Click to have moving obstacles";

	mOption = mQuad;
	mOption.GetPosition() = Vector3(-0.8f, 0.0f, -3.0f);
	mOption.GetRotation() = Vector3(-PI / 2, 0, 0);
	mOption.GetScale() = Vector3(0.25f, 0.25f, 0.1f);

	bool hoverPlay = false;
	if (RayToSphere(mOption.GetPosition(), 1.f * mOption.GetScale().z, ray, dist))
		if (RayToPlane(Plane(mOption.GetPosition(), FX::GetViewMatrix().Backward()), ray, dist))
		{
			hoverPlay = true;
			mpFont->DrawString(mpSpriteBatch, play.str().c_str(), Vector2(sw * 3 / 18, sh * 13 / 20), Colours::Yellow, 0, Vector2(0, 0), Vector2(textWidth * 0.6f, textHeight * 0.6f));
			if (mMKInput.GetMouseButton(MouseAndKeys::LBUTTON))
			{
				changeState = true;
				nextState = 2;
			}
		}
	if (!hoverPlay)
	{
		mpFont->DrawString(mpSpriteBatch, play.str().c_str(), Vector2(sw * 3 / 18, sh * 13 / 20), Colours::Magenta, 0, Vector2(0, 0), Vector2(textWidth * 0.6f, textHeight * 0.6f));
	}

	mOption.GetPosition() = Vector3(-0.15f, 0.0f, -3.0f);
	mOption.GetScale() = Vector3(0.25f, 0.25f, 0.1f);

	bool hoverInstruct = false;
	if (RayToSphere(mOption.GetPosition(), 1.f * mOption.GetScale().z, ray, dist))
		if (RayToPlane(Plane(mOption.GetPosition(), FX::GetViewMatrix().Backward()), ray, dist))
		{
			hoverInstruct = true;
			mpFont->DrawString(mpSpriteBatch, instructions.str().c_str(), Vector2(sw * 7 / 18, sh * 13 / 20), Colours::Yellow, 0, Vector2(0, 0), Vector2(textWidth * 0.6f, textHeight * 0.6f));
			if (mMKInput.GetMouseButton(MouseAndKeys::LBUTTON))
			{
				changeState = true;
				nextState = 3;
			}
		}
	if (!hoverInstruct)
			mpFont->DrawString(mpSpriteBatch, instructions.str().c_str(), Vector2(sw * 7 / 18, sh * 13 / 20), Colours::Magenta, 0, Vector2(0, 0), Vector2(textWidth * 0.6f, textHeight * 0.6f));

	mOption.GetPosition() = Vector3(0.5f, 0.0f, -3.0f);
	mOption.GetScale() = Vector3(0.25f, 0.25f, 0.1f);

	bool hoverHighScores = false;
	if (RayToSphere(mOption.GetPosition(), 1.f * mOption.GetScale().z, ray, dist))
		if (RayToPlane(Plane(mOption.GetPosition(), FX::GetViewMatrix().Backward()), ray, dist))
		{
			hoverHighScores = true;
			mpFont->DrawString(mpSpriteBatch, highscores.str().c_str(), Vector2(sw * 11 / 18, sh * 13 / 20), Colours::Yellow, 0, Vector2(0, 0), Vector2(textWidth * 0.6f, textHeight * 0.6f));
			if (mMKInput.GetMouseButton(MouseAndKeys::LBUTTON))
			{
				changeState = true;
				nextState = 4;
			}
		}
	if (!hoverHighScores)
		mpFont->DrawString(mpSpriteBatch, highscores.str().c_str(), Vector2(sw * 11 / 18, sh * 13 / 20), Colours::Magenta, 0, Vector2(0, 0), Vector2(textWidth * 0.6f, textHeight * 0.6f));
			

	mOption.GetPosition() = Vector3(1.05f, 0.0f, -3.0f);
	mOption.GetScale() = Vector3(0.25f, 0.25f, 0.1f);

	bool hoverQuit = false;
	if (RayToSphere(mOption.GetPosition(), 1.f * mOption.GetScale().z, ray, dist))
		if (RayToPlane(Plane(mOption.GetPosition(), FX::GetViewMatrix().Backward()), ray, dist))
		{
			hoverQuit = true;
			mpFont->DrawString(mpSpriteBatch, quit.str().c_str(), Vector2(sw * 15 / 18, sh * 13 / 20), Colours::Yellow, 0, Vector2(0, 0), Vector2(textWidth * 0.6f, textHeight * 0.6f));
			if (mMKInput.GetMouseButton(MouseAndKeys::LBUTTON))
			{
				PostQuitMessage(0);
			}
		}
	if (!hoverQuit)
		mpFont->DrawString(mpSpriteBatch, quit.str().c_str(), Vector2(sw * 15 / 18, sh * 13 / 20), Colours::Magenta, 0, Vector2(0, 0), Vector2(textWidth * 0.6f, textHeight * 0.6f));


	mOption.GetPosition() = Vector3(0.0f, -0.35f, -3.0f);
	mOption.GetScale() = Vector3(0.5f, 0.25f, 0.1f);

	bool hoverMovObs = false;
	if (RayToSphere(mOption.GetPosition(), 2.f * mOption.GetScale().z, ray, dist))
		if (RayToPlane(Plane(mOption.GetPosition(), FX::GetViewMatrix().Backward()), ray, dist))
		{
			hoverMovObs = true;
			mpFont->DrawString(mpSpriteBatch, moveObstacles.str().c_str(), Vector2(sw * 6 / 18, sh * 17 / 20), Colours::Yellow, 0, Vector2(0, 0), Vector2(textWidth * 0.6f, textHeight * 0.6f));
			if (mMKInput.GetMouseButton(MouseAndKeys::LBUTTON))
			{
				moveObstaclesClicked = !moveObstaclesClicked;
				mpGame->SetMoveObstacles(moveObstaclesClicked);
			}
		}
	if (!hoverMovObs)
		if (moveObstaclesClicked)
			mpFont->DrawString(mpSpriteBatch, moveObstacles.str().c_str(), Vector2(sw * 6 / 18, sh * 17 / 20), Colours::Green, 0, Vector2(0, 0), Vector2(textWidth * 0.6f, textHeight * 0.6f));
		else
			mpFont->DrawString(mpSpriteBatch, moveObstacles.str().c_str(), Vector2(sw * 6 / 18, sh * 17 / 20), Colours::Red, 0, Vector2(0, 0), Vector2(textWidth * 0.6f, textHeight * 0.6f));
			

	mpSpriteBatch->End();

	EndRender();

	mMKInput.PostProcess();
}

LRESULT Menu::WindowsMssgHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
		}
	case WM_INPUT:
		mMKInput.MessageEvent((HRAWINPUT)lParam);
		break;
	}
	return DefaultMssgHandler(hwnd, msg, wParam, lParam);
}