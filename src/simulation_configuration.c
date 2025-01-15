#include <stdio.h>
#include <stdlib.h>

#include "simerr.h"
#include "simulation_configuration.h"
#include "semapi.h"


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
		if (sscanf(line, "%s %s", key, value) != 2){
			fprintf(stderr, "syntax error in config file %s at line %d\n", "config_sim.conf", lineNum);
			err_exit("error in configuration file");
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

int update_timeout(int semId, int d){
	if (reserve_sem(semId, 0) == -1){
		printf("error reserving config sem\n");
		err_exit(strerror(errno));
	}

	FILE *configFile = fopen("config_timout.conf", "w+");
	if (configFile == NULL){
		printf("error in opening config_timout file\n");
		return -1;
	}
	fprintf(configFile, "%s %d\n", "timeout", d);

	fclose(configFile);
	if (release_sem(semId, 0) == -1){
		printf("error releasing config sem\n");
		err_exit(strerror(errno));
	}
	return 0;
}

int update_explode(int semId, int count){
	if (reserve_sem(semId, 0) == -1){
		printf("error reserving config sem\n");
		err_exit(strerror(errno));
	}
	
	FILE *configFile = fopen("config_explode.conf", "r");
	if (configFile == NULL){
		printf("error in opening config_explode file\n");
		return -1;
	}

	int explode = 0;
	char line[256];
	int lineNum = 0;
	while (fgets(line, 256, configFile) != NULL){
		char key[256], value[256];
		lineNum++;
		if (line[0] == '#') continue;
		if (sscanf(line, "%s %s", key, value) != 2){
			fprintf(stderr, "syntax error in config file %s at line %d\n", "explode_config.conf", lineNum);
			return -1;
		}
		explode = atoi(value);
	}
	explode += count;
	fprintf(configFile, "%s %d\n", "explode", explode);

	fclose(configFile);
	if (release_sem(semId, 0) == -1){
		printf("error releasing config sem\n");
		err_exit(strerror(errno));
	}
	return 0;
}

int get_timeout (int semId){
	if (reserve_sem(semId, 0) == -1){
		printf("error reserving config sem\n");
		err_exit(strerror(errno));
	}

	FILE *configFile = fopen("config_timout.conf", "r");
	if (configFile == NULL){
		printf("error in opening config_timout file\n");
		return -1;
	}
	int timeout = 0;
	char line[256];
	int lineNum = 0;
	while (fgets(line, 256, configFile) != NULL){
		char key[256], value[256];
		lineNum++;
		if (line[0] == '#') continue;
		if (sscanf(line, "%s %s", key, value) != 2){
			fprintf(stderr, "syntax error in config file %s at line %d\n", "timout_config.conf", lineNum);
			err_exit("error in configuration file");
		}
		timeout = atoi(value);
	}

	fclose(configFile);
	if (release_sem(semId, 0) == -1){
		printf("error releasing config sem\n");
		err_exit(strerror(errno));
	}
	return timeout;
}


int get_explode (int semId){
	if (reserve_sem(semId, 0) == -1){
		printf("error reserving config sem\n");
		err_exit(strerror(errno));
	}
	
	FILE *configFile = fopen("config_explode.conf", "r");
	if (configFile == NULL){
		printf("error in opening config_explode file\n");
		return -1;
	}

	int explode = 0;
	char line[256];
	int lineNum = 0;
	while (fgets(line, 256, configFile) != NULL){
		char key[256], value[256];
		lineNum++;
		if (line[0] == '#') continue;
		if (sscanf(line, "%s %s", key, value) != 2){
			fprintf(stderr, "syntax error in config file %s at line %d\n", "explode_config.conf", lineNum);
			return -1;
		}
		explode = atoi(value);
	}
	fclose(configFile);
	if (release_sem(semId, 0) == -1){
		printf("error releasing config sem\n");
		err_exit(strerror(errno));
	}
	return explode;
}
