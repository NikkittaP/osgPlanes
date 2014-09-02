#include "stdafx.h"
#include "Variables.h"
#include "OverlayPanels.h"
#include "EventHandlers.h"

using namespace osgEarth::Util;

Controls::LabelControl* titleLabel;
Controls::LabelControl* latLabel;
Controls::LabelControl* lonLabel;
Controls::LabelControl* altLabel;
Controls::LabelControl* psiLabel;
Controls::LabelControl* gammaLabel;
Controls::LabelControl* thetaLabel;
Controls::LabelControl* latLabelValue;
Controls::LabelControl* lonLabelValue;
Controls::LabelControl* altLabelValue;
Controls::LabelControl* psiLabelValue;
Controls::LabelControl* gammaLabelValue;
Controls::LabelControl* thetaLabelValue;

osg::Node* createOverlayPanels(osgViewer::View* view)
{
	canvasOverlay = Controls::ControlCanvas::get(view);

	commonControlsGrid = new Controls::Grid();
	commonControlsGrid->setBackColor(0, 0, 0, 0.5);
	commonControlsGrid->setMargin(10);
	commonControlsGrid->setPadding(10);
	commonControlsGrid->setChildSpacing(10);
	commonControlsGrid->setChildVertAlign(Controls::Control::ALIGN_CENTER);
	commonControlsGrid->setAbsorbEvents(true);
	commonControlsGrid->setVertAlign(Controls::Control::ALIGN_TOP);
	commonControlsGrid->setHorizAlign(Controls::Control::ALIGN_CENTER);

	messagesGrid = new Controls::Grid();
	messagesGrid->setBackColor(0, 0, 0, 0.5);
	messagesGrid->setMargin(10);
	messagesGrid->setPadding(10);
	messagesGrid->setChildSpacing(10);
	messagesGrid->setChildVertAlign(Controls::Control::ALIGN_CENTER);
	messagesGrid->setAbsorbEvents(true);
	messagesGrid->setVertAlign(Controls::Control::ALIGN_TOP);
	messagesGrid->setHorizAlign(Controls::Control::ALIGN_LEFT);
	messagesGrid->setVisible(false);

	planeInfoGrid = new Controls::Grid();
	planeInfoGrid->setBackColor(0, 0, 0, 0.5);
	planeInfoGrid->setMargin(10);
	planeInfoGrid->setPadding(10);
	planeInfoGrid->setChildSpacing(10);
	planeInfoGrid->setChildVertAlign(Controls::Control::ALIGN_CENTER);
	planeInfoGrid->setAbsorbEvents(true);
	planeInfoGrid->setVertAlign(Controls::Control::ALIGN_TOP);
	planeInfoGrid->setHorizAlign(Controls::Control::ALIGN_RIGHT);
	planeInfoGrid->setVisible(false);

	canvasOverlay->addControl(commonControlsGrid);
	canvasOverlay->addControl(messagesGrid);
	canvasOverlay->addControl(planeInfoGrid);
	return canvasOverlay;
}
void createPlaneInfoPanel(int id)
{
	planeInfoGrid->setVisible(true);

	titleLabel = new Controls::LabelControl(planesOnEarth[id]->getName());
	titleLabel->setHorizAlign(Controls::Control::ALIGN_RIGHT);
	titleLabel->setPadding(20);
	titleLabel->setFontSize(24);
	planeInfoGrid->setControl(1, 0, titleLabel);

	latLabel = new Controls::LabelControl("Latitude: ");
	latLabel->setHorizAlign(Controls::Control::ALIGN_LEFT);
	planeInfoGrid->setControl(0, 1, latLabel);
	latLabelValue = new Controls::LabelControl("");
	latLabelValue->setHorizAlign(Controls::Control::ALIGN_RIGHT);
	planeInfoGrid->setControl(1, 1, latLabelValue);

	lonLabel = new Controls::LabelControl("Longitude: ");
	lonLabel->setHorizAlign(Controls::Control::ALIGN_LEFT);
	planeInfoGrid->setControl(0, 2, lonLabel);
	lonLabelValue = new Controls::LabelControl("");
	lonLabelValue->setHorizAlign(Controls::Control::ALIGN_RIGHT);
	planeInfoGrid->setControl(1, 2, lonLabelValue);

	altLabel = new Controls::LabelControl("Altitude: ");
	altLabel->setHorizAlign(Controls::Control::ALIGN_LEFT);
	planeInfoGrid->setControl(0, 3, altLabel);
	altLabelValue = new Controls::LabelControl("");
	altLabelValue->setHorizAlign(Controls::Control::ALIGN_RIGHT);
	planeInfoGrid->setControl(1, 3, altLabelValue);

	psiLabel = new Controls::LabelControl("Heading: ");
	psiLabel->setHorizAlign(Controls::Control::ALIGN_LEFT);
	planeInfoGrid->setControl(0, 4, psiLabel);
	psiLabelValue = new Controls::LabelControl("");
	psiLabelValue->setHorizAlign(Controls::Control::ALIGN_RIGHT);
	planeInfoGrid->setControl(1, 4, psiLabelValue);

	gammaLabel = new Controls::LabelControl("Gamma: ");
	gammaLabel->setHorizAlign(Controls::Control::ALIGN_LEFT);
	planeInfoGrid->setControl(0, 5, gammaLabel);
	gammaLabelValue = new Controls::LabelControl("");
	gammaLabelValue->setHorizAlign(Controls::Control::ALIGN_RIGHT);
	planeInfoGrid->setControl(1, 5, gammaLabelValue);

	thetaLabel = new Controls::LabelControl("Theta: ");
	thetaLabel->setHorizAlign(Controls::Control::ALIGN_LEFT);
	planeInfoGrid->setControl(0, 6, thetaLabel);
	thetaLabelValue = new Controls::LabelControl("");
	thetaLabelValue->setHorizAlign(Controls::Control::ALIGN_RIGHT);
	planeInfoGrid->setControl(1, 6, thetaLabelValue);

	planeInfoGrid->setControl(0, 15, new Controls::LabelControl("Follow plane"));
	Controls::CheckBoxControl* checkBox = new Controls::CheckBoxControl(false, new Toggle("followPlane"));
	checkBox->setHorizAlign(Controls::Control::ALIGN_CENTER);
	planeInfoGrid->setControl(1, 15, checkBox);
}
void updatePlaneInfoPanel(int flight_id)
{
	latLabelValue->setText(std::to_string(((int)round(planesCurrentPosition[flight_id].lat * 100)) / 100.0));
	lonLabelValue->setText(std::to_string(((int)round(planesCurrentPosition[flight_id].lon * 100)) / 100.0));
	altLabelValue->setText(std::to_string(((int)round(planesCurrentPosition[flight_id].alt * 100)) / 100.0));
	psiLabelValue->setText(std::to_string(((int)round(planesCurrentPosition[flight_id].psi * 100)) / 100.0));
	gammaLabelValue->setText(std::to_string(((int)round(planesCurrentPosition[flight_id].gamma * 100)) / 100.0));
	thetaLabelValue->setText(std::to_string(((int)round(planesCurrentPosition[flight_id].theta * 100)) / 100.0));
}