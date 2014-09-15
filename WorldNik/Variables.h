#ifndef Variables_HEADER
#define Variables_HEADER

#include "stdafx.h"
#include "NodeVisitors.h"
#include "BackgroundThreads.h"

enum DataSource
{
	DB_SOURCE,
	FILE_SOURCE
};

extern DataSource dataSource;

const int SCREEN_NUM = 1;
extern std::string EARTH_FILE;
extern int STEPS_PER_SECOND;
extern std::string PATH;
extern unsigned int screenWidth, screenHeight, hudChartWidth, hudChartHeight;

extern int numPlanesToGet;
extern osg::ref_ptr<osg::Node> globe;
extern const osgEarth::Util::SpatialReference* mapSRS;
extern const osgEarth::Util::SpatialReference* geoSRS;

extern osgEarth::Annotation::FeatureNode* pathNode;
extern osgEarth::Annotation::FeatureNode* pathNode_surface;
extern osg::ref_ptr<osg::Group> connections;
extern osg::ref_ptr<osg::Group> visualTrajectories;
extern osg::ref_ptr<osg::Group> visualChart;

extern osg::ref_ptr<osgEarth::MapNode> mapNode;
extern osg::ref_ptr<osg::Group> planesGroup;
extern osg::ref_ptr<osg::Group> planesNamesGroup;
extern osg::ref_ptr<osg::Group> root;
extern QHash<int, osg::ref_ptr<osgEarth::Annotation::ModelNode>> planesOnEarth;
extern QHash<int, osg::ref_ptr<osg::Switch>> labelsOnEarth;
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
extern bool DBUpdateIntervalUpdated;
extern double DBUpdateInterval;
extern int InterpolationIndexVariable;
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
extern UpdatePlanesInTheSkyThread *updatePlanesInTheSkyThread;
extern bool isUpdatingPlanesInTheSky;

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

struct FlightPlan {
	int WPTnum;
	QString WPTname;
	double lat;
	double lon;
	double alt;
};

extern QHash<int, AirportData> airportsList;
extern QMutex mMutex;
extern QVector<int> planesInTheSky;
extern QVector<int> justLandedPlanes;
extern QVector<int> landedPlanes;
extern QHash<int, FlightInfo> planesList;
extern QHash<int, std::vector<FlightPlan>> flightPlans;
extern QHash<int, int> planeCurrentIndex;
extern QHash<int, std::vector<FlightPoint>> planePoints;

extern QHash<int, FlightPoint> planesCurrentPosition;

extern QString file;

extern osg::ref_ptr<osg::Node> plane_high;

extern QSqlDatabase db;
#endif