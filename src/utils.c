#include <stdio.h>
#include <sys/types.h>

#include "simerr.h"
#include "service_api.h"
#include "sportello_api.h"
#include "shmapi.h"
#include "semapi.h"
#include "msgapi.h"



void print_services_in_shm (int shmId, int semId){
	//printf("start print_services_in_shm\n");
	if (reserve_sem(semId, 0) == -1){
		printf("error in reserving sem\n");
		err_exit(strerror(errno));
	}

	ServizioAdtPtr servicesPtr = shmat(shmId, NULL, SHM_RND);
	if (servicesPtr == (void*)-1){
		printf("error in shmat\n");
		err_exit(strerror(errno));
	}
	printf("%s %s %s\n", "service", "temp", "available");
	for (int i = 0; i < NUMBER_OF_SERVICES; i++){
		printf("%7s%5d%10s\n", servicesPtr[i].name, servicesPtr[i].temp, servicesPtr[i].available ? "true" : "false");

	}
	
	if (release_sem(semId, 0) == -1){
		printf("error in releasing sem\n");
		err_exit(strerror(errno));
	}
	if (shmdt(servicesPtr) == -1){
		printf("error in shmdt\n");
		err_exit(strerror(errno));
	}
	//printf("end print_services_in_shm\n");
}

void print_sportelli_in_shm (int shmId, int semId, int nofWorkerSeats){
	//printf("start print_sportelli_in_shm\n");
	if (reserve_sem(semId, 0) == -1){
		printf("error in reserving sem\n");
		err_exit(strerror(errno));
	}
	SportelloAdtPtr sportelliPtr = shmat(shmId, NULL, SHM_RND);
	if (sportelliPtr == (void*)-1){
		printf("shmat failed\n");
		err_exit(strerror(errno));
	}
	printf("%s %s %s %s %s %s %s %s\n", "service", "sportello_pid", "operator_pid", "available", "deskSemId", "deskSemun", "workerDeskSemId", "workerDeskSemun");
	for (int i = 0; i < nofWorkerSeats; i++){
		printf("%7s%14d%12d%10s%10d%10d%16d%16d\n", sportelliPtr[i].serviceName, sportelliPtr[i].sportelloPid, sportelliPtr[i].operatorPid, sportelliPtr[i].deskAvailable ? "true" : "false", sportelliPtr[i].deskSemId, sportelliPtr[i].deskSemun, sportelliPtr[i].workerDeskSemId, sportelliPtr[i].workerDeskSemun);

	}

	if (release_sem(semId, 0) == -1){
		printf("error in releasing sem\n");
		err_exit(strerror(errno));
	}

	if (shmdt(sportelliPtr) == -1){
		printf("error in shmdt\n");
		err_exit(strerror(errno));
	}
	//printf("end print_sportelli_in_shm\n");
}


void print_statitics (int shmId, int semId){




}


void dump_data (char *filename){





}


int delete_ipc_resources (int id, char *resourceType){
	if (resourceType == NULL){
		return -1;
	}
	if (strcmp(resourceType, "shm") == 0){
		return rm_shm(id);	
	} else if (strcmp(resourceType, "sem") == 0){
		return rm_sem(id);	
	} else {
		return rm_msgq(id);	
	}	
	return 0;

}

int rm_msgq(int id){
	return msgctl(id, IPC_RMID, NULL);
}

int rm_shm(int id){
	return shmctl(id, IPC_RMID, NULL);
}

int rm_sem(int id){
	return semctl(id, 0, IPC_RMID);
}
