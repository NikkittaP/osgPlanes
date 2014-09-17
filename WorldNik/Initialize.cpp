#include "stdafx.h"
#include "Variables.h"
#include "Initialize.h"
#include "Data.h"
#include "UpdateCallbacks.h"
#include "OverlayPanels.h"
#include "EventHandlers.h"

using namespace osgEarth;
using namespace osgEarth::Annotation;
using namespace osgEarth::Util;

void Initialize()
{
	ParseConfigFile();
	InitAuxiliary();
	InitEarth();
	InitSky();
	InitOcean();
	InitModels();

	planesNamesGroup = new osg::Group();
	planesNamesGroup->setDataVariance(osg::Object::DYNAMIC);
	Decluttering::setEnabled(planesNamesGroup->getOrCreateStateSet(), true);

	planesGroup = new osg::Group();
	planesGroup->setDataVariance(osg::Object::DYNAMIC);
	planesGroup->setName("All planes");
	planesGroup->setUpdateCallback(new MovePlanes);

	switch (dataSource)
	{
	case DB_SOURCE:
		connectToDB();
		loadAirports();
		loadPlanesList();
		loadFlightPlan();
		loadPlanesPoints(currentDateTime.asTimeStamp());
		break;
	case FILE_SOURCE:
		parseFlightFile();
		break;
	}

	root = new osg::Group;
	root->setDataVariance(osg::Object::DYNAMIC);
	root->addChild(sky);
	root->addChild(planesGroup.get());
	root->addChild(planesNamesGroup.get());

	visualTrajectories = new osg::Group();
	visualTrajectories->setDataVariance(osg::Object::DYNAMIC);
	root->addChild(visualTrajectories.get());

	visualChart = new osg::Group();
	visualChart->setDataVariance(osg::Object::DYNAMIC);
	root->addChild(visualChart.get());

	sky->attach(&viewer);

	//viewer.setThreadingModel(osgViewer::ViewerBase::ThreadPerCamera);

	//osgUtil::Optimizer optimizer;
	//optimizer.optimize(root);

	InitViewer();
	InitPanels();
}
void ParseConfigFile()
{
	std::ifstream infile(PATH + "config.cfg");
	std::string line;
	std::getline(infile, line);
	EARTH_FILE = line;
}
void InitModels()
{
	/* Load plane */
	plane_high = osgDB::readNodeFile(PATH + "my_plane.osg");
	plane_high->accept(whiteColor);
}
void InitEarth()
{
	/* Load earth */
	globe = osgDB::readNodeFile(EARTH_FILE);
	globe->setName("Earth");
	mapNode = MapNode::get(globe);
	LODBlending* effect = new LODBlending();
	mapNode->getTerrainEngine()->addEffect(effect);

	mapSRS = mapNode->getMapSRS();
	geoSRS = mapSRS->getGeographicSRS();

	osg::DisplaySettings::instance()->setNumMultiSamples(8);
}
void InitAuxiliary()
{
	InitColors();
}
void InitSky()
{
	/* Adding sky */
	sky = SkyNode::create(mapNode);
	sky->setDateTime(currentDateTime);
	sky->addChild(globe.get());
	sky->setUpdateCallback(new UpdatePlanesInTheSky);
}
void InitOcean()
{
	/* Adding ocean */
	OceanNode* ocean = OceanNode::create(mapNode);
	ocean->setSeaLevel(10);
	sky->addChild(ocean);
}
void InitViewer()
{
	viewer.setSceneData(root);
	viewer.setUpViewOnSingleScreen(SCREEN_NUM);
	viewer.getCamera()->setNearFarRatio(0.000001);
	viewer.addEventHandler(new PickHandler);
	e_manip = new EarthManipulator();
	viewer.setCameraManipulator(e_manip);

	viewer.addEventHandler(new osgViewer::StatsHandler()); // Can be deleted.
}
void InitPanels()
{
	createOverlayPanels(&viewer);

	speedControlLabel = new Controls::LabelControl("1.00x");
	speedControlLabel->setVertAlign(Controls::Control::ALIGN_CENTER);
	commonControlsGrid->setControl(2, 0, speedControlLabel);


	currentDateTimeLabel = new Controls::LabelControl(currentDateTime.asISO8601());
	currentDateTimeLabel->setVertAlign(Controls::Control::ALIGN_CENTER);
	currentDateTimeLabel->setHorizAlign(Controls::Control::ALIGN_CENTER);
	commonControlsGrid->setControl(1, 1, currentDateTimeLabel);

	speedControl = new Controls::HSliderControl(0, 10, 1);
	speedControl->setWidth(250);
	speedControl->setHeight(20);
	speedControl->setVertAlign(Controls::Control::ALIGN_CENTER);
	speedControl->addEventHandler(new SpeedHandler());
	commonControlsGrid->setControl(1, 0, speedControl);

	speedControlCheckBox_100 = new Controls::CheckBoxControl(false);
	speedControlCheckBox_100->addEventHandler(new SpeedHandler());
	speedControlCheckBox_100->setName("100x");
	commonControlsGrid->setControl(3, 0, speedControlCheckBox_100);

	Controls::LabelControl* lbl_100 = new Controls::LabelControl("100x");
	commonControlsGrid->setControl(4, 0, lbl_100);

	speedControlCheckBox_1000 = new Controls::CheckBoxControl(false);
	speedControlCheckBox_1000->addEventHandler(new SpeedHandler());
	speedControlCheckBox_1000->setName("1000x");
	commonControlsGrid->setControl(3, 1, speedControlCheckBox_1000);

	Controls::LabelControl* lbl_1000 = new Controls::LabelControl("1000x");
	commonControlsGrid->setControl(4, 1, lbl_1000);
}