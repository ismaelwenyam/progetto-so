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
};


ConfigurationAdt get_config ();

#endif
