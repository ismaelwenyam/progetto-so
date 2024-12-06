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

extern ConfigurationAdt configuration;

ConfigurationAdtPtr init_config (const char const *configPath);
void ds_config (ConfigurationAdtPtr configPtr);

#endif
