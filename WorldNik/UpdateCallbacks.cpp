#include "stdafx.h"
#include "Variables.h"
#include "UpdateCallbacks.h"
#include "OverlayPanels.h"
#include "Data.h"

using namespace osgEarth;
using namespace osgEarth::Annotation;
using namespace osgEarth::Util;

void MovePlanes::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
	clock_t begin = clock();
	osg::Group* planes = dynamic_cast<osg::Group*>(node);

	if (planes && (clock() - _prev_clock) / double(CLOCKS_PER_SEC) >= 1.0 / STEPS_PER_SECOND)
	{
		mMutex.lock();
		int planesNum = planesInTheSky.size();
		double _denom = STEPS_PER_SECOND / _speed;

		_time += 1.0f / _denom;
		if ((_time - currentDateTime.asTimeStamp()) > 1)
		{
			currentDateTime = osgEarth::DateTime(round(_time));
			currentDateTimeLabel->setText(currentDateTime.asISO8601());
		}

		for (QVector<int>::iterator it = planesInTheSky.begin();
			it != planesInTheSky.end();)
		{
			int flight_id = *it;
			if (flight_id < 0)
				break;
			int _idx = planeCurrentIndex[flight_id];
			if ((_idx + 1) < planePoints[flight_id].size() && planesCurrentPosition[flight_id].seconds >= planePoints[flight_id][_idx + 1].seconds)
			{
				planeCurrentIndex[flight_id]++;
				_idx++;
			}

			if ((_idx + 1) >= planePoints[flight_id].size())
			{
				justLandedPlanes.push_back(flight_id);
				it = planesInTheSky.erase(it);
			}
			else
			{
				planesCurrentPosition[flight_id].seconds += 1.0f / _denom;
				planesCurrentPosition[flight_id].lat += (planePoints[flight_id][_idx + 1].lat - planePoints[flight_id][_idx].lat) / (10.0f * _denom);
				planesCurrentPosition[flight_id].lon += (planePoints[flight_id][_idx + 1].lon - planePoints[flight_id][_idx].lon) / (10.0f * _denom);
				planesCurrentPosition[flight_id].alt += (planePoints[flight_id][_idx + 1].alt - planePoints[flight_id][_idx].alt) / (10.0f * _denom);
				planesCurrentPosition[flight_id].theta += (planePoints[flight_id][_idx + 1].theta - planePoints[flight_id][_idx].theta) / (10.0f * _denom);
				planesCurrentPosition[flight_id].gamma += (planePoints[flight_id][_idx + 1].gamma - planePoints[flight_id][_idx].gamma) / (10.0f * _denom);
				planesCurrentPosition[flight_id].psi += (planePoints[flight_id][_idx + 1].psi - planePoints[flight_id][_idx].psi) / (10.0f * _denom);

				
				FindNamedNode findNode(planesList[flight_id].flight.toStdString());
				planes->accept(findNode);
				if (findNode.getNode() != NULL)
				{
					ModelNode* plane = dynamic_cast<ModelNode*>(findNode.getNode());
					plane->setPosition(GeoPoint(geoSRS, planesCurrentPosition[flight_id].lon, planesCurrentPosition[flight_id].lat, planesCurrentPosition[flight_id].alt, ALTMODE_ABSOLUTE));
					osg::Quat quat(
						osg::PI*planesCurrentPosition[flight_id].theta / 180.0, osg::X_AXIS,
						osg::PI*planesCurrentPosition[flight_id].gamma / 180.0, osg::Y_AXIS,
						-osg::PI* planesCurrentPosition[flight_id].psi / 180.0, osg::Z_AXIS
						);
					plane->setLocalRotation(quat);
				}
				FindNamedNode findLabelNode(planesList[flight_id].flight.toStdString());
				planesNamesGroup->accept(findLabelNode);
				if (findLabelNode.getNode() != NULL)
				{
					osg::Switch* _switch = dynamic_cast<osg::Switch*>(findLabelNode.getNode());
					LabelNode* _lbl = dynamic_cast<LabelNode*>(_switch->getChild(0));
					_lbl->setPosition(GeoPoint(geoSRS, planesCurrentPosition[flight_id].lon, planesCurrentPosition[flight_id].lat, planesCurrentPosition[flight_id].alt, ALTMODE_ABSOLUTE));
				}

				/***************************************************/

				if (flight_id == _selectedPlane)
				{
					updatePlaneInfoPanel(flight_id);

					if (isShowingChart)
					{
						chartGeode->removeDrawable(chartPlanePosition);
						chartPlanePosition = new osg::ShapeDrawable;
						planeAltitudeSphere = new osg::Sphere(osg::Vec3((screenWidth - hudChartWidth) / 2 + (planesCurrentPosition[flight_id].seconds / XunitsInPixel), (planesCurrentPosition[flight_id].alt / YunitsInPixel), 1), 5);
						chartPlanePosition->setShape(planeAltitudeSphere);
						chartPlanePosition->setColor(osg::Vec4(1, 0, 0, 1));
						chartGeode->addDrawable(chartPlanePosition.get());

						planeAltitudeText->setText(std::to_string((int)round(planesCurrentPosition[flight_id].alt)));
						planeAltitudeText->setPosition(osg::Vec3((screenWidth - hudChartWidth) / 2 + (planesCurrentPosition[flight_id].seconds / XunitsInPixel) + 10, (planesCurrentPosition[flight_id].alt / YunitsInPixel) - 5, 0));
					}
				}

				++it;
			}
		}
		mMutex.unlock();

		_prev_clock = clock();
	}

	if (e_manip->getDistance() != prev_distance_to_earth)
	{
		if (e_manip->getViewpoint().getRange() < 15000)
			viewer.getCamera()->setNearFarRatio(0.000001);
		else
			viewer.getCamera()->setNearFarRatio(0.0005);

		scale = 1 + round((e_manip->getDistance() - 500) / 500);
		if (scale < 1)
			scale = 1;
		else if (scale >1000)
			scale = 1000;

		osg::ref_ptr<osgEarth::Annotation::ModelNode> planeOnEarth;
		foreach(planeOnEarth, planesOnEarth)
		{
			if (planeOnEarth != NULL)
				planeOnEarth->setScale(osg::Vec3(scale, scale, scale));
		}
	}
	prev_distance_to_earth = e_manip->getDistance();

	clock_t end = clock();
	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	std::cout << "Move planes: " << elapsed_secs << std::endl;
	file += "Move planes: " + QString::number(elapsed_secs) + "\n";

	traverse(node, nv);
}

void UpdatePlanesInTheSky::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
	clock_t begin = clock();
	if ((clock() - _prev_sun_clock) / double(CLOCKS_PER_SEC) >= 1.0 / STEPS_PER_SECOND)
	{
		sky->setDateTime(currentDateTime);
		_prev_sun_clock = clock();
	}

	if (dataSource==DB_SOURCE && (currentDateTime.asTimeStamp() - _prev_clock) >= 300.0)
	{
		double timestamp = currentDateTime.asTimeStamp();

		updatePlanesInTheSkyThread = new UpdatePlanesInTheSkyThread(timestamp);
		updatePlanesInTheSkyThread->setCancelModeDeferred();
		updatePlanesInTheSkyThread->start();

		_prev_clock = currentDateTime.asTimeStamp();
	}

	clock_t end = clock();
	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	std::cout << "UpdatePlanesInTheSky: " << elapsed_secs << std::endl;
	file += "UpdatePlanesInTheSky: " + QString::number(elapsed_secs) + "\n";

	if (currentDateTime.asTimeStamp() >= 1372636800 + 60 * 60)
	{
		QString filename = "H:\Data.txt";
		QFile _file(filename);
		if (_file.open(QIODevice::ReadWrite))
		{
			QTextStream stream(&_file);
			stream << file << endl;
		}
		std::cout << "end" << endl;
		int aqe;
		std::cin >> aqe;
	}

	traverse(node, nv);
}