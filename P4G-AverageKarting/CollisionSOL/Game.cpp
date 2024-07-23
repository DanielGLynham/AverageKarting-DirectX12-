#include "Game.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::vector<PickUp> mPickUps;

void Game::OnResize(int screenWidth, int screenHeight)
{
	OnResize_Default(screenWidth, screenHeight);
}

void Setup(Model& m, Mesh& source, float scale, const Vector3& pos, const Vector3& rot)
{
	m.Initialise(source);
	m.GetScale() = Vector3(scale, scale, scale);
	m.GetPosition() = pos;
	m.GetRotation() = rot;
}

void Game::LoadMap()
{
	//get the image
	int x, y, n;
	unsigned char *data = stbi_load("../bin/data/prettyMap.jpg", &x, &y, &n, 0); // needs to be a nice looking map
	assert(n == 3);
	//promote to 32bit, DX11 textures are RedGreenBlueAlpha (8bit per channel)
	//but the image was RGB 8bit which is annoying
	unsigned char *data32 = new unsigned char[4 * x*y];
	unsigned char* pSource = data, *pDest = data32;
	for (int i = 0; i < (x*y); ++i)
	{
		for (int ii = 0; ii < 3; ii++)
		{
			*pDest = *pSource;
			pDest++;
			pSource++;
		}
		*pDest = 255;
		pDest++;
	}
	//create a texture to match, yuck, this M$ code is nasty looking
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = x;
	desc.Height = y;
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.ArraySize = 1;
	ID3D11Texture2D *pTexture = NULL;
	HR(gd3dDevice->CreateTexture2D(&desc, NULL, &pTexture));
	//grab it to write into
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	HR(gd3dImmediateContext->Map(pTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	//copy the image data across
	memcpy(mappedResource.pData, data32, sizeof(unsigned char)*x*y * 4);
	gd3dImmediateContext->Unmap(pTexture, 0);
	//free our data, DX has it now
	stbi_image_free(data);
	delete[] data32;
	//convert the texture into a shader resource we can use
	D3D11_SHADER_RESOURCE_VIEW_DESC resViewDesc;
	ZeroMemory(&resViewDesc, sizeof(resViewDesc));
	resViewDesc.Format = desc.Format;
	resViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	resViewDesc.Texture2D.MostDetailedMip = 0;
	resViewDesc.Texture2D.MipLevels = 1;
	MaterialExt *pMat = &mQuad.GetMesh().GetSubMesh(0).material;
	HR(gd3dDevice->CreateShaderResourceView(pTexture, &resViewDesc, &pMat->pTextureRV));
	//we should really tell the texture cache about this or it won't get deleted :)

	//load the map and convert to a 2d array
	data = stbi_load("../bin/data/TrackMapMain.jpg", &x, &y, &n, 0); // load the dots map
	assert(n == 3);
	assert(x == MAP_WIDTH && y == MAP_WIDTH);
	for (int xi = 0; xi < MAP_WIDTH; ++xi)
		for (int yi = 0; yi < MAP_WIDTH; ++yi)
		{
			MapData& d = mTrack[MAP_WIDTH - 1 - yi][xi]; //invert it so it's orientated with the world coordinate system

			d.track = (data[yi*y * 3 + xi * 3] > 200 && d.obstacle == false) ? true : false; //red
			d.gravel = (data[yi*y * 3 + xi * 3 + 1] > 200) ? true : false; //green
			d.checkpoint = (data[yi*y * 3 + xi * 3 + 2] > 200) ? true : false; //blue

			if ((data[yi*y * 3 + xi * 3] > 200) && (data[yi*y * 3 + xi * 3 + 1] > 200))
			{
				d.obstacle = true;
				mObstacles.insert(mObstacles.end(), 1, Obstacle());
			}
			else d.obstacle = false;

			if ((data[yi*y * 3 + xi * 3 + 1] < 200) && (data[yi*y * 3 + xi * 3 + 1] > 100) && (data[yi*y * 3 + xi * 3 + 2] < 200) && (data[yi*y * 3 + xi * 3 + 2] > 100))
			{
				d.pickUp = true;
				mPickUps.insert(mPickUps.end(), 1, PickUp());
			}
			else d.pickUp = false;
		}
	stbi_image_free(data);
}
void Game::Load()
{
	FX::SetupDirectionalLight(0, true, Vector3(-0.7f, -0.7f, 0.7f), Vector3(0.8f, 0.8f, 0.8f), Vector3(0.15f, 0.15f, 0.15f), Vector3(0.25f, 0.25f, 0.25f));

	mpMenu->Load();

	mpSpriteBatch = new SpriteBatch(gd3dImmediateContext);
	assert(mpSpriteBatch);
	mpFont = new SpriteFont(gd3dDevice, L"../bin/data/comicSansMS.spritefont");
	assert(mpFont);

	mpFont2 = new SpriteFont(gd3dDevice, L"../bin/data/algerian.spritefont");
	assert(mpFont2);

	mCube.Initialise(BuildCube(mMeshMgr));
	mLoadData.loadedSoFar++;


	Mesh& sb = mMeshMgr.CreateMesh("skybox");
	sb.CreateFrom("data/skybox.fbx", gd3dDevice, mFX.mCache);
	Setup(mSkybox, sb, 300.f, Vector3(0, 0, 0), Vector3(270, 225, 300));
	mLoadData.loadedSoFar++;


	for (int i = 0; i < 4; i++)
	{
		cameras[i].Initialise(FPScamPos(), cars[i].GetRotation(), gViews[i]);
		cameras[i].LockMovementAxis(FPSCamera::UNLOCK, -4.5f, FPSCamera::UNLOCK);
	}

	mSphere.Initialise(BuildSphere(mMeshMgr, 16, 16));
	for (int i = 0; i < mPickUps.size(); i++)
	{
		mPickUps.at(i).Initialise(mMeshMgr);
		mLoadData.loadedSoFar++;
	}
	for (int i = 0; i < mObstacles.size(); i++)
	{
		mObstacles.at(i).Initialise(mMeshMgr, false); // set bool to true if you want the blocks to move up and down
		mLoadData.loadedSoFar++;
	}

	int x = 0; // counter for pickups
	int y = 0; // counter for obstacles
	vector<CheckPoint> allCheckPoints;
	for (int i = 0; i < MAP_WIDTH; i++)
	{
		for (int j = 0; j < MAP_WIDTH; j++) // loop over map, decide what's there and place objects accordingly
		{
			if (mTrack[j][i].obstacle)
			{
				mObstacles.at(y).mCube.GetScale() = Vector3(1, 1, 1);
				mObstacles.at(y).mCube.GetPosition() = Vector3(i, 0.5f, j);
				y++;
			}
			if (mTrack[j][i].pickUp)
			{
				float pickUpScale = 0.005f;
				mPickUps.at(x).mCube.GetScale() = Vector3(pickUpScale, pickUpScale, pickUpScale);
				mPickUps.at(x).mCube.GetPosition() = Vector3(i, pickUpScale, j); // moves it off the floor
				mPickUps.at(x).SetActivity(true); // start off visible
				x++;
			}
			if (mTrack[j][i].checkpoint)
			{
				allCheckPoints.push_back(CheckPoint());
				allCheckPoints.at(allCheckPoints.size() - 1).x = i; // set position
				allCheckPoints.at(allCheckPoints.size() - 1).y = j;
			}
		}
	}
	vector<CheckPoint> aCPList; // create a vector to store all the points from the map that make a checkpoint
	checkpoints.push_back(aCPList);
	checkpoints.at(0).push_back(allCheckPoints.at(0)); // insert the first checkpoint as a start point
	for (int i = 0; i < allCheckPoints.size(); i++) // for every checkpoint point on the map
	{
		if (allCheckPoints.at(i).used == false) // if it isn't already put into a checkpoint vector
		{
			CheckPointsConnectedCheck(allCheckPoints.at(i), allCheckPoints); // check if point is connected to a checkpoint
			allCheckPoints.at(i).used = true; // it's been collected by a checkpoint vector now so don't check it again
			vector<CheckPoint> CPList;
			checkpoints.push_back(CPList);
		}

		mMKInput.Initialise(GetMainWnd());
		mGamepad.Initialise();
	}
	mLoadData.loadedSoFar++;
	for (int i = 0; i < 4; i++) // Initialise players
	{
		cars[i].Initialise(mMeshMgr, i);
		mLoadData.loadedSoFar++;
		int CP = 4;          // decides which checkpoint to spawn players at
		int pPosx = checkpoints.at(CP).at((checkpoints.at(CP).size() / 6) * (i + 2)).x;   // seperates the players out nicely on checkpoint
		int pPosy = checkpoints.at(CP).at((checkpoints.at(CP).size() / 6) * (i + 2)).y;
		cars[i].mCar.GetPosition() = Vector3(pPosx, 0.2, pPosy);
	}

	for (int i = 0; i < checkpoints.size(); i++) // creates a vector<bool> which tracks the checkpoints for each car
	{
		for (int y = 0; y < 4; y++)
		{
			bool pushable = false;
			cars[y].GetCheckPointsPassed().push_back(pushable);
		}
	}
	mLoadData.running = false;
}


void Game::LoadDisplay(float dTime)
{
	
}

void Game::Initialise(ModeManager* mmPtr, Menu& menu)
{
	mpMenu = &menu;
	this->mmPtr = mmPtr;

	mFX.Init(gd3dDevice);

	// map
	mQuad.Initialise(BuildQuad(mMeshMgr));
	MaterialExt *pMat = &mQuad.GetMesh().GetSubMesh(0).material;
	pMat->gfxData.Set(Vector4(0.9f, 0.8f, 0.8f, 0), Vector4(0.9f, 0.8f, 0.8f, 0), Vector4(0, 0, 0, 1));
	int scale = MAP_WIDTH / 2;
	mQuad.GetPosition() = Vector3(scale, 0, scale);
	mQuad.GetRotation() = Vector3(0, 0, 0);
	mQuad.GetScale() = Vector3(scale, 1, scale);
	mLoadData.loadedSoFar++;

	LoadMap();

	mLoadData.totalToLoad = mPickUps.size() + mObstacles.size() + 3;
	mLoadData.loadedSoFar = 0;
	mLoadData.running = true;
	mLoadData.loader = std::async(launch::async, &Game::Load, this);
	mInitialised = true;
}

void Game::CheckPointsConnectedCheck(CheckPoint point, vector<CheckPoint>& allCheckPoints)
{
	int x = point.x;
	int y = point.y;
	CheckPoint up, down, left, right;
	up.x = x; up.y = y + 1; up.used = false;
	down.x = x; down.y = y - 1; down.used = false;
	left.x = x - 1; left.y = y; left.used = false;
	right.x = x + 1; right.y = y; right.used = false; // each avaliable position, ie up, down, left, right

	for (int i = 0; i < allCheckPoints.size(); i++) // check every checkpoint
	{
		if ((up.x == allCheckPoints.at(i).x && up.y == allCheckPoints.at(i).y)) // if one space above point is a checkpoint then include it to our checkpoint
		{
			if (allCheckPoints.at(i).used == false) // make sure we haven't already done this
			{
				checkpoints.at(checkpoints.size() - 1).push_back(allCheckPoints.at(i)); // add it to our vector
				allCheckPoints.at(i).used = true; // make sure we don't do this one again
				CheckPointsConnectedCheck(up, allCheckPoints); // continue the search 
			}
		} // do this for every up/down/left/right until there are no more points in any direction.
		if ((down.x == allCheckPoints.at(i).x && down.y == allCheckPoints.at(i).y))
		{
			if (allCheckPoints.at(i).used == false)
			{
				checkpoints.at(checkpoints.size() - 1).push_back(allCheckPoints.at(i));
				allCheckPoints.at(i).used = true;
				CheckPointsConnectedCheck(down, allCheckPoints);
			}
		}
		if ((left.x == allCheckPoints.at(i).x && left.y == allCheckPoints.at(i).y))
		{
			if (allCheckPoints.at(i).used == false)
			{
				checkpoints.at(checkpoints.size() - 1).push_back(allCheckPoints.at(i));
				allCheckPoints.at(i).used = true;
				CheckPointsConnectedCheck(left, allCheckPoints);
			}
		}
		if ((right.x == allCheckPoints.at(i).x && right.y == allCheckPoints.at(i).y))
		{
			if (allCheckPoints.at(i).used == false)
			{
				checkpoints.at(checkpoints.size() - 1).push_back(allCheckPoints.at(i));
				allCheckPoints.at(i).used = true;
				CheckPointsConnectedCheck(right, allCheckPoints);
			}
		}
	} // now we have every point on the map for a single checkpoint so go back and get the next point for another checkpoint
}

void Game::Release()
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

bool Game::FinishedLoading()
{
	if (!mInitialised)
		return false;
	return !mLoadData.running;
}


void Game::Update(float dTime)
{
	//don't update anything that relies on loaded assets until they are loaded
	if (mLoadData.running)
		return;

	mGamepad.Update();
	for (int i = 0; i < 4; i++) // update every car
	{
		for (int j = 0; j < mObstacles.size(); j++) // obstacles move if we want them to so use update
		{
			mObstacles.at(j).Update(dTime);
		}
		for (int j = 0; j < mPickUps.size(); j++)
		{
			mPickUps.at(j).Update(dTime);
		}
		int helpfulLoopCounter = cars[i].GetCheckPointsPassed().size() - 1; // how many checkpoints this car has passed so far
		cars[i].lapTimer += dTime; 
		////// make sure the camera is at the right position with the car
		float carYRotation = cars[i].GetRotation().y;
		mCamPos.x = cars[i].GetPos().x + camRadius * cos((1.5 * M_PI) - fmodf(carYRotation, 2 * M_PI));
		mCamPos.y = cars[i].GetPos().y + 1.0f;
		mCamPos.z = cars[i].GetPos().z + camRadius * sin((1.5 * M_PI) - fmodf(carYRotation, 2 * M_PI));
		////// who is responsibile, why? don't leave it in if not needed
		//don't update anything that relies on loaded assets until they are loaded
		//if (mLoadData.running)
		//	return;
		cars[i].Update(dTime, isOnTrack(cars[i].GetCar()), mCamPos, mMKInput); // let the car update itself
		if (isOnTrack(cars[i].GetCar()) < 3) // not a checkpoint
		{
			isOn = false;
			cars[i].doesntCount = false; // fixes bug
		}
		else if (isOnTrack(cars[i].GetCar()) == 3) // is on a checkpoint
		{
				for (int j = 0; j < helpfulLoopCounter; j++) // bools for checkpoints
				{
					if (cars[i].GetCheckPointsPassed().at(j) == false) // only check not got to yet checkpoints
					{
						for (int k = 0; k < checkpoints.at(j).size(); k++) // all blue pixels in that checkpoint
						{
							if (!cars[i].doesntCount) // hasn't just finished a lap (fixes bug)
							{
								int carx = (int)cars[i].GetCar().GetPosition().x;
								int cary = (int)cars[i].GetCar().GetPosition().z;
								if (carx == checkpoints.at(j).at(k).x && cary == checkpoints.at(j).at(k).y) // if on a checkpoint
								{
									cars[i].GetCheckPointsPassed().at(j) = true; // set this car to have passed the checkpoint
								}
							}
						}
					}
				}
		}
		bool lapcomplete = true;
		for (int x = 0; x < helpfulLoopCounter; x++) // go through checkpoints 
		{
			if (cars[i].GetCheckPointsPassed().at(x) == false) // if car hasn't been to all checkpoints yet
			{
				lapcomplete = false; // lap not complete
			}
		}
		if (lapcomplete) 
		{
			// do highscore stuff
			if (cars[i].lapTimer < currentBestTime || currentBestTime == 0)
			{
				currentBestTime = cars[i].lapTimer;
			}
			if (cars[i].lapTimer < cars[i].currentBestTime || cars[i].currentBestTime == 0)
			{
				cars[i].currentBestTime = cars[i].lapTimer;
			}
			SetAHighScore(cars[i].lapTimer);
			cars[i].lapTimer = 0; // reset for another lap
			cars[i].laps++; // inc counter
			cars[i].doesntCount = true; // sorts bug -- don't count current checkpoint as a checkpoint (ask Dan or check GitHub)
			for (int x = 0; x < helpfulLoopCounter; x++) // reset cars checkpoints so we can go over them again
			{
				cars[i].GetCheckPointsPassed().at(x) = false;
			}
		}
		if (cars[i].laps == 3) // end game 
		{
			cars[i].laps = 0;
			mmPtr->SetMode(4);

		}
	}
	SortPositions(); // as in 1st place, 2nd place, etc.
}

void Game::SortPositions()
{
	for (int i = 0; i < 4; i++) // for each car, add up how many laps and checkpoints they have passed
	{
		int checkpointsPassed = 0;
		for (int x = 0; x < cars[i].GetCheckPointsPassed().size(); x++)
		{
			if (cars[i].GetCheckPointsPassed().at(x) == true)
			{
				checkpointsPassed++;
			}
		}
		cars[i].helpfulCounter += (cars[i].laps * checkpoints.size()) + checkpointsPassed;
	}
	for (int i = 0; i < 4; i++) // sort order - heighest to lowest
	{
		int anotherhelpfulCounter = 0;
		if (cars[i].helpfulCounter > cars[0].helpfulCounter)
		{
			anotherhelpfulCounter++;
		}
		if (cars[i].helpfulCounter > cars[1].helpfulCounter)
		{
			anotherhelpfulCounter++;
		}
		if (cars[i].helpfulCounter > cars[2].helpfulCounter)
		{
			anotherhelpfulCounter++;
		}
		if(cars[i].helpfulCounter > cars[3].helpfulCounter)
		{
			anotherhelpfulCounter++;
		}
		cars[i].place = 4 - anotherhelpfulCounter;
	}
}

void Game::ResetGame()
{
	for (int i = 0; i < mPickUps.size(); i++)
		mPickUps.at(i).SetActivity(true);
	for (int i = 0; i < mObstacles.size(); i++)
		mObstacles.at(i).mCube.GetPosition().y = 0.5;

	for (int i = 0; i < 4; i++)
	{
		cars[i].Start(i);
	}
}

DirectX::SimpleMath::Vector3 Game::FPScamPos() 
{ 

	float carYRotation = mCar.GetRotation().y;
	return Vector3(mCar.GetPos().x + camRadius * cos((1.5 * M_PI) - fmodf(carYRotation, 2 * M_PI)),
		mCar.GetPos().y + 1.0f,
		mCar.GetPos().z + camRadius * sin((1.5 * M_PI) - fmodf(carYRotation, 2 * M_PI)));
}

void CollisionManager(const BoundingBox& box, const BoundingSphere& sphere, float vel, Vector3& pos, float COR, float dTime, BallSim& car) // car to cube collision
{
	Vector3 cn;
	if (SphereToSphere(sphere, BoundingSphere(box.Center, box.Extents.x*1.5f), cn))
	{
		//cube collision
		if (SphereToAABBox(box, sphere, cn))
		{
			car.Crash();
		}
	}
}
void CollisionManagerPickUps(const BoundingBox& box, const BoundingSphere& sphere, float vel, Vector3& pos, float COR, float dTime, BallSim& car, int pickUpNum) // car to pickUp collision
{
	Vector3 cn;
	if (SphereToSphere(sphere, BoundingSphere(box.Center, box.Extents.x*1.5f), cn))
	{
		//cube collision
		if (SphereToAABBox(box, sphere, cn))
		{
			mPickUps.at(pickUpNum).Collect();
			car.PickUpObtained(car);
		}
	}
}

void Game::Render(float dTime)
{
	if (mLoadData.running)
	{
		if (!mLoadData.loader._Is_ready())
		{
			LoadDisplay(dTime);
			return;
		}
		mLoadData.loader.get();
		mLoadData.running = false;
		return;
	}

	BeginRender(Colours::White);

	int sw, sh;
	GetClientExtents(sw, sh);
	CreateProjectionMatrix(FX::GetProjectionMatrix(), 0.25f*PI, sw / (sh*0.5f), 1, 1000.f);

	for (int i = 0; i < 4; i++) // for each car
	{
		cameras[i].Move(dTime, cars[i].GetPos(), cars[i].GetRotation());
		switch (i) // set the viewport for each car in the correct place
		{
		case 0:
			SetViewportDimensions(gd3dImmediateContext, 0, 0, sw / 2, sh / 2, gScreenViewport);
			break;
		case 1:
			SetViewportDimensions(gd3dImmediateContext, sw / 2, 0, sw / 2, sh / 2, gScreenViewport);
			break;
		case 2:
			SetViewportDimensions(gd3dImmediateContext, 0, sh / 2, sw / 2, sh / 2, gScreenViewport);
			break;
		case 3:
			SetViewportDimensions(gd3dImmediateContext, sw / 2, sh / 2, sw / 2, sh / 2, gScreenViewport);
			break;
		}

			
		FX::SetPerFrameConsts(gd3dImmediateContext, cameras[i].GetPos());
		FX::GetViewMatrix() = gViews[i] * scaling;

		MaterialExt& mat = mSkybox.GetMesh().GetSubMesh(0).material;
		mat.flags &= ~MaterialExt::LIT;
		mat.flags &= ~MaterialExt::ZTEST;
		mFX.Render(mSkybox, gd3dImmediateContext);

		for (int j = 0; j < 4; j++) // all the cars
		{
			cars[j].Render(mFX, dTime);
		}

		mFX.Render(mQuad, gd3dImmediateContext); // floor

		for (int j = 0; j < mObstacles.size(); j++) // obstacles
		{
			mObstacles.at(j).Render(mFX, dTime);
		}
		// collision checker
		for (int j = 0; j < mObstacles.size(); j++)
		{
			CollisionManager(BoundingBox(mObstacles.at(j).mCube.GetPosition(), Vector3(1, 1, 1)), BoundingSphere(cars[i].GetPos(), cars[i].mCar.GetScale().x), 1, cars[i].GetPos(), cars[i].mCOR, dTime, cars[i]);
		}

		for (int j = 0; j < mPickUps.size(); j++) // render pickups if they are actice
		{
			if (mPickUps.at(j).GetActivity() == true)
			{
				mPickUps.at(j).Render(mFX, dTime);
			}
		}
		for (int j = 0; j < mPickUps.size(); j++)
		{
			if (mPickUps.at(j).GetActivity() == true)
			{
				CollisionManagerPickUps(BoundingBox(mPickUps.at(j).mCube.GetPosition(), Vector3(1, 1, 1)), BoundingSphere(cars[i].GetPos(), 0.2f), 1, cars[i].GetPos(), cars[i].mCOR, dTime, cars[i], j);
			}
		}
	}
	for (int i = 0; i < 4; i++) // not sure why we can't do all of this in the one for loop but I tested it and we defo do need both
	{
		switch (i)
		{
		case 0:
			SetViewportDimensions(gd3dImmediateContext, 0, 0, sw / 2, sh / 2, gScreenViewport);
			break;
		case 1:
			SetViewportDimensions(gd3dImmediateContext, sw / 2, 0, sw / 2, sh / 2, gScreenViewport);
			break;
		case 2:
			SetViewportDimensions(gd3dImmediateContext, 0, sh / 2, sw / 2, sh / 2, gScreenViewport);
			break;
		case 3:
			SetViewportDimensions(gd3dImmediateContext, sw / 2, sh / 2, sw / 2, sh / 2, gScreenViewport);
			break;
		}
		CommonStates state(gd3dDevice);
		mpSpriteBatch->Begin(SpriteSortMode_Deferred, state.NonPremultiplied());
		wstringstream oT;

		switch (cars[i].place)
		{
		case 1:
			oT << cars[i].place << "st place";
			break;
		case 2: 
			oT << cars[i].place << "nd place";
			break;
		case 3:
			oT << cars[i].place << "rd place";
			break;
		case 4:
			oT << cars[i].place << "th place";
			break;
		}
		if (currentBestTime == 0)
		{
			oT << "\nBest lap: Hasn't been set";
		}
		else
		{
		oT << "\nBest lap: " << currentBestTime;
		}
		if (cars[i].currentBestTime == 0)
		{
			oT << "\npersonal best: Hasn't been set";
		}
		else
		{
			oT << "\npersonal best: " << cars[i].currentBestTime;
		}
		

		mpFont->DrawString(mpSpriteBatch, oT.str().c_str(), Vector2(10, 100), Colours::Red, 0, Vector2(0, 0), Vector2(1, 1));
		mpSpriteBatch->End();
	}
	EndRender();

	mMKInput.PostProcess();
	mMKInput.PostProcess();
}
int Game::isOnTrack(Model& car)
{
	int x = (int)car.GetPosition().x;
	int z = (int)car.GetPosition().z;
	if (mTrack[z][x].track || mTrack[z][x].pickUp)
	{
		return 1;
	}
	else if (mTrack[z][x].gravel)
	{
		return 2;
	}
	else if (mTrack[z][x].checkpoint)
	{
		return 3;
	}
	else
	{
		return 0;
	}
}
float Game::GetHighScores(int pos)
{
	return highScores[pos];
}

void Game::SetAHighScore(float highScore)
{
	bool scoreset = false;
	for (int i = 0; i < 10; i++) 
	{
		if (highScore < highScores[i] && scoreset == false)
		{
			scoreset = true;
			for (int x = 0; x < 9 - i; x++) // this pushes down the values under the new highscore
			{
				int y = 9 - x;
				highScores[y] = highScores[y - 1];
			}
			highScores[i] = highScore;
			return;
		}
	}
}
void Game::SetHighScores(int pos, float highScore)
{
	(highScores[pos]) = highScore;
}

void Game::SetMoveObstacles(bool moveObstacles)
{
	for (int i = 0; i < mObstacles.size(); i++)
		mObstacles.at(i).SetWantToMove(moveObstacles);
}

LRESULT Game::WindowsMssgHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	const float camInc = 20.f * GetElapsedSec();
	switch (msg)
	{
		// Responds to a keyboard event.
	case WM_CHAR:
		switch (wParam)
		{
		case 27:
		case 'q':
		case 'Q':
			PostQuitMessage(0);
			return 0;
		case 'm':
		case 'M':
			mmPtr->SetMode(0);
			break;
		case ' ':
			mCamPos = mDefCamPos;
			break;
		}
	case WM_INPUT:
		mMKInput.MessageEvent((HRAWINPUT)lParam);
		break;
	}
	//default message handling (resize window, full screen, etc)
	return DefaultMssgHandler(hwnd, msg, wParam, lParam);
}