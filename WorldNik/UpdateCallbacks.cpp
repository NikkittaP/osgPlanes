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

	if (isPlanesAdded)
	{
		for (int i = 0; i < planesInTheSky.size(); i++)
		{
			if (!planesGroup->containsNode(planesOnEarth[planesInTheSky[i]].get()))
			{
				planesNamesGroup->addChild(labelsOnEarth[planesInTheSky[i]].get());
				planesGroup->addChild(planesOnEarth[planesInTheSky[i]].get());
			}
		}
		isPlanesAdded = false;
	}

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

			planesCurrentPosition[flight_id].seconds += 1.0f / _denom;

			while ((_idx + 1) < planePoints[flight_id].size() && planesCurrentPosition[flight_id].seconds >= planePoints[flight_id][_idx + 1].seconds)
			{
				planeCurrentIndex[flight_id] += 1;
				_idx += 1;
			}

			if ((_idx + 1) >= planePoints[flight_id].size())
			{
				int size = planePoints[flight_id].size() - 1;
				if (size != -1)
				{
					planesCurrentPosition[flight_id].seconds = planePoints[flight_id][size].seconds;
					planesCurrentPosition[flight_id].lat = planePoints[flight_id][size].lat;
					planesCurrentPosition[flight_id].lon = planePoints[flight_id][size].lon;
					planesCurrentPosition[flight_id].alt = planePoints[flight_id][size].alt;
					planesCurrentPosition[flight_id].theta = planePoints[flight_id][size].theta;
					planesCurrentPosition[flight_id].gamma = planePoints[flight_id][size].gamma;
					planesCurrentPosition[flight_id].psi = planePoints[flight_id][size].psi;
				}
				justLandedPlanes.push_back(flight_id);
				it = planesInTheSky.erase(it);
			}
			else
			{
				double dt = (planesCurrentPosition[flight_id].seconds - planePoints[flight_id][_idx].seconds) /
					(planePoints[flight_id][_idx + 1].seconds - planePoints[flight_id][_idx].seconds);

				planesCurrentPosition[flight_id].lat = planePoints[flight_id][_idx].lat +
					(planePoints[flight_id][_idx + 1].lat - planePoints[flight_id][_idx].lat)*
					dt;
				planesCurrentPosition[flight_id].lon = planePoints[flight_id][_idx].lon +
					(planePoints[flight_id][_idx + 1].lon - planePoints[flight_id][_idx].lon)*
					dt;
				planesCurrentPosition[flight_id].alt = planePoints[flight_id][_idx].alt +
					(planePoints[flight_id][_idx + 1].alt - planePoints[flight_id][_idx].alt)*
					dt;
				planesCurrentPosition[flight_id].theta = planePoints[flight_id][_idx].theta +
					(planePoints[flight_id][_idx + 1].theta - planePoints[flight_id][_idx].theta)*
					dt;
				planesCurrentPosition[flight_id].gamma = planePoints[flight_id][_idx].gamma +
					(planePoints[flight_id][_idx + 1].gamma - planePoints[flight_id][_idx].gamma)*
					dt;
				planesCurrentPosition[flight_id].psi = planePoints[flight_id][_idx].psi +
					(planePoints[flight_id][_idx + 1].psi - planePoints[flight_id][_idx].psi)*
					dt;

				++it;
			}
			planesOnEarth[flight_id]->setPosition(GeoPoint(geoSRS, planesCurrentPosition[flight_id].lon, planesCurrentPosition[flight_id].lat, planesCurrentPosition[flight_id].alt, ALTMODE_ABSOLUTE));
			osg::Quat quat(
				osg::PI*planesCurrentPosition[flight_id].theta / 180.0, osg::X_AXIS,
				osg::PI*planesCurrentPosition[flight_id].gamma / 180.0, osg::Y_AXIS,
				-osg::PI* planesCurrentPosition[flight_id].psi / 180.0, osg::Z_AXIS
				);
			planesOnEarth[flight_id]->setLocalRotation(quat);

			LabelNode* _lbl = dynamic_cast<LabelNode*>(labelsOnEarth[flight_id]->getChild(0));
			_lbl->setPosition(GeoPoint(geoSRS, planesCurrentPosition[flight_id].lon, planesCurrentPosition[flight_id].lat, planesCurrentPosition[flight_id].alt, ALTMODE_ABSOLUTE));

			/***************************************************/

			if (flight_id == _selectedPlane)
			{
				updatePlaneInfoPanel(flight_id);

				if (isShowingChart && !drawFlightInfoThread->isRunning())
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
		}
		mMutex.unlock();

		_prev_clock = clock();
	}

	traverse(node, nv);
}

void UpdatePlanesInTheSky::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
	if ((clock() - _prev_sun_clock) / double(CLOCKS_PER_SEC) >= 1.0 / STEPS_PER_SECOND)
	{
		sky->setDateTime(currentDateTime);
		_prev_sun_clock = clock();
	}

	if (dataSource == DB_SOURCE && (DBUpdateIntervalUpdated || (currentDateTime.asTimeStamp() - _prev_clock) >= DBUpdateInterval))
	{
		double timestamp = currentDateTime.asTimeStamp();

		if (updatePlanesInTheSkyThread == NULL || !updatePlanesInTheSkyThread->isRunning())
		{
			isUpdatingPlanesInTheSky = true;
			/* Delete planes that have landed */
			mMutex.lock();
			for (int i = 0; i < justLandedPlanes.size(); i++)
			{
				planeCurrentIndex.remove(justLandedPlanes[i]);
				planePoints.remove(justLandedPlanes[i]);
				planesCurrentPosition.remove(justLandedPlanes[i]);
				planesGroup->removeChild(planesOnEarth[justLandedPlanes[i]]);
				planesOnEarth.remove(justLandedPlanes[i]);
				planesNamesGroup->removeChild(labelsOnEarth[justLandedPlanes[i]]);
				labelsOnEarth.remove(justLandedPlanes[i]);
				landedPlanes.push_back(justLandedPlanes[i]);
			}
			mMutex.unlock();
			QVector<int>().swap(justLandedPlanes);
			/***********************************/

			updatePlanesInTheSkyThread = new UpdatePlanesInTheSkyThread(timestamp);
			updatePlanesInTheSkyThread->setCancelModeDeferred();
			updatePlanesInTheSkyThread->start();
		}

		_prev_clock = currentDateTime.asTimeStamp();

		if (DBUpdateIntervalUpdated)
			DBUpdateIntervalUpdated = false;
	}

	traverse(node, nv);
}