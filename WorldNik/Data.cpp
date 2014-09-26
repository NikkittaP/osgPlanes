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
void loadFlightPlan()
{
	QHashIterator<int, FlightInfo> iter(planesList);
	while (iter.hasNext())
	{
		iter.next();
		int flight_id = iter.key();
		QSqlQuery query(db);
		std::vector<FlightPlan> tmp;
		query.prepare("SELECT wpt_num, name, latitude, longitude, altitude FROM flight_plans WHERE flight_info_id = :id");
		query.bindValue(":id", flight_id);
		query.exec();
		while (query.next()) {
			FlightPlan fpln;
			fpln.WPTnum = query.value(0).toInt();
			fpln.WPTname = query.value(1).toString();
			fpln.lat = query.value(2).toFloat();
			fpln.lon = query.value(3).toFloat();
			fpln.alt = query.value(4).toFloat();

			tmp.push_back(fpln);
		}
		if (tmp.size() != 0)
		{
			flightPlans.insert(flight_id, tmp);
		}
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

			if (timestamp < flight.arrivalTime.asTimeStamp() && timestamp >= (flight.departureTime.asTimeStamp() - DBUpdateInterval))
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
					if (timestamp < flight.departureTime.asTimeStamp())
						planesCurrentPosition.insert(flight_id, planePoints[flight_id][0]);
					else
					{
						for (int i = 0; i < tmp.size(); i++)
						{
							if (timestamp == (flight.departureTime.asTimeStamp() + tmp[i].seconds))
							{
								planesCurrentPosition.insert(flight_id, planePoints[flight_id][i]);
								break;
							}
						}
					}

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
	planesOnEarth[flight_id]->setName("Flight " + std::to_string(flight_id));
	planesOnEarth[flight_id]->setPosition(GeoPoint(geoSRS, planesCurrentPosition[flight_id].lon, planesCurrentPosition[flight_id].lat, planesCurrentPosition[flight_id].alt, ALTMODE_RELATIVE));
	planesOnEarth[flight_id]->setScale(osg::Vec3(scale, scale, scale));

	osg::ref_ptr<LabelNode> _lbl = new LabelNode(mapNode, GeoPoint(geoSRS, planesCurrentPosition[flight_id].lon, planesCurrentPosition[flight_id].lat, planesCurrentPosition[flight_id].alt, ALTMODE_RELATIVE), planesList[flight_id].flight.toStdString(), labelStyle);
	osg::ref_ptr<osg::Switch> _showHideLbl = new osg::Switch();
	_showHideLbl->setName("Label " + std::to_string(flight_id));
	_showHideLbl->addChild(_lbl);
	labelsOnEarth.insert(flight_id, _showHideLbl);

	isPlanesAdded = true;
	//planesNamesGroup->addChild(labelsOnEarth[flight_id].get());
	//planesGroup->addChild(planesOnEarth[flight_id].get());
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
		std::vector<FlightPlan> tmp2;
		int prev_wpt = -1;
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

			if (atoi(arr[4].c_str()) != prev_wpt)
			{
				FlightPlan fpln;
				fpln.WPTnum = tmp2.size();
				fpln.WPTname = "WPT_" + QString::number(tmp2.size());
				fpln.lat = point.lat;
				fpln.lon = point.lon;
				fpln.alt = point.alt;

				tmp2.push_back(fpln);
				prev_wpt = atoi(arr[4].c_str());
			}

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

			FlightPlan fpln;
			fpln.WPTnum = tmp2.size();
			fpln.WPTname = "WPT_" + QString::number(tmp2.size());
			fpln.lat = tmp[tmp.size() - 1].lat;
			fpln.lon = tmp[tmp.size() - 1].lon;
			fpln.alt = tmp[tmp.size() - 1].alt;
			tmp2.push_back(fpln);
			flightPlans.insert(i, tmp2);
		}
	}
}

void DrawFlightLine(int flight_id)
{
	visualTrajectories->removeChildren(0, visualTrajectories->getNumChildren());

	Geometry* flightLine = new Polygon();
	double r = 17.0;
	double dR = r / 6371000.0;
	double brng, lat1;
	double _lat0, _lat1, _lon0, _lon1, _alt0, _alt1, _psi0, _psi1, _gamma0, _gamma1;
	double _latc, _lonc, _altc, _psic, _gammac;
	double n = 1;
	for (int i = 0; i < planePoints[flight_id].size(); i++)
	{
		_lat0 = planePoints[flight_id][i].lat;
		_lat1 = planePoints[flight_id][i + 1].lat;
		_lon0 = planePoints[flight_id][i].lon;
		_lon1 = planePoints[flight_id][i + 1].lon;
		_psi0 = planePoints[flight_id][i].psi;
		_psi1 = planePoints[flight_id][i + 1].psi;
		_alt0 = planePoints[flight_id][i].alt;
		_alt1 = planePoints[flight_id][i + 1].alt;
		_gamma0 = planePoints[flight_id][i].gamma;
		_gamma1 = planePoints[flight_id][i + 1].gamma;
		for (int k = 0; k < n; k++)
		{
			_latc = _lat0 + k*(_lat1 - _lat0) / n;
			_lonc = _lon0 + k*(_lon1 - _lon0) / n;
			_psic = _psi0 + k*(_psi1 - _psi0) / n;
			_altc = _alt0 + k*(_alt1 - _alt0) / n;
			_gammac = _gamma0 + k*(_gamma1 - _gamma0) / n;
			lat1 = _latc*osg::PI / 180.0;
			brng = osg::PI*(_psic - 90) / 180.0;
			double lat2 = asin(sin(lat1)*cos(dR) + cos(lat1)*sin(dR)* cos(brng));
			//std::cout << _altc<<"     "<<_altc + r*sin(_gammac*osg::PI / 180.0) << std::endl;
			flightLine->push_back(osg::Vec3d(
				_lonc + atan2(sin(brng)*sin(dR)*cos(lat2), cos(dR) - sin(lat1)*sin(lat2))*180.0 / osg::PI,
				lat2*180.0 / osg::PI,
				_altc - r*sin(_gammac*osg::PI / 180.0)));
		}
	}
	for (int i = planePoints[flight_id].size()-1; i > 0; i--)
	{
		_lat0 = planePoints[flight_id][i].lat;
		_lat1 = planePoints[flight_id][i - 1].lat;
		_lon0 = planePoints[flight_id][i].lon;
		_lon1 = planePoints[flight_id][i - 1].lon;
		_psi0 = planePoints[flight_id][i].psi;
		_psi1 = planePoints[flight_id][i - 1].psi;
		_alt0 = planePoints[flight_id][i].alt;
		_alt1 = planePoints[flight_id][i - 1].alt;
		_gamma0 = planePoints[flight_id][i].gamma;
		_gamma1 = planePoints[flight_id][i - 1].gamma;
		for (int k = 0; k < n; k++)
		{
			_latc = _lat0 - k*(_lat0 - _lat1) / n;
			_lonc = _lon0 - k*(_lon0 - _lon1) / n;
			_psic = _psi0 - k*(_psi0 - _psi1) / n;
			_altc = _alt0 - k*(_alt0 - _alt1) / n;
			_gammac = _gamma0 - k*(_gamma0 - _gamma1) / n;
			lat1 = _latc*osg::PI / 180.0;
			brng = osg::PI*(_psic + 90) / 180.0;
			double lat2 = asin(sin(lat1)*cos(dR) + cos(lat1)*sin(dR)* cos(brng));
			flightLine->push_back(osg::Vec3d(
				_lonc + atan2(sin(brng)*sin(dR)*cos(lat2), cos(dR) - sin(lat1)*sin(lat2))*180.0 / osg::PI,
				lat2*180.0 / osg::PI,
				_altc - r*sin(_gammac*osg::PI / 180.0)));
		}
	}
	Style flightLineStyle;
	flightLineStyle.getOrCreate<PolygonSymbol>()->fill()->color() = osg::Vec4(42.0f / 255, 122.0f / 255, 171.0f / 255, 0.3f);
	flightLineStyle.getOrCreate<PointSymbol>()->fill()->color() = osg::Vec4(205.0f / 255, 255.0f / 255, 255.0f / 255, 1.0f);
	flightLineStyle.getOrCreate<PointSymbol>()->size() = 3.0f;
	flightLineStyle.getOrCreate<LineSymbol>()->stroke()->color() = osg::Vec4(205.0f / 255, 255.0f / 255, 255.0f / 255, 1.0f);
	flightLineStyle.getOrCreate<LineSymbol>()->stroke()->width() = 2.0f;
	//flightLineStyle.getOrCreate<AltitudeSymbol>()->clamping() = AltitudeSymbol::CLAMP_NONE;
	Feature* connectionFeature = new Feature(flightLine, geoSRS, flightLineStyle);
	FeatureNode* connectionNode = new FeatureNode(mapNode, connectionFeature);
	visualTrajectories->addChild(connectionNode);
	
	
	
	/*
	Geometry* path = new LineString();
	Geometry* path_surface = new LineString();
	for (int i = 0; i < planePoints[flight_id].size(); i++)
	{
	path->push_back(osg::Vec3d(planePoints[flight_id][i].lon, planePoints[flight_id][i].lat, planePoints[flight_id][i].alt));
	path_surface->push_back(osg::Vec3d(planePoints[flight_id][i].lon, planePoints[flight_id][i].lat, 0));

	Geometry* connection = new LineString();
	connection->push_back(osg::Vec3d(planePoints[flight_id][i].lon, planePoints[flight_id][i].lat, planePoints[flight_id][i].alt));
	connection->push_back(osg::Vec3d(planePoints[flight_id][i].lon, planePoints[flight_id][i].lat, 0));
	Style connectionStyle;
	connectionStyle.getOrCreate<LineSymbol>()->stroke()->color() = Color::Yellow;
	connectionStyle.getOrCreate<LineSymbol>()->stroke()->width() = 3.0f;
	connectionStyle.getOrCreate<AltitudeSymbol>()->clamping() = AltitudeSymbol::CLAMP_NONE;
	Feature* connectionFeature = new Feature(connection, geoSRS, connectionStyle);
	FeatureNode* connectionNode = new FeatureNode(mapNode, connectionFeature);

	visualTrajectories->addChild(connectionNode);
	}

	Style pathStyle, pathStyle_surface;
	pathStyle.getOrCreate<LineSymbol>()->stroke()->color() = Color::Yellow;
	pathStyle.getOrCreate<LineSymbol>()->stroke()->width() = 3.0f;
	pathStyle.getOrCreate<AltitudeSymbol>()->clamping() = AltitudeSymbol::CLAMP_RELATIVE_TO_TERRAIN;
	pathStyle_surface.getOrCreate<LineSymbol>()->stroke()->color() = Color::Yellow;
	pathStyle_surface.getOrCreate<LineSymbol>()->stroke()->width() = 3.0f;
	pathStyle_surface.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->clamping() = osgEarth::Symbology::AltitudeSymbol::CLAMP_TO_TERRAIN;
	pathStyle_surface.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->technique() = osgEarth::Symbology::AltitudeSymbol::TECHNIQUE_GPU;

	Feature* pathFeature = new Feature(path, geoSRS, pathStyle);
	Feature* pathFeature_surface = new Feature(path_surface, geoSRS, pathStyle_surface);
	pathFeature->geoInterp() = GEOINTERP_GREAT_CIRCLE;
	pathFeature_surface->geoInterp() = GEOINTERP_GREAT_CIRCLE;
	visualTrajectories->addChild(new FeatureNode(mapNode, pathFeature));
	visualTrajectories->addChild(new FeatureNode(mapNode, pathFeature_surface));
	*/
}