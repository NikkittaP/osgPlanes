#include "stdafx.h"
#include "Variables.h"
#include "Helpers.h"

std::string ZeroPadNumber(int num)
{
	std::ostringstream ss;
	ss << std::setw(4) << std::setfill('0') << num;
	return ss.str();
}

void getScreenSize()
{
	osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
	if (!wsi)
	{
		osg::notify(osg::NOTICE) << "Error, no WindowSystemInterface available, cannot create windows." << std::endl;
		return;
	}

	wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(SCREEN_NUM), screenWidth, screenHeight);
	std::cout << screenWidth << "x" << screenHeight << std::endl;
}

osg::Geode* DrawSimplePlane()
{
	osg::Geode* plane = new osg::Geode();

	osg::Geometry* pyramidGeometry = new osg::Geometry();
	osg::Vec3Array* pyramidVertices = new osg::Vec3Array;
	pyramidVertices->push_back(osg::Vec3(0, 0, 0)); // front left
	pyramidVertices->push_back(osg::Vec3(10, 0, 0)); // front right
	pyramidVertices->push_back(osg::Vec3(10, 10, 0)); // back right
	pyramidVertices->push_back(osg::Vec3(0, 10, 0)); // back left
	pyramidVertices->push_back(osg::Vec3(5, 5, 10)); // peak

	pyramidGeometry->setVertexArray(pyramidVertices);

	osg::DrawElementsUInt* pyramidBase =
		new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
	pyramidBase->push_back(3);
	pyramidBase->push_back(2);
	pyramidBase->push_back(1);
	pyramidBase->push_back(0);
	pyramidGeometry->addPrimitiveSet(pyramidBase);

	osg::DrawElementsUInt* pyramidFaceOne =
		new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
	pyramidFaceOne->push_back(0);
	pyramidFaceOne->push_back(1);
	pyramidFaceOne->push_back(4);
	pyramidGeometry->addPrimitiveSet(pyramidFaceOne);

	osg::DrawElementsUInt* pyramidFaceTwo =
		new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
	pyramidFaceTwo->push_back(1);
	pyramidFaceTwo->push_back(2);
	pyramidFaceTwo->push_back(4);
	pyramidGeometry->addPrimitiveSet(pyramidFaceTwo);

	osg::DrawElementsUInt* pyramidFaceThree =
		new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
	pyramidFaceThree->push_back(2);
	pyramidFaceThree->push_back(3);
	pyramidFaceThree->push_back(4);
	pyramidGeometry->addPrimitiveSet(pyramidFaceThree);

	osg::DrawElementsUInt* pyramidFaceFour =
		new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
	pyramidFaceFour->push_back(3);
	pyramidFaceFour->push_back(0);
	pyramidFaceFour->push_back(4);
	pyramidGeometry->addPrimitiveSet(pyramidFaceFour);

	plane->addDrawable(pyramidGeometry);

	return plane;
}