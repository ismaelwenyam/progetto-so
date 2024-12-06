#include <stdio.h>
#include <stdlib.h>

#include "simerr.h"
#include "config_sim.h"

ConfigurationAdtPtr init_config(const char const *configPath){
	printf("init_config\n");
	if (configPath == NULL) return NULL;	

	FILE *configurationFile = fopen(configPath, "r");
	if (configurationFile == NULL) return NULL;

	ConfigurationAdtPtr config = malloc(sizeof(ConfigurationAdt));
	if (config == NULL) return NULL;

	char line[256];
	int lineNum = 0;
	while (fgets(line, 256, configurationFile) != NULL){
		char key[256], value[256];
		lineNum++;
		if (line[0] == '#') continue;
		if (sscanf(line, "%s" "%s", key, value) != 2){
			fprintf(stderr, "syntax error in config file %s at line %d\n", configPath, lineNum);
			return NULL;
		}
		if (strcmp(key, "nof_worker_seats") == 0){
			config->nofWorkerSeats = atoi(value);	
		} else if (strcmp(key, "nof_workers") == 0) {
			config->nofWorkers = atoi(value);	
		} else if (strcmp(key, "nof_users") == 0) {	
			config->nofUsers = atoi(value);	
		} else if (strcmp(key, "sim_duration") == 0) {	
			config->simDuration = atoi(value);	
		} else if (strcmp(key, "n_nano_secs") == 0) {	
			config->nNanoSecs = atoi(value);	
		} else if (strcmp(key, "nof_pause") == 0) {	
			config->nofPause = atoi(value);	
		} else if (strcmp(key, "p_serv_min") == 0) {	
			config->pServMin = atoi(value);	
		} else if (strcmp(key, "p_serv_max") == 0) {	
			config->pServMax = atoi(value);	
		} else {
			config->explodeThreshold = atoi(value);	
		}
	}
		
	fclose(configurationFile);
	return config;
}
