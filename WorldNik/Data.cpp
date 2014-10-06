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

	osg::AutoTransform* at = new osg::AutoTransform;
	at->addChild(plane_high_.get());
	at->setAutoRotateMode(osg::AutoTransform::NO_ROTATION);
	at->setAutoScaleToScreen(true);
	at->setMinimumScale(1);
	at->getOrCreateStateSet()->setMode(GL_NORMALIZE, osg::StateAttribute::ON);

	style.getOrCreate<ModelSymbol>()->setModel(at);

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

void compute_intervals(int *u, int n, int t)   // figure out the knots 
{
	int j;

	for (j = 0; j <= n + t; j++)
	{
		if (j<t)
			u[j] = 0;
		else
		if ((t <= j) && (j <= n))
			u[j] = j - t + 1;
		else if (j>n)
			u[j] = n - t + 2;  // if n-t=-2 then we're screwed, everything goes to 0 
	}
}

double blend(int k, int t, int *u, double v)  // calculate the blending value 
{
	double value;

	if (t == 1)	 // base case for the recursion 
	{
		if ((u[k] <= v) && (v < u[k + 1]))
			value = 1;
		else
			value = 0;
	}
	else
	{
		if ((u[k + t - 1] == u[k]) && (u[k + t] == u[k + 1]))  // check for divide by zero 
			value = 0;
		else
		if (u[k + t - 1] == u[k]) // if a term's denominator is zero,use just the other 
			value = (u[k + t] - v) / (u[k + t] - u[k + 1]) * blend(k + 1, t - 1, u, v);
		else
		if (u[k + t] == u[k + 1])
			value = (v - u[k]) / (u[k + t - 1] - u[k]) * blend(k, t - 1, u, v);
		else
			value = (v - u[k]) / (u[k + t - 1] - u[k]) * blend(k, t - 1, u, v) +
			(u[k + t] - v) / (u[k + t] - u[k + 1]) * blend(k + 1, t - 1, u, v);
	}
	return value;
}

void compute_point(int *u, int n, int t, double v, osg::Vec3dArray *control, osg::Vec3d &outPoint)
{
	int k;
	double temp;
	// initialize the variables that will hold our outputted point 
	outPoint = osg::Vec3d(0, 0, 0);
	for (k = 0; k <= n; k++)
	{
		temp = blend(k, t, u, v);  // same blend is used for each dimension coordinate 
		outPoint = outPoint + control->at(k) * temp;
	}
}

/*
bspline:
n          - the number of control points minus 1
t          - the degree of the polynomial plus 1
control    - control point array made up of point stucture
output     - array in which the calculate spline points are to be put

Pre-conditions:
n+2>t  (no curve results if n+2<=t)
control array contains the number of points specified by n
output array is the proper size to hold num_output point structures
*/

void bspline(int n, int t, osg::Vec3dArray *control, osg::Vec3dArray *output)
{
	int num_output = output->size();
	int *u;
	double increment, interval;
	osg::Vec3d calcxyz;
	int output_index;

	u = new int[n + t + 1];
	compute_intervals(u, n, t);

	increment = (double)(n - t + 2) / (num_output - 1);  // how much parameter goes up each time 
	interval = 0;

	for (output_index = 0; output_index < num_output - 1; output_index++)
	{
		compute_point(u, n, t, interval, control, calcxyz);
		output->at(output_index) = calcxyz;
		interval = interval + increment;  // increment our parameter 
	}
	output->at(num_output - 1) = control->at(n);
	delete[]u;
}

void DrawFlightLine(int flight_id)
{
	visualTrajectories->removeChildren(0, visualTrajectories->getNumChildren());

	Geometry* flightLine = new Polygon();

	double r = 17.0;
	double dR = r / 6371000.0;
	double brng;
	double _lat, _lon, _alt, _psi, _gamma;
	double n = 3;
	osg::Vec3dArray* control = new osg::Vec3dArray();
	osg::Vec3d target_vec;
	for (int i = 0; i < planePoints[flight_id].size() - 1; i++)
	{
		_lat = planePoints[flight_id][i].lat;
		_lon = planePoints[flight_id][i].lon;
		_psi = planePoints[flight_id][i].psi;
		_alt = planePoints[flight_id][i].alt;
		_gamma = planePoints[flight_id][i].gamma;

		_lat = _lat*osg::PI / 180.0;
		brng = osg::PI*(_psi - 90) / 180.0;
		double lat2 = asin(sin(_lat)*cos(dR) + cos(_lat)*sin(dR)* cos(brng));

		mapNode->getMap()->getSRS()->getEllipsoid()->convertLatLongHeightToXYZ(
			lat2,
			osg::DegreesToRadians(_lon) + atan2(sin(brng)*sin(dR)*cos(lat2), cos(dR) - sin(_lat)*sin(lat2)),
			_alt + r*tan(_gamma*osg::PI / 180.0),
			target_vec.x(), target_vec.y(), target_vec.z());
		control->push_back(target_vec);

	}
	osg::Vec3dArray* spline = new osg::Vec3dArray(control->size() * n);
	bspline(control->size() - 1, n, control, spline);
	for (int i = 0; i < spline->size() - 1; i++)
	{
		osg::Vec3d src_vec = spline->asVector()[i];
		double lat, lon, alt;
		mapNode->getMap()->getSRS()->getEllipsoid()->convertXYZToLatLongHeight(src_vec.x(), src_vec.y(), src_vec.z(), lat, lon, alt);
		flightLine->push_back(osg::Vec3d(osg::RadiansToDegrees(lon), osg::RadiansToDegrees(lat), alt));
	}

	control = new osg::Vec3dArray();
	for (int i = planePoints[flight_id].size() - 1; i > 0; i--)
	{
		_lat = planePoints[flight_id][i].lat;
		_lon = planePoints[flight_id][i].lon;
		_psi = planePoints[flight_id][i].psi;
		_alt = planePoints[flight_id][i].alt;
		_gamma = planePoints[flight_id][i].gamma;

		_lat = _lat*osg::PI / 180.0;
		brng = osg::PI*(_psi + 90) / 180.0;
		double lat2 = asin(sin(_lat)*cos(dR) + cos(_lat)*sin(dR)* cos(brng));

		mapNode->getMap()->getSRS()->getEllipsoid()->convertLatLongHeightToXYZ(
			lat2,
			osg::DegreesToRadians(_lon) + atan2(sin(brng)*sin(dR)*cos(lat2), cos(dR) - sin(_lat)*sin(lat2)),
			_alt - r*tan(_gamma*osg::PI / 180.0),
			target_vec.x(), target_vec.y(), target_vec.z());
		control->push_back(target_vec);
	}
	spline = new osg::Vec3dArray(control->size() * n);
	bspline(control->size() - 1, n, control, spline);
	for (int i = 0; i < spline->size() - 1; i++)
	{
		osg::Vec3d src_vec = spline->asVector()[i];
		double lat, lon, alt;
		mapNode->getMap()->getSRS()->getEllipsoid()->convertXYZToLatLongHeight(src_vec.x(), src_vec.y(), src_vec.z(), lat, lon, alt);
		flightLine->push_back(osg::Vec3d(osg::RadiansToDegrees(lon), osg::RadiansToDegrees(lat), alt));
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
}