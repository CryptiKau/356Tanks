#include "stdafx.h"
#include "Ass3.h"
#include <stdlib.h>
 
//-------------------------------------------------------------------------------------
Ass3::Ass3(void)
{
	mWalkSpd = 500.0;
	teamSize = 3;

	for (int i = 0; i < MAX_TEAM_SIZE; i++)
	{
		amDistance[i] = 0;
		bmDistance[i] = 0;
		amDirect[i] = Ogre::Vector3::ZERO;
		bmDirect[i] = Ogre::Vector3::ZERO;
		amDestination[i] = Ogre::Vector3::ZERO;
		bmDestination[i] = Ogre::Vector3::ZERO;
	}
}
//-------------------------------------------------------------------------------------
Ass3::~Ass3(void)
{
	if(pathFindingGraph)
		delete pathFindingGraph;
}

bool Ass3::setup(void)
{
	BaseApplication::setup();

	for (int i = 0; i < 29; i++)
	{
		posMoves[i].x = 0;
		posMoves[i].y = 0;
		posMoves[i].z = 0;
	}

	// Set the camera's position and where it is looking
	mCamera->setPosition(0, 100, 300);
	mCamera->lookAt(0, 50, 0);

	// Display the mouse cursor
	mTrayMgr->showCursor();

	mCurrentTank = -1;

	pathFindingGraph = new Graph;
	//mCurrentState = 0;

	mDirection = Ogre::Vector3::ZERO;

	srand(std::time(NULL));

	return true;
};

void Ass3::createCamera(void)
{
	mCamera = mSceneMgr->createCamera("PlayerCam");
 
	mCamera->setPosition(Ogre::Vector3(0,0,40));
	mCamera->setNearClipDistance(5);
 
	mCameraMan = new OgreBites::SdkCameraMan(mCamera);   // create a default camera controller
}

//-------------------------------------------------------------------------------------
void Ass3::createScene(void)
{
	Ogre::SceneNode* freeCam = mSceneMgr->getRootSceneNode()->createChildSceneNode ("freeCam", Ogre::Vector3(0, 900, 100));
	freeCam->attachObject(mCamera);
	
	//skydome
	mSceneMgr->setSkyDome(true, "Examples/CloudySky", 2, 4);
	
	//Create Fog
	Ogre::ColourValue fadeColour(0.9, 0.9, 0.9);
	mWindow->getViewport(0)->setBackgroundColour(fadeColour);

	mSceneMgr->setFog(Ogre::FOG_EXP, fadeColour, 0.0003);
	//mSceneMgr->setFog(Ogre::FOG_EXP2, fadeColour, 0.0004);

	// Set ambient lighting
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.2, 0.2, 0.2));
	Ogre::Vector3 lightdir(0.55, -0.55, 0.75);
	lightdir.normalise();
	mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);
 
	Ogre::Light* light = mSceneMgr->createLight("TestLight");
	light->setType(Ogre::Light::LT_DIRECTIONAL);
	light->setDirection(lightdir);
	light->setDiffuseColour(Ogre::ColourValue::White);
	light->setSpecularColour(Ogre::ColourValue(0.4, 0.4, 0.4));
	
	// Use a plane to represent the ground
	Ogre::Plane plane(Ogre::Vector3::UNIT_Y, 0);
	Ogre::MeshManager::getSingleton().createPlane("ground", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane, 
		3000, 3000, 20, 20, true, 1, 2.5, 2.5, Ogre::Vector3::UNIT_Z);

	Ogre::Entity* entGround = mSceneMgr->createEntity("GroundEntity", "ground");
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(entGround);
	entGround->setMaterialName("Examples/Grass");
	entGround->setCastShadows(false);

	// Create robots
	//mRobot[1] = mSceneMgr->createEntity("Robot2", "robot.mesh");
	//mRobot[1]->setCastShadows(true);
	// Attach robots to scene nodes and position them appropriately
	//Set random value between -1250 and 1250
	//mRobotNode[1] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	//mRobotNode[1]->attachObject(mRobot[1]);
	//mRobotNode[1]->yaw(Ogre::Degree(-90));
	//mRobotNode[1]->translate(1300, 18, -1250);
	// Initialise the robot's health
	//mHealth[1] = 0.5;

	//create tanks
	for (int i = 0; i < MAX_TEAM_SIZE; i++)
	{
		//team A
		createTank(0,i);
		//team B
		createTank(1,i);
	}

	//spawn tanks
	for (int i = 0; i < teamSize; i++)
	{
		//team A
		spawnTank(0,i);
		//team B
		spawnTank(1,i);
	}

	// Create a BillboardSet to represent a health bar and set its properties
	mHealthBar = mSceneMgr->createBillboardSet("Healthbar1");
	mHealthBar->setCastShadows(false);
	mHealthBar->setDefaultDimensions(150, 15);
	mHealthBar->setMaterialName("myMaterial/HealthBar");

	// Create a billboard for the health bar BillboardSet
	mHealthBarBB = mHealthBar->createBillboard(Ogre::Vector3(0, 100, 0));

	// Set it to always draw on top of other objects
	mHealthBar->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY);

	// Create a BillboardSet for a selection circle and set its properties
	mSelectionCircle = mSceneMgr->createBillboardSet("SelectionCircle1");
	mSelectionCircle->setCastShadows(false);
	mSelectionCircle->setDefaultDimensions(60, 60);
	mSelectionCircle->setMaterialName("myMaterial/SelectionCircle");
	mSelectionCircle->setBillboardType(Ogre::BillboardType::BBT_PERPENDICULAR_COMMON);
	mSelectionCircle->setCommonDirection(Ogre::Vector3(0, 1, 0));
	mSelectionCircle->setCommonUpVector(Ogre::Vector3(0, 0, -1));

	// Create a billboard for the selection circle BillboardSet
	mSelectionCircleBB = mSelectionCircle->createBillboard(Ogre::Vector3(0, 1, 0));
	mSelectionCircleBB->setTexcoordRect(0.0, 0.0, 1.0, 1.0);
	
	// Set background colour to dark grey so that you can see the health bar
	mWindow->getViewport(0)->setBackgroundColour(Ogre::ColourValue(0.2f, 0.2f, 0.2f));


	///////////////////////////

	for(int nodeNumber=0; nodeNumber<TOTAL_NODES; nodeNumber++)
	{
		int contents = pathFindingGraph->getContent(nodeNumber);

		if(contents)
		{
			// Create unique name
			std::ostringstream oss;
			oss << nodeNumber;
			std::string entityName = "Cube" + oss.str();

			// Create entity
			Ogre::Entity* cube = mSceneMgr->createEntity(entityName, "cube.mesh");
			cube->setMaterialName("Examples/RustySteel");
			cube->setCastShadows(true);

			// Attach entity to scene node
			Ogre::SceneNode* myNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
			myNode->attachObject(cube);
			myNode->scale(1.0, 0.5, 1.0);
			
			// Place object at appropriate position
			Ogre::Vector3 position = pathFindingGraph->getPosition(nodeNumber);
			position.y = 0;
			myNode->translate(position);
		}
	}

	// create the path object, and clear it to start off
	for (int i = 0; i < MAX_TEAM_SIZE; i++)
	{
		std::string apathName = "apath"; std::string bpathName = "bpath";
		apathName += std::to_string(i); bpathName += std::to_string(i);
		apath[i] = mSceneMgr->createManualObject(apathName);
		bpath[i] = mSceneMgr->createManualObject(bpathName);
		apath[i]->clear();
		bpath[i]->clear();
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(apath[i]);
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(bpath[i]);
	}	
}

bool Ass3::processUnbufferedInput(const Ogre::FrameEvent& evt)
{
	Ogre::Vector3 transVector = Ogre::Vector3::ZERO;
	mMoveScale = 750;
	mZoomScale = 20000;
	
	if(mMouse->getMouseState().X.abs > (mWindow->getWidth() - 20)) 
	{ 
		transVector.x += mMoveScale; 
	}
	if(mMouse->getMouseState().X.abs < (0+20)) 
	{ 
		transVector.x -= mMoveScale; 
	}
	if(mMouse->getMouseState().Y.abs < (0+20)) 
	{ 
		transVector.z -= mMoveScale; 
	}
	if(mMouse->getMouseState().Y.abs > (mWindow->getHeight() - 20)) 
	{ 
		transVector.z += mMoveScale; 
	}
	if (mMouse->getMouseState().Z.rel < 0)
	{
		transVector.y += mZoomScale;
	}
	if (mMouse->getMouseState().Z.rel > 0)
	{
		transVector.y -= mZoomScale;
	}
	mSceneMgr->getSceneNode("freeCam")->translate(transVector *evt.timeSinceLastFrame, Ogre::Node::TS_LOCAL);


	return true;
}

bool Ass3::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
	bool ret = BaseApplication::frameRenderingQueued(evt);
	processUnbufferedInput(evt);



	for (int i = 0; i < teamSize; i++)
	{
			moveTank(0,i,evt.timeSinceLastFrame);
			moveTank(1,i,evt.timeSinceLastFrame);
	}

	//display healthbar if a tank is selected
	if (mCurrentTank != -1)
	{
		if (mCurrentTank < MAX_TEAM_SIZE) //team A tank selected
		{
			// Calculate the health bar adjustments
			float healthBarAdjuster = (1.0 - amTankHealth[mCurrentTank])/2;	// This must range from 0.0 to 0.5
			// Set the health bar to the appropriate level
			mHealthBarBB->setTexcoordRect(0.0 + healthBarAdjuster, 0.0, 0.5 + healthBarAdjuster, 1.0);
		}
		else //team B tank selected
		{
			// Calculate the health bar adjustments
			float healthBarAdjuster = (1.0 - bmTankHealth[mCurrentTank - MAX_TEAM_SIZE])/2;	// This must range from 0.0 to 0.5
			// Set the health bar to the appropriate level
			mHealthBarBB->setTexcoordRect(0.0 + healthBarAdjuster, 0.0, 0.5 + healthBarAdjuster, 1.0);
		}
	}

	return ret;
}

bool Ass3::mouseMoved( const OIS::MouseEvent &arg )
{
	if (mTrayMgr->injectMouseMove(arg)) return true;

	// only rotate camera if the right mouse button is held
	if (arg.state.buttonDown(OIS::MB_Right))
	{
		mCamera->yaw(Ogre::Degree(-0.1 * arg.state.X.rel));
		mCamera->pitch(Ogre::Degree(-0.1 * arg.state.Y.rel));
	}

	return true;
}

bool Ass3::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	if (mTrayMgr->injectMouseDown(arg, id)) return true;


	switch (id)
	{
		case OIS::MB_Left:
			{

				//mCurrentState = 0;

				// Get the mouse ray, i.e. ray from the mouse cursor 'into' the screen 
				Ogre::Ray mouseRay = mCamera->getCameraToViewportRay(
					static_cast<float>(mMouse->getMouseState().X.abs)/mMouse->getMouseState().width, 
					static_cast<float>(mMouse->getMouseState().Y.abs)/mMouse->getMouseState().height);

				Ogre::RaySceneQuery * mRaySceneQuery = mSceneMgr->createRayQuery(mouseRay);

				// Set type of objects to query
				mRaySceneQuery->setQueryTypeMask(Ogre::SceneManager::ENTITY_TYPE_MASK);
			
				mRaySceneQuery->setSortByDistance(true);

				// Ray-cast and get first hit
				Ogre::RaySceneQueryResult &result = mRaySceneQuery->execute();
				Ogre::RaySceneQueryResult::iterator itr = result.begin();

				// If hit a movable object
				if(itr != result.end() && itr->movable)
				{

					// Store previously selected tank
					int previous = mCurrentTank;

					// Get name of movable object that was hit
					Ogre::String name = itr->movable->getName();

					// Check name with selectable entity names and set selected tank
					bool found = false;
					int i = 0;
					while (i < MAX_TEAM_SIZE && !found)
					{
						//team A clicked
						std::string body = "chbodyA", turret = "chturretA", barrel = "chbarrelA";
						body += std::to_string(i); turret += std::to_string(i); barrel += std::to_string(i); 
						if (name == body || name == turret || name == barrel)
						{
							mCurrentTank = i;
							found = true;
						}
						else //team B clicked
						{
							body = "chbodyB", turret = "chturretB", barrel = "chbarrelB";
							body += std::to_string(i); turret += std::to_string(i); barrel += std::to_string(i);
							if (name == body || name == turret || name == barrel)
							{
								mCurrentTank = i + MAX_TEAM_SIZE;
								found = true;
							}
							else //not-tank clicked
							{ mCurrentTank = -1; }
						}
						i++;
					}

					// If there was a previously selected tank
					if(previous != -1)
					{
						if (previous < MAX_TEAM_SIZE) //team A
						{
							// Detach the healthbar and selection circle
							amTankBodyNode[previous]->detachObject(mHealthBar);
							amTankBodyNode[previous]->detachObject(mSelectionCircle);
						}
						else //team B
						{
							// Detach the healthbar and selection circle
							bmTankBodyNode[previous - MAX_TEAM_SIZE]->detachObject(mHealthBar);
							bmTankBodyNode[previous - MAX_TEAM_SIZE]->detachObject(mSelectionCircle);
						}
					}

					// If there is a selected tank
					if(mCurrentTank > -1)
					{
						if (mCurrentTank < MAX_TEAM_SIZE) //team A tank selected
						{
							// Calculate the health bar adjustments
							float healthBarAdjuster = (1.0 - amTankHealth[mCurrentTank])/2;	// This must range from 0.0 to 0.5
							// Set the health bar to the appropriate level
							mHealthBarBB->setTexcoordRect(0.0 + healthBarAdjuster, 0.0, 0.5 + healthBarAdjuster, 1.0);

							// Attach the healthbar and selection circle
							amTankBodyNode[mCurrentTank]->attachObject(mHealthBar);
							amTankBodyNode[mCurrentTank]->attachObject(mSelectionCircle);
						}
						else //team B tank selected
						{
							// Calculate the health bar adjustments
							float healthBarAdjuster = (1.0 - bmTankHealth[mCurrentTank - MAX_TEAM_SIZE])/2;	// This must range from 0.0 to 0.5
							// Set the health bar to the appropriate level
							mHealthBarBB->setTexcoordRect(0.0 + healthBarAdjuster, 0.0, 0.5 + healthBarAdjuster, 1.0);

							// Attach the healthbar and selection circle
							bmTankBodyNode[mCurrentTank - MAX_TEAM_SIZE]->attachObject(mHealthBar);
							bmTankBodyNode[mCurrentTank - MAX_TEAM_SIZE]->attachObject(mSelectionCircle);
						}

						Ogre::Vector3 location = mouseRay.getPoint(itr->distance);

						//if(mCurrentState == 0)
						//{
						//	// set start node
						//	startNode = pathFindingGraph->getNode(location);
						//	// set state to goal node state
						//	mCurrentState++;
						//}
					}
				}
				// If nothing selected
				else
				{
					// Nothing selected and there is a previously selected tank
					if(mCurrentTank != -1)
					{
						if (mCurrentTank < MAX_TEAM_SIZE) //team A
						{
							// Detach the healthbar and selection circle
							amTankBodyNode[mCurrentTank]->detachObject(mHealthBar);
							amTankBodyNode[mCurrentTank]->detachObject(mSelectionCircle);
						}
						else //team B
						{
							// Detach the healthbar and selection circle
							bmTankBodyNode[mCurrentTank - MAX_TEAM_SIZE]->detachObject(mHealthBar);
							bmTankBodyNode[mCurrentTank - MAX_TEAM_SIZE]->detachObject(mSelectionCircle);
						}

						// Set to no tank selected
						mCurrentTank = -1;
					}
				}
			}
			break;
		/*
			case OIS::MB_Right:
			{
				// if path already exists
				if(mCurrentState > 1)
				{
					// reset
					mCurrentState = 0;
					path1->clear();
				
				}
				// if no path yet
				else
				{
					// Create RaySceneQuery
					Ogre::Ray mouseRay = mCamera->getCameraToViewportRay(
						static_cast<float>(mMouse->getMouseState().X.abs)/mMouse->getMouseState().width, 
						static_cast<float>(mMouse->getMouseState().Y.abs)/mMouse->getMouseState().height);

					Ogre::RaySceneQuery * mRaySceneQuery = mSceneMgr->createRayQuery(Ogre::Ray());

					// Set ray
					mRaySceneQuery->setRay(mouseRay);

					// Ray-cast and get first hit
					Ogre::RaySceneQueryResult &result = mRaySceneQuery->execute();
					Ogre::RaySceneQueryResult::iterator itr = result.begin();

					// if hit an object
					if(itr != result.end())
					{
						// Get hit location
						Ogre::Vector3 location = mouseRay.getPoint(itr->distance);

						// if hit the floor
						if(location.y < 0.001)
						{
							// if no start node yet
							if(mCurrentState == 0)
							{
								// set start node
								startNode = pathFindingGraph->getNode(location);
								// set state to goal node state
								mCurrentState++;
							}
							// if start node already assigned
							else if(mCurrentState == 1)
							{
								// set goal node
								goalNode = pathFindingGraph->getNode(location);

								// check that goal node is not the same as start node
								if(goalNode != startNode)
								{
									// try to find path from start to goal node
									std::vector<int> path;

									// if path exists
									if(mPathFinder.AStar(startNode, goalNode, *pathFindingGraph, path))
									{
										// draw path
										goPath(path1, 0.5, path, Ogre::ColourValue(1, 0, 0));

										// set state to path found
										mCurrentState++;
									}
									else
									{
										// no path so set state to no start node
										mCurrentState = 0;
									}
								}
							}
						}
					}
				}
			}
			break;
		*/
		default:
			break;
	}
	return true;
}

bool Ass3::keyPressed( const OIS::KeyEvent &arg)
{
	BaseApplication::keyPressed(arg);

	Ogre::Vector3 randDest = Ogre::Vector3(0,18,0);

	switch (arg.key)
	{
		case OIS::KC_ADD:
			if (teamSize < MAX_TEAM_SIZE)
			{
				spawnTank(0,teamSize);
				amTankState[teamSize] = 1;
				spawnTank(1,teamSize);
				bmTankState[teamSize] = 1;
				teamSize++;
			}
			break;
		case OIS::KC_SUBTRACT:
			if (teamSize > 1)
			{
				teamSize--;
				amTankHealth[teamSize] = 0.0;
				amTankBodyNode[teamSize]->setPosition(-1350, -50, (100 * teamSize));
				amTankState[teamSize] = 0;
				apath[teamSize]->clear();
				agoalNode[teamSize] = astartNode[teamSize] = -1;

				bmTankHealth[teamSize] = 0.0;
				bmTankBodyNode[teamSize]->setPosition(1250, -50, (100 * teamSize));
				bmTankState[teamSize] = 0;
				bpath[teamSize]->clear();
				bgoalNode[teamSize] = bstartNode[teamSize] = -1;

			}
			break;
		case OIS::KC_SPACE:
				pathTank(0,0,randDest);
			break;
		default:
			break;
	}

	return true;
}

void Ass3::goPath(Ogre::ManualObject* line, float height, std::vector<int>& path, Ogre::ColourValue& colour, int team, int tank)
{
	line->clear();
	line->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_STRIP);
	
	position = Ogre::Vector3(0, 0, 0);
	for(std::vector<int>::iterator it=path.begin(); it!=path.end(); it++)
	{
		
		position = pathFindingGraph->getPosition(*it);
		Ogre::Vector3 temp(position.x, 18, position.z);
		if (team == 0)
			amWalkList[tank].push_back(temp);
		else
			bmWalkList[tank].push_back(temp);
	}

	// Finished defining line
	line->end();
}

bool Ass3::nextLocation(int team, int tank)
{
	if (team == 0)
	{
		if(amWalkList[tank].empty())
			return false;
		amDestination[tank] = amWalkList[tank].front();
		amWalkList[tank].pop_front();
		amDirect[tank] = amDestination[tank] - amTankBodyNode[tank]->getPosition();
		amDistance[tank] = amDirect[tank].normalise();
		return true;
	}
	else
	{
		if(bmWalkList[tank].empty())
			return false;
		bmDestination[tank] = bmWalkList[tank].front();
		bmWalkList[tank].pop_front();
		bmDirect[tank] = bmDestination[tank] - bmTankBodyNode[tank]->getPosition();
		bmDistance[tank] = bmDirect[tank].normalise();
		return true;
	}
}

//initial creation
void Ass3::createTank(int team, int tank) 
{
	if (team == 0) //Team A
	{
		std::string bodyName = "chbodyA";
		//char temp[3]
		//itoa(i, temp, 10)
		//entityName += temp;
		bodyName += std::to_string(tank);
		amTankBody[tank] = mSceneMgr->createEntity(bodyName, "chbody.mesh");
		amTankBody[tank]->setCastShadows(true);
		amTankBody[tank]->setMaterialName("ch_tank_material");

		// Create tank turret entity
		std::string turretName = "chturretA";
		turretName += std::to_string(tank);
		amTankTurret[tank] = mSceneMgr->createEntity(turretName, "chturret.mesh");
		amTankTurret[tank]->setCastShadows(true);
		amTankTurret[tank]->setMaterialName("ch_tank_material");

		// Create tank barrel entity
		std::string barrelName = "chbarrelA";
		barrelName += std::to_string(tank);
		amTankBarrel[tank] = mSceneMgr->createEntity(barrelName, "chbarrel.mesh");
		amTankBarrel[tank]->setCastShadows(true);
		amTankBarrel[tank]->setMaterialName("ch_tank_material");

		// Create a child scene node and attach tank body to it
		amTankBodyNode[tank] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		Ogre::SceneNode * temp = amTankBodyNode[tank]->createChildSceneNode();
		temp->attachObject(amTankBody[tank]);
		// spawn it below the ground initially
		amTankBodyNode[tank]->translate(-1350, -50, (100 * tank));

		// Create a child scene node from tank body's scene node and attach the tank turret to it
		amTankTurretNode[tank] = temp->createChildSceneNode();
		amTankTurretNode[tank]->attachObject(amTankTurret[tank]);
		// Move it above tank body
		amTankTurretNode[tank]->translate(0, 3, 0);

		// Create a child scene node from tank turret's scene node and attach the tank barrel to it
		amTankBarrelNode[tank] = amTankTurretNode[tank]->createChildSceneNode();
		amTankBarrelNode[tank] ->attachObject(amTankBarrel[tank]);
		// Move it to the appropriate position on the turret
		amTankBarrelNode[tank] ->translate(-30, 10, 0);

		temp->rotate(Ogre::Quaternion(0,0,1,0));

		amTankState[tank] = 0; //initialise to dead state
	}
	else //team B
	{
		std::string bodyName = "chbodyB";
		//char temp[3]
		//itoa(i, temp, 10)
		//entityName += temp;
		bodyName += std::to_string(tank);
		bmTankBody[tank] = mSceneMgr->createEntity(bodyName, "chbody.mesh");
		bmTankBody[tank]->setCastShadows(true);
		bmTankBody[tank]->setMaterialName("ch_tank_material");

		// Create tank turret entity
		std::string turretName = "chturretB";
		turretName += std::to_string(tank);
		bmTankTurret[tank] = mSceneMgr->createEntity(turretName, "chturret.mesh");
		bmTankTurret[tank]->setCastShadows(true);
		bmTankTurret[tank]->setMaterialName("ch_tank_material");

		// Create tank barrel entity
		std::string barrelName = "chbarrelB";
		barrelName += std::to_string(tank);
		bmTankBarrel[tank] = mSceneMgr->createEntity(barrelName, "chbarrel.mesh");
		bmTankBarrel[tank]->setCastShadows(true);
		bmTankBarrel[tank]->setMaterialName("ch_tank_material");

		// Create a child scene node and attach tank body to it
		bmTankBodyNode[tank] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		Ogre::SceneNode * temp = bmTankBodyNode[tank]->createChildSceneNode();
		temp->attachObject(bmTankBody[tank]);
		// spawn it below the ground initially
		bmTankBodyNode[tank]->translate(1250, -50, (100 * tank));

		// Create a child scene node from tank body's scene node and attach the tank turret to it
		bmTankTurretNode[tank] = temp->createChildSceneNode();
		bmTankTurretNode[tank]->attachObject(bmTankTurret[tank]);
		// Move it above tank body
		bmTankTurretNode[tank]->translate(0, 3, 0);

		// Create a child scene node from tank turret's scene node and attach the tank barrel to it
		bmTankBarrelNode[tank] = bmTankTurretNode[tank]->createChildSceneNode();
		bmTankBarrelNode[tank] ->attachObject(bmTankBarrel[tank]);
		// Move it to the appropriate position on the turret
		bmTankBarrelNode[tank] ->translate(-30, 10, 0);

		temp->rotate(Ogre::Quaternion(0,0,1,0));

		bmTankState[tank] = 0; //initialise to dead state
	}
}

//spawning / respawning
void Ass3::spawnTank(int team, int tank)
{
	if (team == 0) //Team A
	{
		//get entity name
		//std::string entName = "chbodyA";
		//entName += std::to_string(tank);

		//get a random spawn point for team A
		int x = (rand() % 100) - 1350;
		int z = rand() % 2500;
		if (z > 1250) { z -= 2500; }

		amTankHealth[tank] = 1.0;
		amTankBodyNode[tank]->setPosition(x,18,z);
		amTankState[tank] = 1; //set to idle state
	}
	else //Team B
	{
		//get a random spawn point for team B
		int x = (rand() % 100) + 1250;
		int z = rand() % 2500;
		if (z > 1250) { z -= 2500; }

		bmTankHealth[tank] = 1.0;
		bmTankBodyNode[tank]->setPosition(x,18,z);
		bmTankState[tank] = 1; //set to idle state
	}
}

void Ass3::pathTank(int team, int tank, Ogre::Vector3 destination)
{
	//clear previous path if exists
	// set start node to tanks current position
	if(team == 0) //team A
	{
		apath[tank]->clear();
		astartNode[tank] = pathFindingGraph->getNode(amTankBodyNode[tank]->getPosition());

		// set goal node
		agoalNode[tank] = pathFindingGraph->getNode(destination);

		// check that goal node is not the same as start node
		if(agoalNode[tank] != astartNode[tank])
		{
			// try to find path from start to goal node
			std::vector<int> path;

			// if path exists
			if(mPathFinder.AStar(astartNode[tank], agoalNode[tank], *pathFindingGraph, path))
			{
				// draw path
				goPath(apath[tank], 0.5, path, Ogre::ColourValue(1, 0, 0), team, tank);
			}
		}
	}
	else //team B
	{
		bpath[tank]->clear();
		bstartNode[tank] = pathFindingGraph->getNode(bmTankBodyNode[tank]->getPosition());

		// set goal node
		bgoalNode[tank] = pathFindingGraph->getNode(destination);

		// check that goal node is not the same as start node
		if(bgoalNode[tank] != bstartNode[tank])
		{
			// try to find path from start to goal node
			std::vector<int> path;

			// if path exists
			if(mPathFinder.AStar(bstartNode[tank], bgoalNode[tank], *pathFindingGraph, path))
			{
				// draw path
				goPath(bpath[tank], 0.5, path, Ogre::ColourValue(1, 0, 0), team, tank);
			}
		}
	}

	
}

void Ass3::moveTank(int team, int tank, Ogre::Real tslf)
{
	if (team == 0) //team A
	{
		if(amDirect[tank] == Ogre::Vector3::ZERO)
		{
			if(!nextLocation(0,tank))
			{
				amTankState[tank] = 1;
			}
		}
		else
		{
			Ogre::Real move = mWalkSpd * tslf;
			amDistance[tank] -= move;

			if(amDistance[tank] < 0.0)
			{
				amTankBodyNode[tank]->setPosition(amDestination[tank]);
				amDirect[tank] = Ogre::Vector3::ZERO;

				if(nextLocation(0,tank))
				{
					Ogre::Vector3 src = amTankBodyNode[tank]->getOrientation() * Ogre::Vector3::UNIT_X;
					if((1.0 + src.dotProduct(amDirect[tank])) < 0.0001)
					{
						//NOTE commenting this out makes tanks reverse?
						//amTankBodyNode[tank]->yaw(Ogre::Degree(180)); 
					}
					else
					{
						Ogre::Quaternion quat = src.getRotationTo(amDirect[tank]);
						amTankBodyNode[tank]->rotate(quat);
					}
				}
				else
				{
					amTankState[tank] = 1;
				}
			}
			else
			{
				amTankBodyNode[tank]->translate(move * amDirect[tank]);
			}
		}
		if (amTankState[tank] == 1)
		{
			amTankState[tank] = 2;
			int x = rand() % 1000;
			int z = rand() % 2500;
			if (x > 500)  { x -= 1000; }
			if (z > 1250) { z -= 2500; }
			Ogre::Vector3 randDest = Ogre::Vector3(x,18,z);
			pathTank(team,tank,randDest);
		}
	}
	else //team B
	{
		if(bmDirect[tank] == Ogre::Vector3::ZERO)
		{
			if(!nextLocation(1,tank))
			{
				bmTankState[tank] = 1;
			}
		}
		else
		{
			Ogre::Real move = mWalkSpd * tslf;
			bmDistance[tank] -= move;

			if(bmDistance[tank] < 0.0)
			{
				bmTankBodyNode[tank]->setPosition(bmDestination[tank]);
				bmDirect[tank] = Ogre::Vector3::ZERO;

				if(nextLocation(1,tank))
				{
					Ogre::Vector3 src = bmTankBodyNode[tank]->getOrientation() * Ogre::Vector3::UNIT_X;
					if((1.0 + src.dotProduct(bmDirect[tank])) < 0.0001)
					{
						//NOTE commenting this out makes tanks reverse?
						//bmTankBodyNode[tank]->yaw(Ogre::Degree(180)); 
					}
					else
					{
						Ogre::Quaternion quat = src.getRotationTo(bmDirect[tank]);
						bmTankBodyNode[tank]->rotate(quat);
					}
				}
				else
				{
					bmTankState[tank] = 1;
				}
			}
			else
			{
				bmTankBodyNode[tank]->translate(move * bmDirect[tank]);
			}
		}


		if (bmTankState[tank] == 1)
		{
			bmTankState[tank] = 2;
			int x = rand() % 1000;
			int z = rand() % 2500;
			if (x > 500)  { x -= 1000; }
			if (z > 1250) { z -= 2500; }
			Ogre::Vector3 randDest = Ogre::Vector3(x,18,z);
			pathTank(team,tank,randDest);
		}
	}
}


#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif
 
#ifdef __cplusplus
extern "C" {
#endif
 
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
    int main(int argc, char *argv[])
#endif
    {
        // Create application object
        Ass3 app;
 
        try {
            app.go();
        } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
            std::cerr << "An exception has occured: " <<
                e.getFullDescription().c_str() << std::endl;
#endif
        }
 
        return 0;
    }
 
#ifdef __cplusplus
}
#endif