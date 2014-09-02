#ifndef BackgroundThreads_HEADER
#define BackgroundThreads_HEADER

#include "stdafx.h"

class DrawFlightInfoThread: public OpenThreads::Thread
{
public:
	DrawFlightInfoThread(int _sel) : OpenThreads::Thread(), _id(_sel) {}
	virtual void run();
protected:
	int _id;
};

#endif