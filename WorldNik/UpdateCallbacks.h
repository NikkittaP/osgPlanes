#ifndef UpdateCallbacks_HEADER
#define UpdateCallbacks_HEADER

#include "stdafx.h"

class MovePlanes : public osg::NodeCallback
{
public:
	MovePlanes() :_prev_clock(clock()), _time(currentDateTime.asTimeStamp()){}
	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
protected:
	double _prev_clock;
	double _time;
};

class MoveSun : public osg::NodeCallback
{
public:
	MoveSun() :_prev_clock(clock()){}
	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
protected:
	double _prev_clock;
};

class UpdatePlanesInTheSky : public osg::NodeCallback
{
public:
	UpdatePlanesInTheSky() :_prev_clock(clock()), _prev_sun_clock(clock()){}
	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
protected:
	double _prev_clock, _prev_sun_clock;
};

#endif