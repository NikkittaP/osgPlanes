#include "stdafx.h"
#include "Variables.h"
#include "NodeVisitors.h"

CcolorVisitor blueColor, redColor, whiteColor;
void InitColors()
{
	whiteColor.setColor(osg::Vec4(1, 1, 1, 1));
	blueColor.setColor(osg::Vec4(0, 0, 1, 1));
	redColor.setColor(osg::Vec4(1, 0, 0, 1));
}