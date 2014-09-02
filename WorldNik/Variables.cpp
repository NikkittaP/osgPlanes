#include "stdafx.h"
#include "Variables.h"
#include "NodeVisitors.h"

int STEPS_PER_SECOND = 30;
std::string PATH;
unsigned int screenWidth, screenHeight, hudChartWidth, hudChartHeight = 200;

int numPlanes;
std::vector<std::vector<float>> seconds;
std::vector<std::vector<float>> lat;
std::vector<std::vector<float>> lon;
std::vector<std::vector<float>> alt;
std::vector<std::vector<float>> psi;
std::vector<std::vector<float>> tetta;
std::vector<std::vector<float>> gamma;
//std::vector<float> planeT, planeLat, planeLon, planeAlt, planeTheta, planeGamma, planePsi;
const osgEarth::Util::SpatialReference* mapSRS;
const osgEarth::Util::SpatialReference* geoSRS;

osgEarth::Annotation::FeatureNode* pathNode;
osgEarth::Annotation::FeatureNode* pathNode_surface;
osg::ref_ptr<osg::Group> connections;

osg::ref_ptr<osgEarth::MapNode> mapNode;
osg::ref_ptr<osg::Group> planesGroup;
osg::ref_ptr<osg::Group> planesNamesGroup;
osg::ref_ptr<osg::Group> root;
std::vector<osg::ref_ptr<osgEarth::Annotation::ModelNode>> planesOnEarth;
osgEarth::Util::SkyNode* sky;
osgEarth::DateTime currentDateTime = osgEarth::DateTime("2013-07-01 00:00:00");
osgEarth::Util::Controls::LabelControl* currentDateTimeLabel;

osgViewer::Viewer viewer;
osg::ref_ptr<osgEarth::Util::EarthManipulator> e_manip;
osg::Camera* verticalProfileHUD;

bool isFollowingPlane = false;
double prev_distance_to_earth;
int scale;

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

QHash<int, AirportData> airportsList;
std::vector<int> planesInTheSky;
QHash<int, FlightInfo> planesList;
QHash<int, int> planeCurrentIndex;
QHash<int, std::vector<FlightPoint>> planePoints;

QHash<int, FlightPoint> planesCurrentPosition;

QString file;