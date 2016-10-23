#include "stdafx.h"
#include "Ass3.h"
 
//-------------------------------------------------------------------------------------
Ass3::Ass3(void):
	mDistance(0),
	mWalkSpd(200.0),
	mDirect(Ogre::Vector3::ZERO),
	mDestination(Ogre::Vector3::ZERO)
{

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

	mCurrentRobot = -1;

	pathFindingGraph = new Graph;
	mCurrentState = 0;

	mDirection = Ogre::Vector3::ZERO;

	return true;
};

void Ass3::createCamera(void)
{
	// Create the camera
	mCamera = mSceneMgr->createCamera("PlayerCam");
 
	// Position it at 500 in Z direction
	mCamera->setPosition(Ogre::Vector3(0,0,40));
	// Look back along -Z
	//mCamera->lookAt(Ogre::Vector3(0,0,-300));
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
	mRobot[0] = mSceneMgr->createEntity("chbody1", "chbody.mesh");
	mRobot[0]->setCastShadows(true);
	mRobot[0]->setMaterialName("ch_tank_material");
	mRobot[1] = mSceneMgr->createEntity("Robot2", "robot.mesh");
	mRobot[1]->setCastShadows(true);


	// Attach robots to scene nodes and position them appropriately
	mRobotNode[0] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	mRobotNode[0]->attachObject(mRobot[0]);
	mRobotNode[0]->yaw(Ogre::Degree(-90));
	mRobotNode[0]->translate(-100, 18, 0);

	mRobotNode[1] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	mRobotNode[1]->attachObject(mRobot[1]);
	mRobotNode[1]->yaw(Ogre::Degree(-90));


	// Initialise the robot's health
	mRobotHealth[0] = 0.2;
	mRobotHealth[1] = 0.5;

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
			position.y = 24.5;
			myNode->translate(position);
		}
	}

	// create the path objects, and clear them to start off
	path1 = mSceneMgr->createManualObject("DijkstraPath");
	path1->clear();
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(path1);

	path2 = mSceneMgr->createManualObject("AStarPath");
	path2->clear();
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(path2);



}

bool Ass3::processUnbufferedInput(const Ogre::FrameEvent& evt)
{



	Ogre::Vector3 transVector = Ogre::Vector3::ZERO;
	mMoveScale = 250;
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


	if(mDirect == Ogre::Vector3::ZERO)
	{
		if(nextLocation())
		{
		}

	}
	else
	{
		Ogre::Real move = mWalkSpd * evt.timeSinceLastFrame;
		mDistance -= move;

		if(mDistance <= 0.0)
		{
			mRobotNode[mCurrentRobot]->setPosition(mDestination);
			mDirect = Ogre::Vector3::ZERO;

			if(nextLocation())
			{
				Ogre::Vector3 src = mRobotNode[mCurrentRobot]->getOrientation() * Ogre::Vector3::UNIT_X;
				if((1.0 + src.dotProduct(mDirect)) < 0.0001)
				{
					mRobotNode[mCurrentRobot]->yaw(Ogre::Degree(180));
				}
				else
				{
					Ogre::Quaternion quat = src.getRotationTo(mDirect);
					mRobotNode[mCurrentRobot]->rotate(quat);
				}
			}
			else
			{
				//idle
			}
		}
		else
		{
			mRobotNode[mCurrentRobot]->translate(move * mDirect);
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
			case OIS::MB_Middle:
			{
				// if path already exists
				if(mCurrentState > 1)
				{
					// reset
					mCurrentState = 0;
					path1->clear();
					path2->clear();
				
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
									if(mPathFinder.Dijkstra(startNode, goalNode, *pathFindingGraph, path))
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
		case OIS::MB_Left:
			{

				mCurrentState = 0;
				path1->clear();
				path2->clear();

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

					// Store previously selected robot
					int previous = mCurrentRobot;

					// Get name of movable object that was hit
					Ogre::String name = itr->movable->getName();

					// Check name with selectable entity names and set selected robot
					if(name == "Robot1")
						mCurrentRobot = 0;
					else if(name == "Robot2")
						mCurrentRobot = 1;
					else if(name == "Robot3")
						mCurrentRobot = 2;
					else
						mCurrentRobot = -1;

					// If there was a previously selected robot
					if(previous != -1)
					{
						// Detach the healthbar and selection circle
						mRobotNode[previous]->detachObject(mHealthBar);
						mRobotNode[previous]->detachObject(mSelectionCircle);
					}

					// If there is a selected robot
					if(mCurrentRobot != -1)
					{
						// Calculate the health bar adjustments
						float healthBarAdjuster = (1.0 - mRobotHealth[mCurrentRobot])/2;	// This must range from 0.0 to 0.5
						// Set the health bar to the appropriate level
						mHealthBarBB->setTexcoordRect(0.0 + healthBarAdjuster, 0.0, 0.5 + healthBarAdjuster, 1.0);

						// Attach the healthbar and selection circle
						mRobotNode[mCurrentRobot]->attachObject(mHealthBar);
						mRobotNode[mCurrentRobot]->attachObject(mSelectionCircle);

						Ogre::Vector3 location = mouseRay.getPoint(itr->distance);
					////////////////////////////
						if(mCurrentState == 0)
						{
							// set start node
							startNode = pathFindingGraph->getNode(location);
							// set state to goal node state
							mCurrentState++;
						}

					}
				}
				// If nothing selected
				else
				{
					// Nothing selected and there is a previously selected robot
					if(mCurrentRobot != -1)
					{
						// Detach the healthbar and selection circle
						mRobotNode[mCurrentRobot]->detachObject(mHealthBar);
						mRobotNode[mCurrentRobot]->detachObject(mSelectionCircle);

						// Set to no robot selected
						mCurrentRobot = -1;
					}
				}
			}
			break;

		default:
			break;
	}

	return true;
}

void Ass3::goPath(Ogre::ManualObject* line, float height, std::vector<int>& path, Ogre::ColourValue& colour)
{
	line->clear();


	// Specify the material and rendering type
	line->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_STRIP);
	
	// Specify the vertices and vertex colour for the line
	position = Ogre::Vector3(0, 0, 0);
	for(std::vector<int>::iterator it=path.begin(); it!=path.end(); it++)
	{
		
		position = pathFindingGraph->getPosition(*it);
		//line->position(Ogre::Vector3(position.x, height, position.z));
		//line->colour(colour);
		//mRobotNode[mCurrentRobot]->translate(mRobotNode[mCurrentRobot]->getOrientation() * Ogre::Vector3(position.x, 0, position.z));
		//mRobotNode[mCurrentRobot]->setPosition(Ogre::Vector3(position.x, 0, position.z));
		Ogre::Vector3 temp(position.x, 0, position.z);
		mWalkList.push_back(temp);
	}


	// Finished defining line
	line->end();
}

bool Ass3::nextLocation()
{
	if(mWalkList.empty())
		return false;
	mDestination = mWalkList.front();
	mWalkList.pop_front();
	mDirect = mDestination - mRobotNode[mCurrentRobot]->getPosition();
	mDistance = mDirect.normalise();
	return true;
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