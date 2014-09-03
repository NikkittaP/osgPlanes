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
	DrawFlightLines(_id);
	lbl->setText("Loading vertical profile ...");
	ShowVerticalProfile(_id);
	isShowingChart = true;

	lbl->setText("Done");
	messagesGrid->clearControls();
	messagesGrid->setVisible(false);
}

void UpdatePlanesInTheSkyThread::run()
{
	clock_t begin = clock();
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
		landedPlanes.push_back(justLandedPlanes[i]);
	}
	mMutex.unlock();
	QVector<int>().swap(justLandedPlanes);
	/***********************************/

	loadPlanesPoints(timestamp);

	isUpdatingPlanesInTheSky = false;

	clock_t end = clock();
	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	std::cout << "Background: " << elapsed_secs << std::endl;
	file += "Background: " + QString::number(elapsed_secs) + "\n";
}