#include "stdafx.h"
#include "Variables.h"
#include "EventHandlers.h"
#include "Helpers.h"
#include "NodeVisitors.h"
#include "OverlayPanels.h"
#include "BackgroundThreads.h"

using namespace osgEarth::Annotation;
using namespace osgEarth::Util;

bool PickHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	if (ea.getEventType() != osgGA::GUIEventAdapter::RELEASE
		|| ea.getButton() != osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
		return(false);
	osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
	if (!viewer)
		return(false);

	osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector = new osgUtil::LineSegmentIntersector(osgUtil::Intersector::WINDOW, ea.getX(), ea.getY());
	osgUtil::IntersectionVisitor iv(intersector.get());
	iv.setTraversalMask(~0x1);
	viewer->getCamera()->accept(iv);
	if (intersector->containsIntersections())
	{
		const osg::NodePath& nodePath = intersector->getFirstIntersection().nodePath;
		std::string prepend;
		for (int i = 0; i < nodePath.size(); i++)
		{
			/* Selecting plane */
			if (nodePath[i]->getName().find("Flight") != std::string::npos) {
				int _newID;
				if (nodePath[i]->getName().find("Label") != std::string::npos)
				{
					_newID = planesNamesGroup->getChildIndex(nodePath[i]);
				}
				else {
					std::string str = nodePath[i]->getName();
					str.erase(0, 7);
					_newID = std::stoi(str);
				}
				if (_selectedPlane != _newID)
				{
					if (_selectedPlane != -1)
					{
						labelsOnEarth[_selectedPlane]->setValue(0, true);
						planesOnEarth[_selectedPlane]->accept(whiteColor);
					}
					_selectedPlane = _newID;

					labelsOnEarth[_selectedPlane]->setValue(0, false);

					createPlaneInfoPanel(_selectedPlane);

					if (isFollowingPlane)
					{
						e_manip->setTetherNode(0L);
						isFollowingPlane = false;
					}

					if (drawFlightInfoThread == NULL || !drawFlightInfoThread->isRunning())
					{
						drawFlightInfoThread = new DrawFlightInfoThread(_selectedPlane);
						drawFlightInfoThread->setCancelModeDeferred();
						drawFlightInfoThread->start();
					}

					planesOnEarth[_selectedPlane]->accept(redColor);
					//LocalizedNode* node = dynamic_cast<LocalizedNode*>(nodePath[i]);
					//e_manip->setTetherNode(nodePath[i + 2], 0.5);
				}
			}
			else if (nodePath[i]->getName() == "Earth" && (ea.getModKeyMask()&osgGA::GUIEventAdapter::MODKEY_CTRL))
			{
				isShowingChart = false;
				labelsOnEarth[_selectedPlane]->setValue(0, true);
				planesOnEarth[_selectedPlane]->accept(whiteColor);
				_selectedPlane = -1;
				visualChart->removeChildren(0, visualChart->getNumChildren());
				visualTrajectories->removeChildren(0, visualTrajectories->getNumChildren());
				planeInfoGrid->clearControls();
				planeInfoGrid->setVisible(false);
				e_manip->setTetherNode(0L);
			}
		}
	}
	return(false);
}

void SpeedHandler::onValueChanged(Controls::Control* control, float value) {
	if (value >= 1)
	{
		_speed = round(value);
		if (value == 10)
		{
			DBUpdateInterval = 1200.0;
			DBUpdateIntervalUpdated = true;
		}
	}
	else
		_speed = round(value * 10) / 10.0;
	speedControlLabel->setText(std::to_string(_speed).substr(0, 5) + "x");
	speedControlCheckBox_100->setValue(false);
	speedControlCheckBox_1000->setValue(false);
	InterpolationIndexVariable = 1;
}

void SpeedHandler::onClick(osgEarth::Util::Controls::Control* control)
{
	Controls::CheckBoxControl* checkBox = dynamic_cast<Controls::CheckBoxControl*>(control);
	if (checkBox)
	{
		if (checkBox->getValue() == false)
			SpeedHandler::onValueChanged(speedControl, speedControl->getValue());
		else
		{
			if (checkBox->getName() == "100x")
			{
				_speed = 100;
				speedControlCheckBox_1000->setValue(false);
				InterpolationIndexVariable = 1;
				DBUpdateInterval = 3600.0;
			}
			else if (checkBox->getName() == "1000x")
			{
				_speed = 1000;
				speedControlCheckBox_100->setValue(false);
				InterpolationIndexVariable = 1;
				DBUpdateInterval = 7200.0;
			}
			DBUpdateIntervalUpdated = true;
			speedControlLabel->setText(std::to_string(_speed).substr(0, 5) + "x");
		}
	}
}

void Toggle::onValueChanged(Controls::Control*, bool value) {
	if (_type == "followPlane")
	{
		if (value)
		{
			e_manip->setTetherNode(planesOnEarth[_selectedPlane]->getChild(0), 0.5);
			isFollowingPlane = true;
		}
		else
		{
			e_manip->setTetherNode(0L);
			isFollowingPlane = false;
		}
	}
}