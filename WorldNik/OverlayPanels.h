#ifndef OverlayPanels_HEADER
#define OverlayPanels_HEADER

osg::Node* createOverlayPanels(osgViewer::View* view);
void createPlaneInfoPanel(int id);
void updatePlaneInfoPanel(int id);
#endif