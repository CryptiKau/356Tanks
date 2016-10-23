#ifndef __Connection_h_
#define __Connection_h_
#include "stdafx.h"
class Connection
{
private:
	float	mCost;
	int		mToNode;
	int		mFromNode;

public:
	Connection();
	~Connection();

	void set(float c, int to, int from);
	void clear();

	float getCost();
	int getFromNode();
	int getToNode();
};

#endif