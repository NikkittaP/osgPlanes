#ifndef Data_HEADER
#define Data_HEADER

void connectToDB();
void loadAirports();
void loadPlanesList();
void loadFlightPlan();
void loadPlanesPoints(double timestamp);
void addPlanesToEarth(int flight_id);

void parseFlightFile();

void DrawFlightLine(int flight_id);

#endif