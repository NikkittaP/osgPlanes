#include "stdafx.h"
#include "Variables.h"
#include "NodeVisitors.h"

DataSource dataSource;

int STEPS_PER_SECOND = 30;
std::string PATH;
unsigned int screenWidth, screenHeight, hudChartWidth, hudChartHeight = 200;

int numPlanesToGet;
osg::ref_ptr<osg::Node> globe;
const osgEarth::Util::SpatialReference* mapSRS;
const osgEarth::Util::SpatialReference* geoSRS;

osgEarth::Annotation::FeatureNode* pathNode;
osgEarth::Annotation::FeatureNode* pathNode_surface;
osg::ref_ptr<osg::Group> connections;
osg::ref_ptr<osg::Group> visualTrajectories;

osg::ref_ptr<osgEarth::MapNode> mapNode;
osg::ref_ptr<osg::Group> planesGroup;
osg::ref_ptr<osg::Group> planesNamesGroup;
osg::ref_ptr<osg::Group> root;
QHash<int, osg::ref_ptr<osgEarth::Annotation::ModelNode>> planesOnEarth;
QHash<int, osg::ref_ptr<osg::Switch>> labelsOnEarth;
osgEarth::Util::SkyNode* sky;
osgEarth::DateTime currentDateTime = osgEarth::DateTime("2011-07-01 00:00:00");
osgEarth::Util::Controls::LabelControl* currentDateTimeLabel;

osgViewer::Viewer viewer;
osg::ref_ptr<osgEarth::Util::EarthManipulator> e_manip;
osg::Camera* verticalProfileHUD;

bool isFollowingPlane = false;
double prev_distance_to_earth;
int scale = 1;

double _speed = 1;
int _planeIndex = 0;
int _selectedPlane = -1;
clock_t start, end;

std::vector<float> _time, timeNorm;
std::vector<float> lat_whole, lon_whole;
std::vector<float> H_whole, H_wholeNorm;
std::vector<float> HNorm;
double _chartMaxHeight, _chartMaxTime;
double XunitsInPixel, YunitsInPixel;
osg::Sphere* planeAltitudeSphere;
osgText::Text* planeAltitudeText;
osg::ref_ptr<osg::ShapeDrawable> chartPlanePosition;
bool isShowingChart = false;

osg::Geode* chartGeode;

osgEarth::Util::Controls::ControlCanvas* canvasOverlay;
osgEarth::Util::Controls::Grid* commonControlsGrid = NULL;
osgEarth::Util::Controls::Grid* messagesGrid = NULL;
osgEarth::Util::Controls::Grid* planeInfoGrid = NULL;
osgEarth::Util::Controls::LabelControl* speedControlLabel;
osgEarth::Util::Controls::HSliderControl* speedControl;
osgEarth::Util::Controls::CheckBoxControl* speedControlCheckBox_100;
osgEarth::Util::Controls::CheckBoxControl* speedControlCheckBox_1000;

DrawFlightInfoThread *drawFlightInfoThread;
UpdatePlanesInTheSkyThread *updatePlanesInTheSkyThread;
bool isUpdatingPlanesInTheSky;

QHash<int, AirportData> airportsList;
QMutex mMutex;
QVector<int> planesInTheSky;
QVector<int> justLandedPlanes;
QVector<int> landedPlanes;
QHash<int, FlightInfo> planesList;
QHash<int, std::vector<FlightPlan>> flightPlans;
QHash<int, int> planeCurrentIndex;
QHash<int, std::vector<FlightPoint>> planePoints;

QHash<int, FlightPoint> planesCurrentPosition;

QString file;

//osg::ref_ptr<osg::Node> plane_high_ = osgDB::readNodeFile("D:/Progs/3DModels/A320-200/A320-200.osg.0,0,180.rot");
//osg::ref_ptr<osg::Node> plane_high = osgDB::readNodeFile("D:/Progs/3DModels/cessna_simple.osg.0,0,180.rot");
osg::ref_ptr<osg::Node> plane_high;

QSqlDatabase db;