#include "HighScores.h"

void HighScores::OnResize(int screenWidth, int screenHeight)
{
	OnResize_Default(screenWidth, screenHeight);
}

void HighScores::Load()
{

	mQuad.Initialise(BuildQuad(mMeshMgr));
	MaterialExt *pMat = &mQuad.GetMesh().GetSubMesh(0).material;
	pMat->pTextureRV = mFX.mCache.LoadTexture("podium.dds", true, gd3dDevice);
}

void HighScores::LoadDisplay(float dTime)
{

}

void HighScores::Initialise(ModeManager* mmPtr)
{
	this->mmPtr = mmPtr;

	mFX.Init(gd3dDevice);


	mpSpriteBatch = new SpriteBatch(gd3dImmediateContext);
	assert(mpSpriteBatch);
	mpFont = new SpriteFont(gd3dDevice, L"../bin/data/comicSansMS.spritefont");
	assert(mpFont);

	mpFont2 = new SpriteFont(gd3dDevice, L"../bin/data/algerian.spritefont");
	assert(mpFont2);

	mLoadData.totalToLoad = 4;
	mLoadData.loadedSoFar = 0;
	mLoadData.running = true;
	mLoadData.loader = std::async(launch::async, &HighScores::Load, this);

}

void HighScores::Release()
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

void HighScores::Update(float dTime)
{

}

void HighScores::Render(float dTime, float highScores1, float highScores2, float highScores3, float highScores4, float highScores5, float highScores6, float highScores7, float highScores8, float highScores9, float highScores10)
{
	if (mLoadData.running)
	{
		if (!mLoadData.loader._Is_ready())
		{
			return;
		}
		mLoadData.loader.get();
		mLoadData.running = false;
		return;
	}


	FX::SetPerFrameConsts(gd3dImmediateContext, mCamPos);

	int sw, sh;
	GetClientExtents(sw, sh);

	CreateViewMatrix(FX::GetViewMatrix(), mCamPos, Vector3(0, 0, 0), Vector3(0, 1, 0));
	CreateProjectionMatrix(FX::GetProjectionMatrix(), 0.25f*PI, GetAspectRatio(), 1, 1000.f);
	SetViewportDimensions(gd3dImmediateContext, 0, 0, sw, sh, gScreenViewport);

	MaterialExt mat = MaterialExt::default;
	mQuad.GetScale() = Vector3(5.0f, 4.0f, 4.0f);
	mQuad.GetRotation() = Vector3(-1.0f, 0.0f, 0.0f);
	mQuad.GetPosition() = Vector3(0, 0, 1);
	mFX.Render(mQuad, gd3dImmediateContext); //render background

	CommonStates statetwo(gd3dDevice);
	mpSpriteBatch->Begin(SpriteSortMode_Deferred, statetwo.NonPremultiplied());


	float swf = sw;
	float shf = sh;

	float textWidth = swf * 0.0018f;
	float textHeight = shf * 0.0032857f;

	wstringstream t;
	t << L"Average                 Karting";
	mpFont2->DrawString(mpSpriteBatch, t.str().c_str(), Vector2(sw / 11, sh / 8), Colours::White, 0, Vector2(0, 0), Vector2(textWidth, textHeight));

	wstringstream play, instructions, highscores, back;
	play << L"Start Game";
	instructions << L"Instructions";
	highscores << L"Highscores\n   " << highScores1 << "\n   " << highScores2 << "\n   " << highScores3 << "\n   " << highScores4 << "\n   " << highScores5 << "\n   " <<
		highScores6 << "\n   " << highScores7 << "\n   " << highScores8 << "\n   " << highScores9 << "\n   " << highScores10;
	
	mpFont2->DrawString(mpSpriteBatch, highscores.str().c_str(), Vector2(sw / 2.5f, sh / 3), Colours::White, 0, Vector2(0, 0), Vector2(textWidth * 0.4f, textHeight * 0.4f));

	back << L"Press 'B' to go back to main menu";
	mpFont->DrawString(mpSpriteBatch, back.str().c_str(), Vector2(sw * 0.75f, sh * 0.7f), Colours::White, 0, Vector2(0, 0), Vector2(textWidth * 0.4f, textHeight * 0.4f));

	mpSpriteBatch->End();

	EndRender();

	mMKInput.PostProcess();
}

LRESULT HighScores::WindowsMssgHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
	//default message handling (resize window, full screen, etc)
	return DefaultMssgHandler(hwnd, msg, wParam, lParam);
}