#ifndef __Ass3_h_
#define __Ass3_h_
 
#include "stdafx.h"
#include "BaseApplication.h"
#include "PathFinding.h"

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
	/////
 
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
	Ogre::Entity* mRobot[3];
	Ogre::SceneNode* mRobotNode[3];
	float mRobotHealth[3];
	int mCurrentRobot;

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
	Ogre::ManualObject* path2;

	void createPath(Ogre::ManualObject* line, float height, std::vector<int>& path, Ogre::ColourValue& colour);
	void goPath(Ogre::ManualObject* line, float height, std::vector<int>& path, Ogre::ColourValue& colour);

};
 
#endif // #ifndef __Ass3_h_