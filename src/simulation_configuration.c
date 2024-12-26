#include <stdio.h>
#include <stdlib.h>

#include "simerr.h"
#include "simulation_configuration.h"


ConfigurationAdt get_config(){
	ConfigurationAdt configuration;

	FILE *configurationFile = fopen("simulation_configuration.conf", "r");
	if (configurationFile == NULL){
		printf("error in opening configuration file\n");
		err_exit(strerror(errno));
	}

	char line[256];
	int lineNum = 0;
	while (fgets(line, 256, configurationFile) != NULL){
		char key[256], value[256];
		lineNum++;
		if (line[0] == '#') continue;
		if (sscanf(line, "%s" "%s", key, value) != 2){
			fprintf(stderr, "syntax error in config file %s at line %d\n", "config_sim.conf", lineNum);
			//return NULL;
		}
		if (strcmp(key, "nof_worker_seats") == 0){
			configuration.nofWorkerSeats = atoi(value);	
		} else if (strcmp(key, "nof_workers") == 0) {
			configuration.nofWorkers = atoi(value);	
		} else if (strcmp(key, "nof_users") == 0) {	
			configuration.nofUsers = atoi(value);	
		} else if (strcmp(key, "sim_duration") == 0) {	
			configuration.simDuration = atoi(value);	
		} else if (strcmp(key, "n_nano_secs") == 0) {	
			configuration.nNanoSecs = atoi(value);	
		} else if (strcmp(key, "nof_pause") == 0) {	
			configuration.nofPause = atoi(value);	
		} else if (strcmp(key, "p_serv_min") == 0) {	
			configuration.pServMin = atoi(value);	
		} else if (strcmp(key, "p_serv_max") == 0) {	
			configuration.pServMax = atoi(value);	
		} else if (strcmp(key, "explode_threshold") == 0) {
			configuration.explodeThreshold = atoi(value);	
		} else if (strcmp(key, "n_requests") == 0) {
			configuration.nRequests = atoi(value);
		} else {
			configuration.nNewUsers = atoi(value);
		}
	}
		
	fclose(configurationFile);
	return configuration;
}
