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
	osg::Group* planes = dynamic_cast<osg::Group*>(node);

	if (planes && (clock() - _prev_clock) / double(CLOCKS_PER_SEC) >= 1.0 / STEPS_PER_SECOND)
	{
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
			int _idx = planeCurrentIndex[flight_id];
			if ((_idx + 1) < planePoints[flight_id].size() && planesCurrentPosition[flight_id].seconds >= planePoints[flight_id][_idx + 1].seconds)
			{
				planeCurrentIndex[flight_id]++;
				_idx++;
			}

			if ((_idx + 1) >= planePoints[flight_id].size())
			{
				it = planesInTheSky.erase(it);
				justLandedPlanes.push_back(*it);
			}
			else
			{

				/* Блок независящий от источника данных */

				planesCurrentPosition[flight_id].seconds += 1.0f / _denom;
				planesCurrentPosition[flight_id].lat += (planePoints[flight_id][_idx + 1].lat - planePoints[flight_id][_idx].lat) / (10.0f * _denom);
				planesCurrentPosition[flight_id].lon += (planePoints[flight_id][_idx + 1].lon - planePoints[flight_id][_idx].lon) / (10.0f * _denom);
				planesCurrentPosition[flight_id].alt += (planePoints[flight_id][_idx + 1].alt - planePoints[flight_id][_idx].alt) / (10.0f * _denom);
				planesCurrentPosition[flight_id].theta += (planePoints[flight_id][_idx + 1].theta - planePoints[flight_id][_idx].theta) / (10.0f * _denom);
				planesCurrentPosition[flight_id].gamma += (planePoints[flight_id][_idx + 1].gamma - planePoints[flight_id][_idx].gamma) / (10.0f * _denom);
				planesCurrentPosition[flight_id].psi += (planePoints[flight_id][_idx + 1].psi - planePoints[flight_id][_idx].psi) / (10.0f * _denom);

				file += QString::number(planeCurrentIndex[flight_id]) + "\t" +
					QString::number(planesCurrentPosition[flight_id].seconds) + "\t" +
					QString::number(_time, 'g', 16) + "\t" +
					QString::fromStdString(currentDateTime.asISO8601()) + "\n";

				/*if (planesCurrentPosition[flight_id].seconds >= 2000)
				{
				QString filename = "H:\Data.txt";
				QFile _file(filename);
				if (_file.open(QIODevice::ReadWrite))
				{
				QTextStream stream(&_file);
				stream << file << endl;
				}
				break;
				}*/

				FindNodeVisitor findNode(planesList[*it].flight.toStdString());
				planes->accept(findNode);
				ModelNode* plane = dynamic_cast<ModelNode*>(findNode.getFirst());
				plane->setPosition(GeoPoint(geoSRS, planesCurrentPosition[flight_id].lon, planesCurrentPosition[flight_id].lat, planesCurrentPosition[flight_id].alt, ALTMODE_ABSOLUTE));
				osg::Quat quat(
					osg::PI*planesCurrentPosition[flight_id].theta / 180.0, osg::X_AXIS,
					osg::PI*planesCurrentPosition[flight_id].gamma / 180.0, osg::Y_AXIS,
					-osg::PI* planesCurrentPosition[flight_id].psi / 180.0, osg::Z_AXIS
					);
				plane->setLocalRotation(quat);

				FindNodeVisitor findLabelNode(planesList[*it].flight.toStdString());
				planesNamesGroup->accept(findLabelNode);
				osg::Switch* _switch = dynamic_cast<osg::Switch*>(findLabelNode.getFirst());
				LabelNode* _lbl = dynamic_cast<LabelNode*>(_switch->getChild(0));
				_lbl->setPosition(GeoPoint(geoSRS, planesCurrentPosition[flight_id].lon, planesCurrentPosition[flight_id].lat, planesCurrentPosition[flight_id].alt, ALTMODE_ABSOLUTE));

				/***************************************************/

				if (*it == _selectedPlane)
				{
					updatePlaneInfoPanel(*it);

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

		for (int i = 0; i < planesOnEarth.size(); i++)
			planesOnEarth[i]->setScale(osg::Vec3(scale, scale, scale));
	}
	prev_distance_to_earth = e_manip->getDistance();

	traverse(node, nv);
}

void MoveSun::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
	

	traverse(node, nv);
}

void UpdatePlanesInTheSky::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
	if ((clock() - _prev_sun_clock) / double(CLOCKS_PER_SEC) >= 1.0 / STEPS_PER_SECOND)
	{
		sky->setDateTime(currentDateTime);
		std::cout << "In the sky: " << planesInTheSky.size() << ";\t landed: " << landedPlanes.size() << std::endl;
		_prev_sun_clock = clock();
	}

	if ((currentDateTime.asTimeStamp() - _prev_clock) >= 300.0)
	{
		double timestamp = currentDateTime.asTimeStamp();

		/* Delete planes that have landed */
		for (int i = 0; i < justLandedPlanes.size(); i++)
		{
			planeCurrentIndex.remove(justLandedPlanes[i]);
			planePoints.remove(justLandedPlanes[i]);
			planesCurrentPosition.remove(justLandedPlanes[i]);
			landedPlanes.push_back(justLandedPlanes[i]);
		}
		QVector<int>().swap(justLandedPlanes);
		/***********************************/

		loadPlanesPoints(timestamp);

		_prev_clock = currentDateTime.asTimeStamp();
	}

	traverse(node, nv);
}