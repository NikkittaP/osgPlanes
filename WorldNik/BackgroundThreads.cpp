#include "stdafx.h"
#include "Variables.h"
#include "BackgroundThreads.h"
#include "Data.h"
#include "Chart.h"

using namespace osgEarth::Util;

void DrawFlightInfoThread::run()
{
	messagesGrid->setVisible(true);
	Controls::LabelControl* lbl = new Controls::LabelControl("Loading flight line ...");
	lbl->setVertAlign(Controls::Control::ALIGN_BOTTOM);
	messagesGrid->setControl(0, 0, lbl);

	isShowingChart = false;
	DrawFlightLine(_id);
	lbl->setText("Loading vertical profile ...");
	ShowVerticalProfile(_id);
	isShowingChart = true;

	lbl->setText("Done");
	messagesGrid->clearControls();
	messagesGrid->setVisible(false);
}

void UpdatePlanesInTheSkyThread::run()
{
	loadPlanesPoints(timestamp);

	isUpdatingPlanesInTheSky = false;
}