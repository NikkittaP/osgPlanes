#include "stdafx.h"
#include "Variables.h"
#include "UpdateCallbacks.h"
#include "Helpers.h"
#include "Chart.h"
#include "OverlayPanels.h"
#include "EventHandlers.h"
#include "NodeVisitors.h"
#include "Data.h"

using namespace osgEarth;
using namespace osgEarth::Annotation;
using namespace osgEarth::Util;

int _tmain(int argc, _TCHAR* argv[])
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
	db.setHostName("localhost");
	db.setDatabaseName("planes");
	db.setUserName("root");
	db.setPassword("");
	bool ok = db.open();

	loadAirports();
	loadPlanesList();
	loadPlanesPoints();
	
	QString s = QString::fromStdString(currentDateTime.asISO8601());
	s.replace("T", " ").replace("Z", "");
	qDebug() << s;

	InitColors();

	boost::filesystem::path full_path(boost::filesystem::initial_path<boost::filesystem::path>());
	full_path = boost::filesystem::system_complete(boost::filesystem::path(argv[0]));
	PATH = full_path.string().substr(0, full_path.string().size() - 12);

	std::cout << PATH << std::endl;
	//osg::ref_ptr<osg::Geode> plane = DrawSimplePlane();

	/* Load plane */
	osg::DisplaySettings::instance()->setNumMultiSamples(8);
	//osg::ref_ptr<osg::Node> plane_high_ = osgDB::readNodeFile("D:/Progs/3DModels/A320-200/A320-200.osg.0,0,180.rot");
	//osg::ref_ptr<osg::Node> plane_high = osgDB::readNodeFile("D:/Progs/3DModels/cessna_simple.osg.0,0,180.rot");
	osg::ref_ptr<osg::Node> plane_high = osgDB::readNodeFile(PATH + "cessna_simple.osg.0,0,0.rot");
	plane_high->accept(whiteColor);

	/* Load earth */
	//osg::ref_ptr<osg::Node> globe = osgDB::readNodeFile("C:/GoogleMaps_TMS/tms2.earth");
	osg::ref_ptr<osg::Node> globe = osgDB::readNodeFile("\\\\NIKKITTA\\GoogleMaps_TMS\\tms.earth");
	//osg::ref_ptr<osg::Node> globe = osgDB::readNodeFile(PATH + "readymap.earth");
	globe->setName("Earth");
	mapNode = MapNode::get(globe);
	LODBlending* effect = new LODBlending();
	mapNode->getTerrainEngine()->addEffect(effect);

	mapSRS = mapNode->getMapSRS();
	geoSRS = mapSRS->getGeographicSRS();

	/* Positioning plane on earth */
	Style style;
	planesNamesGroup = new osg::Group();
	Decluttering::setEnabled(planesNamesGroup->getOrCreateStateSet(), true);
	Style labelStyle;
	labelStyle.getOrCreate<TextSymbol>()->alignment() = TextSymbol::ALIGN_LEFT_BOTTOM_BASE_LINE;
	labelStyle.getOrCreate<TextSymbol>()->fill()->color() = Color::White;
	for (int i = 0; i < planesInTheSky.size(); i++)
	{
		osg::ref_ptr<osg::Node> plane_high_ = dynamic_cast<osg::Node*>(plane_high->clone(osg::CopyOp::DEEP_COPY_ALL));
		style.getOrCreate<ModelSymbol>()->setModel(plane_high_);
		planesOnEarth.push_back(new ModelNode(mapNode, style));
		planesOnEarth[i]->setName("Plane " + std::to_string(planesInTheSky[i]));
		planesOnEarth[i]->setPosition(GeoPoint(geoSRS, planesCurrentPosition[i].lon, planesCurrentPosition[i].lat, planesCurrentPosition[i].alt, ALTMODE_RELATIVE));

		osg::ref_ptr<LabelNode> _lbl = new LabelNode(mapNode, GeoPoint(geoSRS, planesCurrentPosition[i].lon, planesCurrentPosition[i].lat, planesCurrentPosition[i].alt, ALTMODE_RELATIVE), planesList[i].flight.toStdString(), labelStyle);
		osg::ref_ptr<osg::Switch> _showHideLbl = new osg::Switch();
		_showHideLbl->setName("Plane " + std::to_string(i) + " Label");
		_showHideLbl->addChild(_lbl);
		planesNamesGroup->addChild(_showHideLbl);
	}

	/* Adding planes to group */
	planesGroup = new osg::Group();
	planesGroup->setName("All planes");
	for (int i = 0; i < planesOnEarth.size(); i++)
		planesGroup->addChild(planesOnEarth[i].get());
	planesGroup->setUpdateCallback(new MovePlanes);

	/* Adding sky */
	sky = SkyNode::create(mapNode);
	sky->setDateTime(currentDateTime);
	sky->addChild(globe.get());
	sky->setUpdateCallback(new MoveSun);

	root = new osg::Group;
	root->addChild(sky);
	root->addChild(planesGroup.get());
	root->addChild(planesNamesGroup.get());

	/* Adding ocean */
	OceanNode* ocean = OceanNode::create(mapNode);
	ocean->setSeaLevel(10);
	sky->addChild(ocean);

	/* Setting up Viewer */
	sky->attach(&viewer);

	/* Optimizing graph */
	osgUtil::Optimizer optimizer;
	optimizer.optimize(root);

	/* Setting up Viewer */
	viewer.setSceneData(root);
	viewer.setUpViewOnSingleScreen(SCREEN_NUM);
	viewer.getCamera()->setNearFarRatio(0.000001);
	viewer.addEventHandler(new PickHandler);

	/* Setting up EarthManipulator */
	e_manip = new EarthManipulator();
	viewer.setCameraManipulator(e_manip);
	//osgEarth::Viewpoint view(4.738333, 52.315, 103.0, 0, 0, 200.0, geoSRS);
	//osgEarth::Viewpoint view(-118.504, 36.3345, 2810, -8.51114, -12.9368, 15678.1, geoSRS);
	//e_manip->setViewpoint(view, 1.0);

	viewer.addEventHandler(new osgViewer::StatsHandler());

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

	viewer.run();
	return 0;
}