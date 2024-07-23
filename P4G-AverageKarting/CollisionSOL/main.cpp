#include "WindowUtils.h"
#include "D3D.h"
#include "Game.h"
#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>

//#include "AudioMgr.h"
#include "AudioMgrFMOD.h"

#include "File.h"

#include "Menu.h"
#include "ModeManager.h"
#include "Instructions.h"
#include "HighScores.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

Game gGame;
Menu menu;
HighScores highScores;
Instructions instructions;
ModeManager modeManager;
unsigned int mainMenuHdl;
unsigned int gameHdl;

int prevMode = 0;

bool fullscreen = false;

void Update(float dTime)
{
	//why is thi sneeded?
	GetIAudioMgr()->Update();
	GetIAudioMgr()->Update();
	GetIAudioMgr()->Update();
	GetIAudioMgr()->Update();
	GetIAudioMgr()->Update();

	int currentMode = modeManager.GetMode();
	switch (currentMode)
	{
	case 0:
	case 1:
		menu.Update(dTime);
		if (!GetIAudioMgr()->GetSongMgr()->IsPlaying(mainMenuHdl))
		{
			GetIAudioMgr()->GetSongMgr()->Play("YoshisIslandClipped", true, false, &mainMenuHdl, 0.7f);
		}
		break;
	case 2:
		if (prevMode != currentMode)
		{
			gGame.ResetGame();
		}
		gGame.Update(dTime);
		if (GetIAudioMgr()->GetSongMgr()->IsPlaying(mainMenuHdl))
		{
			GetIAudioMgr()->GetSongMgr()->Play("YoshisIslandClipped", false, true, &mainMenuHdl, 0.7f);
			if (!GetIAudioMgr()->GetSongMgr()->IsPlaying(gameHdl))
			{
				GetIAudioMgr()->GetSongMgr()->Play("GameSound", true, false, &gameHdl, 0.3f);
			}
		}
		break;
	case 3:				//instructions menu
	{
		instructions.Update(dTime);
		menu.Update(dTime);
		
	}
	break;
	case 4:				//highscores menu
	{
		highScores.Update(dTime);
		menu.Update(dTime);
		if (GetIAudioMgr()->GetSongMgr()->IsPlaying(gameHdl))
		{
			GetIAudioMgr()->GetSongMgr()->Play("GameSound", false, true, &gameHdl, 0.7f);
		}
	
	}
	break;
	}
	prevMode = currentMode;
}

void Render(float dTime)
{
	switch (modeManager.GetMode())
	{
	case 0:
	case 1:
		menu.Render(dTime);
		break;
	case 2:
		gGame.Render(dTime);
		break;
	case 3:
		instructions.Render(dTime);
		break;
	case 4:
		highScores.Render(dTime, gGame.GetHighScores(0), gGame.GetHighScores(1), gGame.GetHighScores(2), gGame.GetHighScores(3), gGame.GetHighScores(4), gGame.GetHighScores(5), gGame.GetHighScores(6), gGame.GetHighScores(7), gGame.GetHighScores(8), gGame.GetHighScores(9));
	}
}

void OnResize(int screenWidth, int screenHeight)
{
	switch (modeManager.GetMode())
	{
	case 0:
	case 1:
		menu.OnResize(screenWidth, screenHeight);
		break;
	case 2:
		gGame.OnResize(screenWidth, screenHeight);
		break;
	}
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (!fullscreen)
	{
		ShowWindow(hwnd, SW_SHOWMAXIMIZED);
		fullscreen = true;
	}

	switch (modeManager.GetMode())
	{
	case 0:
	case 1:
		switch (msg)
		{
		case WM_INPUT:
			menu.GetMouseAndKeys().MessageEvent((HRAWINPUT)lParam);
			break;
		}
		return menu.WindowsMssgHandler(hwnd, msg, wParam, lParam);
		break;
	case 2:
		switch (msg)
		{
		case WM_INPUT:
			gGame.GetMouseAndKeys().MessageEvent((HRAWINPUT)lParam);
			break;
		}
		return gGame.WindowsMssgHandler(hwnd, msg, wParam, lParam);
		break;
	case 3:
		switch (msg)
		{
		case WM_INPUT:
			instructions.GetMouseAndKeys().MessageEvent((HRAWINPUT)lParam);
			break;
		}
		return instructions.WindowsMssgHandler(hwnd, msg, wParam, lParam);
		break;
	case 4:
		switch (msg)
		{
		case WM_INPUT:
			highScores.GetMouseAndKeys().MessageEvent((HRAWINPUT)lParam);
			break;
		}
		return highScores.WindowsMssgHandler(hwnd, msg, wParam, lParam);
		break;
	}
}

void Save() //use XOR encryption 
{
	ofstream myfile("../data.txt");
	if (myfile.is_open())
	{
		for (int i = 0; i < 10; i++)
		{
			stringstream stream;
			stream << std::setprecision(4) << gGame.GetHighScores(i);


			string toEncrypt = stream.str(); //converts lap time to string
			char keyToEncrypt = 'm'; //109 in ascii
			;

			for (int temp = 0; temp < toEncrypt.size(); temp++)
				toEncrypt[temp] ^= keyToEncrypt; //now encrypted using XOR '^'

												 //myfile << gGame.GetHighScores(i) << "\n";
			myfile << toEncrypt << "\n";
		}
		myfile.close();

	}
}

void Load() //if tampered then make it so it will not load
{
	string line;
	ifstream myfile("../data.txt");
	if (myfile.is_open())
	{
		int i = 0;
		while (myfile.good())
		{
			getline(myfile, line);
			//get highscore stuff
			if (i < 10)
			{
				string toDecrypt = line;
				char keyToEncrypt = 'm'; //109 in ascii
				for (int temp = 0; temp < toDecrypt.size(); temp++) //cycles through string
				{
					toDecrypt[temp] ^= keyToEncrypt; //using key to decrypt using XOR '^'
					if (toDecrypt[temp] > 57 || toDecrypt[temp] < 45)
					{
						toDecrypt = '0';
					}
				}
				gGame.SetHighScores(i, std::stof(toDecrypt));
				i++;
			}

		}
	}
	myfile.close();

}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	if (!InitMainWindow(1024, 300, hInstance, "Average Karting", MainWndProc)) // changed to 300 to force our ratio
		assert(false);

	if (!InitDirect3D(OnResize))
		assert(false);

	File::initialiseSystem();

	new AudioMgrFMOD;
	GetIAudioMgr()->Initialise();

	//this should be a reference!!
	gGame.Initialise(&modeManager, menu);
	menu.Initialise(&modeManager, gGame);
	instructions.Initialise(&modeManager);
	highScores.Initialise(&modeManager);

	Load();
	Run(Update, Render);
	Save();

	gGame.Release();
	menu.Release();
	instructions.Release();
	highScores.Release();
	ReleaseD3D();

	delete GetIAudioMgr();
	

	return 0;
}
