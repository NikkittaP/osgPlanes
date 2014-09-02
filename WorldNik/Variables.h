#ifndef Variables_HEADER
#define Variables_HEADER

#include "stdafx.h"
#include "NodeVisitors.h"
#include "BackgroundThreads.h"

const int SCREEN_NUM = 0;
extern int STEPS_PER_SECOND;
extern std::string PATH;
extern unsigned int screenWidth, screenHeight, hudChartWidth, hudChartHeight;

extern int numPlanes;
/*
extern std::vector<std::vector<float>> seconds;
extern std::vector<std::vector<float>> lat;
extern std::vector<std::vector<float>> lon;
extern std::vector<std::vector<float>> alt;
extern std::vector<std::vector<float>> psi;
extern std::vector<std::vector<float>> tetta;
extern std::vector<std::vector<float>> gamma;
*/
//extern std::vector<float> planeT, planeLat, planeLon, planeAlt, planeTheta, planeGamma, planePsi;
extern const osgEarth::Util::SpatialReference* mapSRS;
extern const osgEarth::Util::SpatialReference* geoSRS;

extern osgEarth::Annotation::FeatureNode* pathNode;
extern osgEarth::Annotation::FeatureNode* pathNode_surface;
extern osg::ref_ptr<osg::Group> connections;

extern osg::ref_ptr<osgEarth::MapNode> mapNode;
extern osg::ref_ptr<osg::Group> planesGroup;
extern osg::ref_ptr<osg::Group> planesNamesGroup;
extern osg::ref_ptr<osg::Group> root;
extern std::vector<osg::ref_ptr<osgEarth::Annotation::ModelNode>> planesOnEarth;
extern osgEarth::Util::SkyNode* sky;
extern osgEarth::DateTime currentDateTime;
extern osgEarth::Util::Controls::LabelControl* currentDateTimeLabel;

extern osgViewer::Viewer viewer;
extern osg::ref_ptr<osgEarth::Util::EarthManipulator> e_manip;
extern osg::Camera* verticalProfileHUD;

extern bool isFollowingPlane;
extern double prev_distance_to_earth;
extern int scale;

extern double _speed;
//extern int _planeIndex;
extern int _selectedPlane;
extern clock_t start, end;

extern std::vector<float> _time, timeNorm;
extern std::vector<float> lat_whole, lon_whole;
extern std::vector<float> H_whole, H_wholeNorm;
extern std::vector<float> HNorm;
extern double _chartMaxHeight, _chartMaxTime;
extern double XunitsInPixel, YunitsInPixel;
extern osg::Sphere* planeAltitudeSphere;
extern osgText::Text* planeAltitudeText;
extern osg::ref_ptr<osg::ShapeDrawable> chartPlanePosition;
extern bool isShowingChart;

extern osg::Geode* chartGeode;

extern osgEarth::Util::Controls::ControlCanvas* canvasOverlay;
extern osgEarth::Util::Controls::Grid* commonControlsGrid;
extern osgEarth::Util::Controls::Grid* messagesGrid;
extern osgEarth::Util::Controls::Grid* planeInfoGrid;
extern osgEarth::Util::Controls::LabelControl* speedControlLabel;
extern osgEarth::Util::Controls::HSliderControl* speedControl;
extern osgEarth::Util::Controls::CheckBoxControl* speedControlCheckBox_100;
extern osgEarth::Util::Controls::CheckBoxControl* speedControlCheckBox_1000;

extern CcolorVisitor whiteColor;
extern CcolorVisitor blueColor;
extern CcolorVisitor redColor;

extern DrawFlightInfoThread *drawFlightInfoThread;

struct AirportData {
	QString IATAcode;
	QString ICAOcode;
	QString name;
	QString city;
	QString country;
};

struct FlightPoint {
	double seconds;
	double lat;
	double lon;
	double alt;
	double psi;
	double theta;
	double gamma;
};

struct FlightInfo {
	QString aType;
	QString flight;
	int from;
	int to;
	osgEarth::DateTime departureTime;
	osgEarth::DateTime arrivalTime;
};

extern QHash<int, AirportData> airportsList;
extern std::vector<int> planesInTheSky;
extern QHash<int, FlightInfo> planesList;
extern QHash<int, int> planeCurrentIndex;
extern QHash<int, std::vector<FlightPoint>> planePoints;

extern QHash<int, FlightPoint> planesCurrentPosition;

extern QString file;
#endif