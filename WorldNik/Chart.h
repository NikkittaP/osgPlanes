#ifndef Chart_HEADER
#define Chart_HEADER

enum hudPositionType
{
	CENTER_BOTTOM = 0
};

void ShowVerticalProfile(int id);

void normalizeData(int hudWidth, int hudHeight);

osg::Geometry* getTerrainElevation(int windowWidth, int windowHeight, int hudWidth, int hudHeight, hudPositionType hudPosition, osg::Vec4 color);

osg::Geometry* getWholePath(int windowWidth, int windowHeight, int hudWidth, int hudHeight, hudPositionType hudPosition, osg::Vec4 color);

osg::Geode* getTicks(int windowWidth, int windowHeight, int hudWidth, int hudHeight, hudPositionType hudPosition, osg::Vec4 color);

osg::Camera* createHUD(int windowWidth, int windowHeight, int hudWidth, int hudHeight, hudPositionType hudPosition);

#endif