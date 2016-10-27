#ifndef __Ass3_h_
#define __Ass3_h_
 
#include "stdafx.h"
#include "BaseApplication.h"
#include "PathFinding.h"

//adjustable max team size
#define MAX_TEAM_SIZE 10

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

	std::deque<Ogre::Vector3> amWalkList[MAX_TEAM_SIZE];
	std::deque<Ogre::Vector3> bmWalkList[MAX_TEAM_SIZE];

	Ogre::Real mWalkSpd;
	Ogre::Real amDistance[MAX_TEAM_SIZE];
	Ogre::Vector3 amDirect[MAX_TEAM_SIZE];
	Ogre::Vector3 amDestination[MAX_TEAM_SIZE];
	Ogre::Real bmDistance[MAX_TEAM_SIZE];
	Ogre::Vector3 bmDirect[MAX_TEAM_SIZE];
	Ogre::Vector3 bmDestination[MAX_TEAM_SIZE];

	bool nextLocation(int team, int tank);
 
protected:
	virtual bool setup();
    virtual void createScene(void);
	virtual void createCamera(void);
	virtual bool processUnbufferedInput(const Ogre::FrameEvent& evt);
	virtual bool frameRenderingQueued(const Ogre::FrameEvent& fe);
    virtual bool mouseMoved( const OIS::MouseEvent &arg );
    virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
	virtual bool keyPressed( const OIS::KeyEvent &arg );
	//virtual bool keyReleased( const OIS::KeyEvent &arg );

	Ogre::Real mRotate;          // The rotate constant
	Ogre::Real mMove;            // The movement constant
	Ogre::Vector3 mDirection;     // Value to move in the correct direction

private:
	//Ogre::Entity* mRobot[3];
	//Ogre::SceneNode* mRobotNode[3];
	//float mRobotHealth[3];
	//int mCurrentRobot;

	//current team size
	int teamSize;

	//Team A
	// Scene nodes for the different tank parts
	Ogre::Entity* amTankBody[MAX_TEAM_SIZE];
	Ogre::Entity* amTankTurret[MAX_TEAM_SIZE];
	Ogre::Entity* amTankBarrel[MAX_TEAM_SIZE];
	Ogre::SceneNode* amTankBodyNode[MAX_TEAM_SIZE];
	Ogre::SceneNode* amTankTurretNode[MAX_TEAM_SIZE];
	Ogre::SceneNode* amTankBarrelNode[MAX_TEAM_SIZE];
	float amTankHealth[MAX_TEAM_SIZE];
	// For tank movement and rotation
	float amMove[MAX_TEAM_SIZE];
	float amBodyRotate[MAX_TEAM_SIZE];
	float amTurretRotate[MAX_TEAM_SIZE];
	float amBarrelRotate[MAX_TEAM_SIZE];
	float amBarrelPitch[MAX_TEAM_SIZE];
	//0=dead, 1=idle, 2=moving, 3=shooting
	int amTankState[MAX_TEAM_SIZE];

	//Team B
	// Scene nodes for the different tank parts
	Ogre::Entity* bmTankBody[MAX_TEAM_SIZE];
	Ogre::Entity* bmTankTurret[MAX_TEAM_SIZE];
	Ogre::Entity* bmTankBarrel[MAX_TEAM_SIZE];
	Ogre::SceneNode* bmTankBodyNode[MAX_TEAM_SIZE];
	Ogre::SceneNode* bmTankTurretNode[MAX_TEAM_SIZE];
	Ogre::SceneNode* bmTankBarrelNode[MAX_TEAM_SIZE];
	float bmTankHealth[MAX_TEAM_SIZE];
	// For tank movement and rotation
	float bmMove[MAX_TEAM_SIZE];
	float bmBodyRotate[MAX_TEAM_SIZE];
	float bmTurretRotate[MAX_TEAM_SIZE];
	float bmBarrelRotate[MAX_TEAM_SIZE];
	float bmBarrelPitch[MAX_TEAM_SIZE];
	//0=dead, 1=idle, 2=moving, 3=shooting
	int bmTankState[MAX_TEAM_SIZE];

	//for selection / health and state display
	int mCurrentTank;
	Ogre::BillboardSet* mHealthBar;
	Ogre::Billboard* mHealthBarBB;
	Ogre::BillboardSet* mSelectionCircle;
	Ogre::Billboard* mSelectionCircleBB;

	//pathfinding
	//int mCurrentState;
	int astartNode[MAX_TEAM_SIZE];
	int agoalNode[MAX_TEAM_SIZE];
	int bstartNode[MAX_TEAM_SIZE];
	int bgoalNode[MAX_TEAM_SIZE];
	Graph* pathFindingGraph;
	PathFinding mPathFinder;
	Ogre::ManualObject* apath[MAX_TEAM_SIZE];
	Ogre::ManualObject* bpath[MAX_TEAM_SIZE];
	void createPath(Ogre::ManualObject* line, float height, std::vector<int>& path, Ogre::ColourValue& colour);
	void goPath(Ogre::ManualObject* line, float height, std::vector<int>& path, Ogre::ColourValue& colour, int team, int tank);

	//tank creation / spawning
	void Ass3::createTank(int team, int tank);
	void Ass3::spawnTank(int team, int tank);
	void Ass3::pathTank(int team, int tank, Ogre::Vector3 destination);
	void Ass3::moveTank(int team, int tank, Ogre::Real tslf);
};
 
#endif // #ifndef __Ass3_h_