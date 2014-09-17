#include "stdafx.h"
#include "Variables.h"
#include "Chart.h"
#include "Helpers.h"

using namespace osgEarth;
using namespace osgEarth::Annotation;
using namespace osgEarth::Util;

void ShowVerticalProfile(int flight_id)
{
	visualChart->removeChildren(0, visualChart->getNumChildren());
	_time.clear();
	H_whole.clear();
	lat_whole.clear();
	lon_whole.clear();
	for (int i = 0; i < planePoints[flight_id].size(); i++)
	{
		_time.push_back(planePoints[flight_id][i].seconds);
		H_whole.push_back(planePoints[flight_id][i].alt);
		lat_whole.push_back(planePoints[flight_id][i].lat);
		lon_whole.push_back(planePoints[flight_id][i].lon);
	}
	getScreenSize();
	hudChartWidth = screenWidth*0.8;
	hudChartHeight = 200;
	verticalProfileHUD = createHUD(screenWidth, screenHeight, hudChartWidth, hudChartHeight, CENTER_BOTTOM);
	visualChart->addChild(verticalProfileHUD);
}

void normalizeData(int hudWidth, int hudHeight)
{
	timeNorm.clear();
	HNorm.clear();
	H_wholeNorm.clear();
	_chartMaxTime = _time.back();
	XunitsInPixel = _chartMaxTime / hudWidth;
	_chartMaxHeight = *std::max_element(std::begin(H_whole), std::end(H_whole));
	YunitsInPixel = _chartMaxHeight / (hudHeight - 20);

	ElevationQuery _elevationQuery = mapNode->getMap();
	double elev;

	int _prevT = -1, prevH = -1;
	int _currT, _currHwhole;

	for (int i = 0; i < _time.size(); i++)
	{
		if (H_whole[i] != prevH)
		{
			_currT = (int)round(_time[i] / XunitsInPixel);
			if (_currT != _prevT)
			{
				_currHwhole = (int)round(H_whole[i] / YunitsInPixel);
				_elevationQuery.getElevation(GeoPoint(geoSRS, lon_whole[i], lat_whole[i]), elev);
				timeNorm.push_back(_currT);
				H_wholeNorm.push_back(_currHwhole);
				_prevT = _currT;
				HNorm.push_back(elev / YunitsInPixel);
			}
		}
		prevH = H_whole[i];
	}
}

osg::Geometry* getTerrainElevation(int windowWidth, int windowHeight, int hudWidth, int hudHeight, hudPositionType hudPosition, osg::Vec4 color)
{
	osg::ref_ptr<osg::Vec3Array> points = new osg::Vec3Array();
	int startX, startY;

	switch (hudPosition)
	{
	case (CENTER_BOTTOM) :
		startX = (windowWidth - hudWidth) / 2;
		startY = 0;
		break;
	default:
		break;
	}

	points->push_back(osg::Vec3(startX, startY, 0.f));
	double _x;
	for (int i = 0; i < HNorm.size(); i++)
	{
		_x = startX + timeNorm[i];
		double _y = startY + HNorm[i];
		points->push_back(osg::Vec3(_x, _y, 0.f));
	}
	points->push_back(osg::Vec3(_x, 0, 0.f));

	osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
	normals->push_back(osg::Vec3(0.0f, -1.0f, 0.0f));

	osg::Geometry* geometry = new osg::Geometry;
	geometry->setVertexArray(points.get());
	geometry->setNormalArray(normals.get());
	geometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
	geometry->addPrimitiveSet(new osg::DrawArrays(GL_POLYGON, 0, points.get()->size()));

	osgUtil::Tessellator tessellator;
	tessellator.retessellatePolygons(*geometry);

	osg::Vec4Array* colors = new osg::Vec4Array;
	colors->push_back(color);
	geometry->setColorArray(colors);
	geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

	return geometry;
}

osg::Geometry* getWholePath(int windowWidth, int windowHeight, int hudWidth, int hudHeight, hudPositionType hudPosition, osg::Vec4 color)
{
	osg::ref_ptr<osg::Vec3Array> points_all = new osg::Vec3Array();
	int startX, startY;

	switch (hudPosition)
	{
	case (CENTER_BOTTOM) :
		startX = (windowWidth - hudWidth) / 2;
		startY = 0;
		break;
	default:
		break;
	}

	for (int i = 0; i < timeNorm.size(); i++)
	{
		double _x = startX + timeNorm[i];
		double _y = startY + H_wholeNorm[i];
		points_all->push_back(osg::Vec3(_x, _y, 0.f));
	}

	osg::Geometry* geometry_all = new osg::Geometry;
	geometry_all->setVertexArray(points_all.get());
	geometry_all->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, points_all.get()->size()));
	osg::Vec4Array* colors = new osg::Vec4Array;
	colors->push_back(color);
	geometry_all->setColorArray(colors);
	geometry_all->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE_SET);

	return geometry_all;
}

osg::Geode* getTicks(int windowWidth, int windowHeight, int hudWidth, int hudHeight, hudPositionType hudPosition, osg::Vec4 color)
{
	int startX, startY;

	switch (hudPosition)
	{
	case (CENTER_BOTTOM) :
		startX = (windowWidth - hudWidth) / 2;
		startY = 0;
		break;
	default:
		break;
	}

	osg::Geode* ticks = new osg::Geode();
	osg::StateSet* stateset = ticks->getOrCreateStateSet();
	stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	int _stepHeight = 10000;
	if (_chartMaxHeight < 5000)
		_stepHeight = 1000;
	else if (_chartMaxHeight < 10000)
		_stepHeight = 3000;
	else if (_chartMaxHeight < 15000)
		_stepHeight = 5000;

	int _line = 0, _lineInPixel;
	while ((_lineInPixel = round(_line / YunitsInPixel)) < hudHeight)
	{
		osg::ref_ptr<osg::Vec3Array> points = new osg::Vec3Array();

		points->push_back(osg::Vec3(startX + 10, startY + _lineInPixel + 1, 0.0f));
		points->push_back(osg::Vec3(startX, startY + _lineInPixel + 1, 0.0f));
		points->push_back(osg::Vec3(startX, startY + _lineInPixel, 0.0f));
		points->push_back(osg::Vec3(startX + hudWidth, startY + _lineInPixel, 0.0f));

		osgText::Text* text = new  osgText::Text;
		ticks->addDrawable(text);

		std::string timesFont("fonts/times.ttf");
		text->setFont(timesFont);
		text->setCharacterSize(16);
		text->setText(std::to_string(_line));
		text->setPosition(osg::Vec3(startX + 5, startY + _lineInPixel + 5, 0));

		osg::Geometry* geometry_all = new osg::Geometry;
		geometry_all->setVertexArray(points.get());
		geometry_all->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, points.get()->size()));
		osg::Vec4Array* colors = new osg::Vec4Array;
		colors->push_back(color);
		geometry_all->setColorArray(colors);
		geometry_all->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE_SET);

		ticks->addDrawable(geometry_all);
		_line += _stepHeight;
	}

	return ticks;
}

osg::Camera* createHUD(int windowWidth, int windowHeight, int hudWidth, int hudHeight, hudPositionType hudPosition)
{
	int hudCenterX;
	int hudCenterY;

	switch (hudPosition)
	{
	case (CENTER_BOTTOM) :
		hudCenterX = windowWidth / 2;
		hudCenterY = hudHeight / 2;
		break;
	default:
		break;
	}
	osg::Camera* camera = new osg::Camera;
	camera->setProjectionMatrix(osg::Matrix::ortho2D(0, windowWidth, 0, windowHeight));
	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	camera->setViewMatrix(osg::Matrix::identity());
	camera->setClearMask(GL_DEPTH_BUFFER_BIT);
	camera->setRenderOrder(osg::Camera::POST_RENDER);
	camera->setAllowEventFocus(false);

	osg::Geode* geode_bg = new osg::Geode();
	// turn lighting off for the text and disable depth test to ensure it's always ontop.
	osg::StateSet* stateset_bg = geode_bg->getOrCreateStateSet();
	stateset_bg->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	stateset_bg->setMode(GL_BLEND, osg::StateAttribute::ON);
	stateset_bg->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

	osg::ref_ptr<osg::ShapeDrawable> shape = new osg::ShapeDrawable;
	shape->setShape(new osg::Box(osg::Vec3(hudCenterX, hudCenterY, -1), hudWidth, hudHeight, 1));
	shape->setColor(osg::Vec4(0, 0, 0, 0.3f));
	geode_bg->addDrawable(shape.get());

	chartGeode = new osg::Geode();
	osg::StateSet* stateset = chartGeode->getOrCreateStateSet();
	stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	normalizeData(hudWidth, hudHeight);

	chartGeode->addDrawable(getTerrainElevation(windowWidth, windowHeight, hudWidth, hudHeight, hudPosition, osg::Vec4(100.0f / 255.0f, 50.0f / 255.0f, 0.0f, 1.0f)));
	chartGeode->addDrawable(getWholePath(windowWidth, windowHeight, hudWidth, hudHeight, hudPosition, osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f)));

	planeAltitudeText = new osgText::Text;
	std::string timesFont("fonts/times.ttf");
	planeAltitudeText->setFont(timesFont);
	planeAltitudeText->setCharacterSize(18);
	planeAltitudeText->setColor(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
	chartGeode->addDrawable(planeAltitudeText.get());

	osg::ref_ptr<osg::Group> chart = new osg::Group;
	chart->addChild(getTicks(windowWidth, windowHeight, hudWidth, hudHeight, hudPosition, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f)));
	chart->addChild(geode_bg);
	chart->addChild(chartGeode.get());
	camera->addChild(chart);

	return camera;
}