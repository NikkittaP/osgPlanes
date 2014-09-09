#include "stdafx.h"
#include "Variables.h"
#include "BackgroundThreads.h"
#include "Data.h"
#include "Chart.h"

using namespace osgEarth::Util;

void DrawFlightInfoThread::run()
{
	root->removeChild(verticalProfileHUD);
	verticalProfileHUD = NULL;
	messagesGrid->setVisible(true);
	Controls::LabelControl* lbl = new Controls::LabelControl("Loading flight line ...");
	lbl->setVertAlign(Controls::Control::ALIGN_BOTTOM);
	messagesGrid->setControl(0, 0, lbl);

	isShowingChart = false;
	//OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
	DrawFlightLine(_id);
	lbl->setText("Loading vertical profile ...");
	//ShowVerticalProfile(_id);
	//isShowingChart = true;

	lbl->setText("Done");
	messagesGrid->clearControls();
	messagesGrid->setVisible(false);
}

void UpdatePlanesInTheSkyThread::run()
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
		planesNamesGroup->removeChild(justLandedPlanes[i]);
		labelsOnEarth.remove(justLandedPlanes[i]);
		landedPlanes.push_back(justLandedPlanes[i]);
	}
	mMutex.unlock();
	QVector<int>().swap(justLandedPlanes);
	/***********************************/

	loadPlanesPoints(timestamp);

	isUpdatingPlanesInTheSky = false;
}