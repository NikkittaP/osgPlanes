// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <unordered_map>


#include <osgDB/ReadFile>
#include <osg/PositionAttitudeTransform>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgEarth/SpatialReference>
#include <osgEarthSymbology/Style>
#include <osgEarth/MapNode>
#include <osgEarthAnnotation/ModelNode>
#include <osgEarthAnnotation/LabelNode>
#include <osgEarthUtil/Sky>
#include <osgEarthUtil/Ocean>
#include <osgEarthUtil/EarthManipulator>
#include <osgEarthUtil/LODBlending>
#include <osgUtil/Optimizer>
#include <osgEarthSymbology/Geometry>
#include <osgEarthFeatures/Feature>
#include <osgEarthAnnotation/FeatureNode>

#include <osg/ShapeDrawable>
#include <osgUtil/Tessellator>

#include <osgEarth/ElevationQuery>
#include <osgText/Text>
#include <osgEarthUtil/Controls>

#include <dirent.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <QtCore/QCoreApplication>
#include <QtSQL/QtSQL>

#include <osg/LineWidth>

// TODO: reference additional headers your program requires here
