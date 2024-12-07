#ifndef CONFIG_SIM_H
#define CONFIG_SIM_H
typedef struct configuration ConfigurationAdt, *ConfigurationAdtPtr;
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
};


ConfigurationAdt get_config ();

#endif
