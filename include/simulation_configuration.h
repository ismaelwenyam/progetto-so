#ifndef SIMULATION_CONFIG_H
#define SIMULATION_CONFIG_H
typedef struct configuration ConfigurationAdt;
struct configuration {
	int nofWorkerSeats;
	int nofWorkers;
	int nofUsers;
	int simDuration;
	int nNanoSecs;
	int nofPause;
	int pServMin;
	int pServMax;
	int explodeThreshold;
	int nRequests;
	int nNewUsers;
	int timeout;
	int explode;
};


ConfigurationAdt get_config ();
int update_timeout (int semId, int d);
int update_explode (int semId, int count);
int get_timeout (int semId);
int get_explode (int semId);
int reset_explode(int semId);

#endif
