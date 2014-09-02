#include "stdafx.h"
#include "Variables.h"
#include "Data.h"

using namespace osgEarth;
using namespace osgEarth::Annotation;
using namespace osgEarth::Util;

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

void loadPlanesPoints()
{
	for (int i = 1; i < 800; i++)
	{
		planesInTheSky.push_back(i);
		planeCurrentIndex.insert(i, 0);

		QSqlQuery query;
		std::vector<FlightPoint> tmp;
		query.prepare("SELECT second, latitude, longitude, altitude, psi, theta, gamma FROM flight_points WHERE flight_info_id = :id");
		query.bindValue(":id", i);
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

		planePoints.insert(i, tmp);

		planesCurrentPosition[i] = planePoints[i][0];

		std::cout << i << " out of 800 is loaded." << std::endl;
	}
}

/*void parseFlightFile()
{
std::vector<std::string> files_list;
DIR *dir;
struct dirent *ent;
//if ((dir = opendir("D:/Progs/OpenGL/OSG/my/2014-06-09/WorldNik/x64/Release/trajectories/")) != NULL) {
std::string _path = PATH + "trajectories/";
int count = 0;
if ((dir = opendir(_path.c_str())) != NULL) {
while ((ent = readdir(dir)) != NULL) {
count++;
if (count == 100)
break;
std::string file = ent->d_name;
if (file != "." && file != "..")
{
std::string path = _path;
path.append(file);
files_list.push_back(path);
}
}
closedir(dir);
}

//for (int i = 0; i < 900; i++)
//files_list.push_back("D:/Progs/OpenGL/OSG/my/2014-06-09/WorldNik/x64/Release/traj_full_" + ZeroPadNumber(i) + ".txt");
//	files_list.push_back("D:/Progs/OpenGL/OSG/my/2014-06-09/WorldNik/x64/Release/traj" + ZeroPadNumber(i) + ".txt");

numPlanes = files_list.size();

for (int i = 0; i < numPlanes; i++)
{
std::vector<float> _a;
std::vector<float> _seconds;
std::vector<float> _lat;
std::vector<float> _lon;
std::vector<float> _alt;
std::vector<float> _psi;
std::vector<float> _tetta;
std::vector<float> _gamma;

std::ifstream infile(files_list[i]);
std::string line;
while (std::getline(infile, line))
{
std::string arr[17];
int i = 0;
std::stringstream ssin(line);
while (ssin.good() && i < 17){
ssin >> arr[i];
++i;
}
_seconds.push_back(atof(arr[0].c_str()));
_lat.push_back(atof(arr[5].c_str()));
_lon.push_back(atof(arr[6].c_str()));
_alt.push_back(atof(arr[1].c_str())*0.3048);
double psi = atof(arr[3].c_str());
//if (psi>180)
//	psi -= 360;
_psi.push_back(psi);
_tetta.push_back(atof(arr[15].c_str()));
_gamma.push_back(atof(arr[16].c_str()));
}
if (_seconds.size() == 0)
numPlanes--;
else
{
seconds.push_back(_seconds);
lat.push_back(_lat);
lon.push_back(_lon);
alt.push_back(_alt);
psi.push_back(_psi);
tetta.push_back(_tetta);
gamma.push_back(_gamma);

planeT.push_back(_seconds[_planeIndex]);
planeLat.push_back(_lat[_planeIndex]);
planeLon.push_back(_lon[_planeIndex]);
planeAlt.push_back(_alt[_planeIndex]);
planeTheta.push_back(_tetta[_planeIndex]);
planeGamma.push_back(_gamma[_planeIndex]);
planePsi.push_back(_psi[_planeIndex]);
}
}
}*/

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