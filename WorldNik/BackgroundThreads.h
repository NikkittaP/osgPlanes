#ifndef BackgroundThreads_HEADER
#define BackgroundThreads_HEADER

#include "stdafx.h"

class DrawFlightInfoThread: public OpenThreads::Thread
{
public:
	DrawFlightInfoThread(int _sel) : OpenThreads::Thread(), _id(_sel) {}
	virtual void run();
protected:
	OpenThreads::Mutex _mutex;
	int _id;
};

class UpdatePlanesInTheSkyThread : public OpenThreads::Thread
{
public:
	UpdatePlanesInTheSkyThread(double _timestamp) : OpenThreads::Thread(), timestamp(_timestamp) {}
	virtual void run();
protected:
	int timestamp;
};

#endif