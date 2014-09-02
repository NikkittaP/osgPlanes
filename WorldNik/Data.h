#ifndef Data_HEADER
#define Data_HEADER

void loadAirports();
void loadPlanesList();
void loadPlanesPoints(double timestamp);
void addPlanesToEarth(int flight_id);

void parseFlightFile();

void DrawFlightLines(int id);

#endif