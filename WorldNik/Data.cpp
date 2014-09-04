#include "stdafx.h"
#include "Variables.h"
#include "Data.h"

using namespace osgEarth;
using namespace osgEarth::Annotation;
using namespace osgEarth::Util;

void connectToDB()
{
	db = QSqlDatabase::addDatabase("QMYSQL");
	db.setHostName("localhost");
	db.setDatabaseName("planes");
	db.setUserName("root");
	db.setPassword("");
	bool ok = db.open();
}

void loadAirports()
{
	QSqlQuery query;
	query.exec("SELECT * FROM airports");
	while (query.next()) {
		AirportData airport;
		airport.IATAcode = query.value(1).toString();
		airport.ICAOcode = query.value(2).toString();
		airport.name = query.value(4).toString();
		airport.city = query.value(3).toString();
		airport.country = query.value(5).toString();

		airportsList.insert(query.value(0).toInt(), airport);
	}
}

void loadPlanesList()
{
	QSqlQuery query;
	query.exec("SELECT * FROM flight_info");
	while (query.next()) {
		FlightInfo flight;
		flight.aType = query.value(2).toString();
		flight.flight = query.value(1).toString();
		flight.from = query.value(3).toInt();
		flight.to = query.value(4).toInt();
		flight.departureTime = osgEarth::DateTime(query.value(5).toString().toStdString());
		flight.arrivalTime = osgEarth::DateTime(query.value(6).toString().toStdString());

		planesList.insert(query.value(0).toInt(), flight);
	}
}

void loadPlanesPoints(double timestamp)
{
	QHashIterator<int, FlightInfo> iter(planesList);
	while (iter.hasNext())
	{
		iter.next();
		int flight_id = iter.key();
		if (!landedPlanes.contains(flight_id) && !planesInTheSky.contains(flight_id))
		{
			FlightInfo flight = iter.value();

			if (timestamp >= (flight.departureTime.asTimeStamp() - 300.0))
			{
				QSqlQuery query(db);
				std::vector<FlightPoint> tmp;
				query.prepare("SELECT second, latitude, longitude, altitude, psi, theta, gamma FROM flight_points WHERE flight_info_id = :id");
				query.bindValue(":id", flight_id);
				query.exec();
				while (query.next()) {
					FlightPoint point;
					point.seconds = query.value(0).toFloat();
					point.lat = query.value(1).toFloat();
					point.lon = query.value(2).toFloat();
					point.alt = query.value(3).toFloat();
					point.psi = query.value(4).toFloat();
					point.theta = query.value(5).toFloat();
					point.gamma = query.value(6).toFloat();

					tmp.push_back(point);
				}
				if (tmp.size() != 0)
				{
					mMutex.lock();
					planeCurrentIndex.insert(flight_id, 0);
					planePoints.insert(flight_id, tmp);
					planesCurrentPosition.insert(flight_id, planePoints[flight_id][0]);

					addPlanesToEarth(flight_id);

					planesInTheSky.push_back(flight_id);
					mMutex.unlock();
				}
			}
		}
	}
}

void addPlanesToEarth(int flight_id)
{
	Style style;
	Style labelStyle;
	labelStyle.getOrCreate<TextSymbol>()->alignment() = TextSymbol::ALIGN_LEFT_BOTTOM_BASE_LINE;
	labelStyle.getOrCreate<TextSymbol>()->fill()->color() = Color::White;

	osg::ref_ptr<osg::Node> plane_high_ = dynamic_cast<osg::Node*>(plane_high->clone(osg::CopyOp::DEEP_COPY_ALL));
	style.getOrCreate<ModelSymbol>()->setModel(plane_high_);
	planesOnEarth.insert(flight_id, new ModelNode(mapNode, style));
	planesOnEarth[flight_id]->setName(planesList[flight_id].flight.toStdString());
	planesOnEarth[flight_id]->setPosition(GeoPoint(geoSRS, planesCurrentPosition[flight_id].lon, planesCurrentPosition[flight_id].lat, planesCurrentPosition[flight_id].alt, ALTMODE_RELATIVE));
	planesOnEarth[flight_id]->setScale(osg::Vec3(scale, scale, scale));

	osg::ref_ptr<LabelNode> _lbl = new LabelNode(mapNode, GeoPoint(geoSRS, planesCurrentPosition[flight_id].lon, planesCurrentPosition[flight_id].lat, planesCurrentPosition[flight_id].alt, ALTMODE_RELATIVE), planesList[flight_id].flight.toStdString(), labelStyle);
	osg::ref_ptr<osg::Switch> _showHideLbl = new osg::Switch();
	_showHideLbl->setName(planesList[flight_id].flight.toStdString());
	_showHideLbl->addChild(_lbl);
	planesNamesGroup->addChild(_showHideLbl);

	planesGroup->addChild(planesOnEarth[flight_id].get());
}

void parseFlightFile()
{
	std::vector<std::string> files_list;
	DIR *dir;
	struct dirent *ent;
	std::string _path = PATH + "trajectories/";
	int count = 0;
	if ((dir = opendir(_path.c_str())) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			if (count == numPlanesToGet)
				break;
			std::string file = ent->d_name;
			if (file != "." && file != "..")
			{
				std::string path = _path;
				path.append(file);
				files_list.push_back(path);
				count++;
			}
		}
		closedir(dir);
	}

	for (int i = 0; i < files_list.size(); i++)
	{
		std::ifstream infile(files_list[i]);
		std::string line;

		std::vector<FlightPoint> tmp;

		while (std::getline(infile, line))
		{
			FlightPoint point;
			std::string arr[17];
			int i = 0;
			std::stringstream ssin(line);
			while (ssin.good() && i < 17){
				ssin >> arr[i];
				++i;
			}

			point.seconds = atof(arr[0].c_str());
			point.lat = atof(arr[5].c_str());
			point.lon = atof(arr[6].c_str());
			point.alt = atof(arr[1].c_str())*0.3048;
			point.psi = atof(arr[3].c_str());
			point.theta = atof(arr[15].c_str());
			point.gamma = atof(arr[16].c_str());

			tmp.push_back(point);
		}
		if (tmp.size() != 0)
		{
			FlightInfo flight;
			flight.aType = "A320";
			flight.flight = "Plane " + QString::number(i);

			planesList.insert(i, flight);

			planeCurrentIndex.insert(i, 0);
			planePoints.insert(i, tmp);
			planesCurrentPosition.insert(i, planePoints[i][0]);

			addPlanesToEarth(i);

			planesInTheSky.push_back(i);
		}
	}
}

void DrawFlightLines(int flight_id)
{
	if (flight_id != -1)
	{
		root->removeChild(connections);
		root->removeChild(pathNode);
		root->removeChild(pathNode_surface);

		connections = new osg::Group();
		Geometry* path = new LineString();
		Geometry* path_surface = new LineString();
		int k = 0;
		for (int i = 0; i < planePoints[flight_id].size(); i++)
		{
			if (k % 1 == 0 || i == planePoints[flight_id].size() - 1)
			{
				path->push_back(osg::Vec3d(planePoints[flight_id][i].lon, planePoints[flight_id][i].lat, planePoints[flight_id][i].alt));
				path_surface->push_back(osg::Vec3d(planePoints[flight_id][i].lon, planePoints[flight_id][i].lat, 0));
			}
			if (k % 50 == 0 || i == planePoints[flight_id].size() - 1)
			{
				Geometry* connection = new LineString();
				connection->push_back(osg::Vec3d(planePoints[flight_id][i].lon, planePoints[flight_id][i].lat, planePoints[flight_id][i].alt));
				connection->push_back(osg::Vec3d(planePoints[flight_id][i].lon, planePoints[flight_id][i].lat, 0));
				Style connectionStyle;
				connectionStyle.getOrCreate<LineSymbol>()->stroke()->color() = Color::Yellow;
				connectionStyle.getOrCreate<LineSymbol>()->stroke()->width() = 3.0f;
				connectionStyle.getOrCreate<AltitudeSymbol>()->clamping() = AltitudeSymbol::CLAMP_NONE;
				Feature* connectionFeature = new Feature(connection, geoSRS, connectionStyle);
				FeatureNode* connectionNode = new FeatureNode(mapNode, connectionFeature);
				connections->addChild(connectionNode);
			}
			k++;
		}

		Style pathStyle, pathStyle_surface;
		pathStyle.getOrCreate<LineSymbol>()->stroke()->color() = Color::Yellow;
		pathStyle.getOrCreate<LineSymbol>()->stroke()->width() = 3.0f;
		pathStyle.getOrCreate<AltitudeSymbol>()->clamping() = AltitudeSymbol::CLAMP_ABSOLUTE;
		pathStyle_surface.getOrCreate<LineSymbol>()->stroke()->color() = Color::Yellow;
		pathStyle_surface.getOrCreate<LineSymbol>()->stroke()->width() = 3.0f;
		pathStyle_surface.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->clamping() = osgEarth::Symbology::AltitudeSymbol::CLAMP_TO_TERRAIN;
		pathStyle_surface.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->technique() = osgEarth::Symbology::AltitudeSymbol::TECHNIQUE_GPU;

		Feature* pathFeature = new Feature(path, geoSRS, pathStyle);
		Feature* pathFeature_surface = new Feature(path_surface, geoSRS, pathStyle_surface);
		pathNode = new FeatureNode(mapNode, pathFeature);
		pathNode_surface = new FeatureNode(mapNode, pathFeature_surface);

		root->addChild(connections);
		root->addChild(pathNode);
		root->addChild(pathNode_surface);
	}
}