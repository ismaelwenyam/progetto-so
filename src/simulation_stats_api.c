#include <stdio.h>
#include <string.h>


#include "simulation_stats.h"
#include "semapi.h"
#include "shmapi.h"

int init_statistics (int shmId, int semId, const char **services){
	printf("start init_statistics\n");
	//reserve semaforo per accesso ad shm statistiche
	if (reserve_sem(semId, 0) == -1){
		printf("failed to stats shm sem\n");
		return -1;
	}
	//accesso a statistiche 
	StatisticsAdtPtr statisticsPtr = shmat(shmId, NULL, SHM_RND);
	if (statisticsPtr == (void*)-1){
		printf("failed to attach stats shm\n");
		return -1;
	}
	//inizializzazione delle statistichie
	statisticsPtr->activeOperatorsDaily = 0;
	statisticsPtr->activeOperatorsSimulation = 0;
	statisticsPtr->averageBreaksDaily = 0.0;
	statisticsPtr->totalBreaksSimulation = 0;
	statisticsPtr->operatorToCounterRatio = 0;
	//TODO inizializzare le statistiche per servizio
	for (int i = 0; i < SERVICES; i++){
		strcpy(statisticsPtr->services[i].serviceType, services[i]);	
		statisticsPtr->services[i].totalUsersServed = 0;
		statisticsPtr->services[i].averageDailyUsersServed = 0.0;
		statisticsPtr->services[i].totalUsersServed = 0;
		statisticsPtr->services[i].totalServicesProvided = 0;
		statisticsPtr->services[i].totalServicesNotProvided = 0;
		statisticsPtr->services[i].averageDailyServicesProvided = 0.0;
		statisticsPtr->services[i].averageDailyServicesNotProvided = 0.0;
		statisticsPtr->services[i].averageWaitingTimeSimulation = 0.0;
		statisticsPtr->services[i].averageServiceDurationDaily = 0.0;
	}
	//release semaforo accesso statistiche
	if (release_sem(semId, 0) == -1){
		printf("failed to release stats shm sem\n");
		return -1;
	}
	//detach da statistiche
	if (shmdt(statisticsPtr) == -1){
		printf("failed to detach stats shm\n");
		return -1;
	}
	printf("end init_statistics\n");
	return 0;
}

int print_stats (int shmId, int semId, int day){
	if (reserve_sem(semId, 0) == -1){
		printf("failed to reserve stats shm sem\n");
		return -1;
	}
	StatisticsAdtPtr statisticsPtr = shmat(shmId, NULL, SHM_RND);
	if (statisticsPtr == (void*)-1){
		printf("failed to attach stats shm\n");
		return -1;
	}
	printf("**********************************************************************************\n");
	printf("\t\t\t\t\t\tDAILY STATS - day: %d\n", day);
	printf("**********************************************************************************\n");
	for (int i = 0; i < SERVICES; i++){
		printf("SERVICE: %s\n", statisticsPtr->services[i].serviceType);	
		printf("%s %s %s %s %s\n", "average_daily_users_served", "average_daily_services_provided", 
									"average_daily_services_not_provided", 
									"average_waiting_time_daily", 
									"average_service_duration_daily");	
		printf("%26.2f %31.2f %34.2f %26.2f %30.2f\n", statisticsPtr->services[i].averageDailyUsersServed, 
						statisticsPtr->services[i].averageDailyServicesProvided, 
						statisticsPtr->services[i].averageDailyServicesNotProvided,
						statisticsPtr->services[i].averageWaitingTimeDaily, 
						statisticsPtr->services[i].averageServiceDurationDaily);
	}
	printf("**********************************************************************************\n");
	printf("\t\t\t\t\t\tEXTRA DAILY STATS - day: %d\n", day);
	printf("**********************************************************************************\n");
	printf("%s %s %s\n", "active_operators_daily", "average_breaks_daily", "operator_to_counter_ratio");
	printf("%22d %20.1f %25.1f\n", statisticsPtr->activeOperatorsDaily,
					statisticsPtr->averageBreaksDaily, statisticsPtr->operatorToCounterRatio);
	
	printf("**********************************************************************************\n");
	printf("\t\t\t\t\t\tTOTAL STATS - day: %d\n", day);
	printf("**********************************************************************************\n");
	for (int i = 0; i < SERVICES; i++){
		printf("SERVICE: %s\n", statisticsPtr->services[i].serviceType);	
		printf("%s %s %s %s %s\n", "tot_users_served", "tot_services_provided", 
									"tot_services_not_provided", 
									"average_waiting_time_simulation", 
									"average_service_duration_simulation");	
		printf("%16d %21d %25d %31.2f %36.2f\n", statisticsPtr->services[i].totalUsersServed, 
						statisticsPtr->services[i].totalServicesProvided, 
						statisticsPtr->services[i].totalServicesNotProvided,
						statisticsPtr->services[i].averageWaitingTimeSimulation, 
						statisticsPtr->services[i].averageServiceDurationSimulation);
	}
	printf("**********************************************************************************\n");
	printf("\t\t\t\t\t\tEXTRA TOTAL STATS - day: %d\n", day);
	printf("**********************************************************************************\n");
	printf("%s %s\n", "active_operators_simulation", "total_breaks_simulation");
	printf("%27d %23d\n", statisticsPtr->activeOperatorsSimulation,
					statisticsPtr->totalBreaksSimulation);
	//TODO stampare statistiche per servizio 
	if (release_sem(semId, 0) == -1){
		printf("failed to release stats shm sem\n");
		return -1;
	}
	//detach da statistiche
	if (shmdt(statisticsPtr) == -1){
		printf("failed to detach stats shm\n");
		return -1;
	}
	return 0;
}

int reset_statistics (int shmId, int semId){
	printf("start reset_statistics");
	//reserve semaforo per accesso ad shm statistiche
	if (reserve_sem(semId, 0) == -1){
		printf("failed to reserve stats shm sem\n");
		return -1;
	}
	//accesso a statistiche 
	StatisticsAdtPtr statisticsPtr = shmat(shmId, NULL, SHM_RND);
	if (statisticsPtr == (void*)-1){
		printf("failed to attach stats shm\n");
		return -1;
	}
	//inizializzazione delle statistichie
	statisticsPtr->activeOperatorsDaily = 0;
	statisticsPtr->averageBreaksDaily = 0.0;
	statisticsPtr->operatorToCounterRatio = 0;
	//TODO inizializzare le statistiche per servizio
	//release semaforo accesso statistiche
	if (release_sem(semId, 0) == -1){
		printf("failed to release stats shm sem\n");
		return -1;
	}
	//detach da statistiche
	if (shmdt(statisticsPtr) == -1){
		printf("failed to detach stats shm\n");
		return -1;
	}
	printf("end reset_statistics\n");
	return 0;
}


int add_operator_to_gen_stat (int shmId, int semId){
	printf("start add_operator_to_gen_stat\n");	
	if (reserve_sem(semId, 0) == -1){
		printf("failed to reserve stats shm sem\n");
		return -1;
	}
	StatisticsAdtPtr statisticsPtr = shmat(shmId, NULL, SHM_RND);
	if (statisticsPtr == (void*)-1){
		printf("failed to attach stats shm\n");
		return -1;
	}
	statisticsPtr->activeOperatorsDaily += 1;
	statisticsPtr->activeOperatorsSimulation += 1;
	if (release_sem(semId, 0) == -1){
		printf("failed to release stats shm sem\n");
		return -1;
	}
	if (shmdt(statisticsPtr) == -1){
		printf("failed to detach stats shm\n");
		return -1;
	}
	printf("end add_operator_to_gen_stat\n");	
	return 0;
}
 int add_pause_to_gen_stat (int shmId, int semId){
	printf("start add_pause_to_gen_stat\n");	
	if (reserve_sem(semId, 0) == -1){
		printf("failed to reserve stats shm sem\n");
		return -1;
	}
	StatisticsAdtPtr statisticsPtr = shmat(shmId, NULL, SHM_RND);
	if (statisticsPtr == (void*)-1){
		printf("failed to attach stats shm\n");
		return -1;
	}
	statisticsPtr->totalBreaksSimulation += 1;
	if (release_sem(semId, 0) == -1){
		printf("failed to release stats shm sem\n");
		return -1;
	}
	if (shmdt(statisticsPtr) == -1){
		printf("failed to detach stats shm\n");
		return -1;
	}
	printf("end add_pause_to_gen_stat\n");	
	return 0;
}


int update_service_stat (int shmId, int semId, char *service, int serviceProvided){
	printf("start add_service_provided\n");
	if (reserve_sem(semId, 0) == -1){
		printf("failed to reserve stats shm sem\n");
		return -1;
	}
	//accesso a statistiche 
	StatisticsAdtPtr statisticsPtr = shmat(shmId, NULL, SHM_RND);
	if (statisticsPtr == (void*)-1){
		printf("failed to attach stats shm\n");
		return -1;
	}

	for (int i = 0; i < SERVICES; i++){
		if (strcmp(statisticsPtr->services[i].serviceType, service) == 0){
			if (serviceProvided){
				statisticsPtr->services[i].totalServicesProvided += 1;	
			}else{
				statisticsPtr->services[i].totalServicesNotProvided += 1;	
			}
			break;	
		}	
	}


	//release semaforo accesso statistiche
	if (release_sem(semId, 0) == -1){
		printf("failed to release stats shm sem\n");
		return -1;
	}
	//detach da statistiche
	if (shmdt(statisticsPtr) == -1){
		printf("failed to detach stats shm\n");
		return -1;
	}
	printf("end add_service_provided\n");
	return 0;
}


int update_user_served_stat (int shmId, int semId, char *service){
	printf("start update_user_served_stat\n");
	if (reserve_sem(semId, 0) == -1){
		printf("failed to reserve stats shm sem\n");
		return -1;
	}
	//accesso a statistiche 
	StatisticsAdtPtr statisticsPtr = shmat(shmId, NULL, SHM_RND);
	if (statisticsPtr == (void*)-1){
		printf("failed to attach stats shm\n");
		return -1;
	}

	for (int i = 0; i < SERVICES; i++){
		if (strcmp(statisticsPtr->services[i].serviceType, service) == 0){
			statisticsPtr->services[i].totalUsersServed += 1;	
			break;	
		}	
	}


	//release semaforo accesso statistiche
	if (release_sem(semId, 0) == -1){
		printf("failed to release stats shm sem\n");
		return -1;
	}
	//detach da statistiche
	if (shmdt(statisticsPtr) == -1){
		printf("failed to detach stats shm\n");
		return -1;
	}
	printf("end update_user_served_stat\n");
	return 0;
}
