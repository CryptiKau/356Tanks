#ifndef __Ass3_h_
#define __Ass3_h_
 
#include "stdafx.h"
#include "BaseApplication.h"
#include "PathFinding.h"

//adjustable team size
#define TEAM_SIZE 4

#include <fstream>
#include <string>

class Ass3 : public BaseApplication
{
public:
    Ass3(void);
    virtual ~Ass3(void);
	int mMoveScale;
	int mZoomScale;
	Ogre::Vector3 position;
	Ogre::Vector3 posMoves[30];
	int globalCheck;

	//////
	std::deque<Ogre::Vector3> mWalkList;


	Ogre::Real mDistance;
	Ogre::Real mWalkSpd;
	Ogre::Vector3 mDirect;
	Ogre::Vector3 mDestination;

	bool nextLocation();
 
protected:
	virtual bool setup();
    virtual void createScene(void);
	virtual void createCamera(void);

	virtual bool processUnbufferedInput(const Ogre::FrameEvent& evt);
	virtual bool frameRenderingQueued(const Ogre::FrameEvent& fe);
    // OIS::MouseListener
    virtual bool mouseMoved( const OIS::MouseEvent &arg );
    virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

	Ogre::Real mRotate;          // The rotate constant
	Ogre::Real mMove;            // The movement constant
 
	Ogre::Vector3 mDirection;     // Value to move in the correct direction

private:

	
	//Ogre::Entity* mRobot[3];
	//Ogre::SceneNode* mRobotNode[3];
	//float mRobotHealth[3];
	//int mCurrentRobot;

	//Team A
	// Scene nodes for the different tank parts
	Ogre::Entity* amTankBody[TEAM_SIZE];
	Ogre::Entity* amTankTurret[TEAM_SIZE];
	Ogre::Entity* amTankBarrel[TEAM_SIZE];
	Ogre::SceneNode* amTankBodyNode[TEAM_SIZE];
	Ogre::SceneNode* amTankTurretNode[TEAM_SIZE];
	Ogre::SceneNode* amTankBarrelNode[TEAM_SIZE];
	float amTankHealth[TEAM_SIZE];
	// For tank movement and rotation
	float amMove[TEAM_SIZE];
	float amBodyRotate[TEAM_SIZE];
	float amTurretRotate[TEAM_SIZE];
	float amBarrelRotate[TEAM_SIZE];
	float amBarrelPitch[TEAM_SIZE];

	//Team B
	// Scene nodes for the different tank parts
	Ogre::Entity* bmTankBody[TEAM_SIZE];
	Ogre::Entity* bmTankTurret[TEAM_SIZE];
	Ogre::Entity* bmTankBarrel[TEAM_SIZE];
	Ogre::SceneNode* bmTankBodyNode[TEAM_SIZE];
	Ogre::SceneNode* bmTankTurretNode[TEAM_SIZE];
	Ogre::SceneNode* bmTankBarrelNode[TEAM_SIZE];
	float bmTankHealth[TEAM_SIZE];
	// For tank movement and rotation
	float bmMove[TEAM_SIZE];
	float bmBodyRotate[TEAM_SIZE];
	float bmTurretRotate[TEAM_SIZE];
	float bmBarrelRotate[TEAM_SIZE];
	float bmBarrelPitch[TEAM_SIZE];

	int mCurrentTank;

	Ogre::BillboardSet* mHealthBar;
	Ogre::Billboard* mHealthBarBB;

	Ogre::BillboardSet* mSelectionCircle;
	Ogre::Billboard* mSelectionCircleBB;

	int mCurrentState;
	int startNode;
	int goalNode;

	Graph* pathFindingGraph;
	PathFinding mPathFinder;
	Ogre::ManualObject* path1;

	void createPath(Ogre::ManualObject* line, float height, std::vector<int>& path, Ogre::ColourValue& colour);
	void goPath(Ogre::ManualObject* line, float height, std::vector<int>& path, Ogre::ColourValue& colour);

};
 
#endif // #ifndef __Ass3_h_