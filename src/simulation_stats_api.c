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
		statisticsPtr->services[i].averageWaitingTimeDaily = 0.0;
		statisticsPtr->services[i].averageWaitingTimeSimulation = 0.0;
		statisticsPtr->services[i].averageServiceDurationDaily = 0.0;
		statisticsPtr->services[i].usersServedDaily = 0;
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

int print_stats (int shmId, int semId, int ssshmId, int sssemId, int day, int nofWorkerSeats){
	if (reserve_sem(semId, 0) == -1){
		printf("failed to reserve stats shm sem\n");
		return -1;
	}
	StatisticsAdtPtr statisticsPtr = shmat(shmId, NULL, SHM_RND);
	if (statisticsPtr == (void*)-1){
		printf("failed to attach stats shm\n");
		return -1;
	}

	if (reserve_sem(sssemId, 0) == -1){
		printf("failed to reserve sportelli stats shm sem\n");
		return -1;
	}
	SportelloStatAdtPtr sportelliStatsPtr = shmat(ssshmId, NULL, SHM_RND);
	if (sportelliStatsPtr == (void*)-1){
		printf("failed to attach sportelli stats shm\n");
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
						/*statisticsPtr->services[i].usersServedDaily == 0 ? 0 : statisticsPtr->services[i].waitingTimeDaily/(float)statisticsPtr->services[i].usersServedDaily,*/
						statisticsPtr->services[i].averageServiceDurationDaily);
	}
	puts("");
	printf("**********************************************************************************\n");
	printf("\t\t\t\t\t\tEXTRA DAILY STATS - day: %d\n", day);
	printf("**********************************************************************************\n");
	printf("%s %s\n", "active_operators_daily", "average_breaks_daily");
	printf("%22d %20.2f\n", statisticsPtr->activeOperatorsDaily,
					statisticsPtr->averageBreaksDaily);
	
	puts("");
	printf("**********************************************************************************\n");
	printf("\t\t\t\t\t\tOPERATOR TO SPORTELLO RATIO - day: %d\n", day);
	printf("**********************************************************************************\n");
	printf("%s %s %s %s\n", "Sportello", "Pid", "Service", "Ratio");
	for (int i = 0; i < nofWorkerSeats; i++){
		printf("%9d %3d %s %5.2f\n", i+1, sportelliStatsPtr[i].pid, sportelliStatsPtr[i].service, sportelliStatsPtr[i].ratio);
	}
	

	puts("");
	printf("**********************************************************************************\n");
	printf("\t\t\t\t\t\tTOTAL STATS - day: %d\n", day);
	printf("**********************************************************************************\n");
	for (int i = 0; i < SERVICES; i++){
		printf("SERVICE: %s\n", statisticsPtr->services[i].serviceType);	
		printf("%s %s %s %s %s\n", "tot_users_served", "tot_services_provided", 
									"tot_services_not_provided", 
									"average_waiting_time_simulation", 
									"average_service_duration_simulation");	
		printf("%16d %21d %25d %31.2f %35.2f\n", statisticsPtr->services[i].totalUsersServed, 
						statisticsPtr->services[i].totalServicesProvided, 
						statisticsPtr->services[i].totalServicesNotProvided,
						statisticsPtr->services[i].averageWaitingTimeSimulation, 
						statisticsPtr->services[i].averageServiceDurationSimulation);
	}
	puts("");
	printf("**********************************************************************************\n");
	printf("\t\t\t\t\t\tEXTRA TOTAL STATS - day: %d\n", day);
	printf("**********************************************************************************\n");
	printf("%s %s\n", "active_operators_simulation", "total_breaks_simulation");
	printf("%27d %23d\n", statisticsPtr->activeOperatorsSimulation,
					statisticsPtr->totalBreaksSimulation);
	if (release_sem(semId, 0) == -1){
		printf("failed to release stats shm sem\n");
		return -1;
	}
	//detach da statistiche
	if (shmdt(statisticsPtr) == -1){
		printf("failed to detach stats shm\n");
		return -1;
	}

	if (release_sem(sssemId, 0) == -1){
		printf("failed to release sportelli stats shm sem\n");
		return -1;
	}
	//detach da statistiche
	if (shmdt(sportelliStatsPtr) == -1){
		printf("failed to detach sportelli stats shm\n");
		return -1;
	}
	return 0;
}

int reset_statistics (int shmId, int semId){
	printf("start reset_statistics\n");
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
	for (int i = 0; i < SERVICES; i++){
		statisticsPtr->services[i].averageDailyUsersServed = 0.0;
		statisticsPtr->services[i].averageDailyServicesProvided = 0.0;
		statisticsPtr->services[i].averageDailyServicesNotProvided = 0.0;
		statisticsPtr->services[i].averageWaitingTimeDaily = 0.0;
		statisticsPtr->services[i].averageServiceDurationDaily = 0.0;
		statisticsPtr->services[i].usersServedDaily = 0;
		//Not to be printed

	}
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
 int add_pause_stat (int shmId, int semId){
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
	//numero medio di pause nella giornata
	statisticsPtr->averageBreaksDaily += 1 / (float)statisticsPtr->activeOperatorsDaily;
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
				statisticsPtr->services[i].averageDailyServicesProvided += 1;
				statisticsPtr->services[i].totalServicesProvided += 1;	
			}else{
				statisticsPtr->services[i].averageDailyServicesNotProvided += 1;
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

//TODO criticita se il numero di utenti dovesse crescere
int update_user_served_stat (int shmId, int semId, char *service, int nofUsers){
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
			statisticsPtr->services[i].usersServedDaily += 1;
			statisticsPtr->services[i].totalUsersServed += 1;	
			statisticsPtr->services[i].averageDailyUsersServed += 1/(float)nofUsers;
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

int update_operator_seat_ratio (int shmId, int semId, char *service, int nofWorkerSeats){
	printf("start update_operator_seat_ratio\n");

	if (reserve_sem(semId, 0) == -1){
		printf("failed to reserve sportelli stats shm sem\n");
		return -1;
	}
	SportelloStatAdtPtr sportelliStatsPtr = shmat(shmId, NULL, SHM_RND);
	if (sportelliStatsPtr == (void*)-1){
		printf("failed to attach sportelli stats shm\n");
		return -1;
	}
	//
	int spCount = 0;
	for (int i = 0; i < nofWorkerSeats; i++){
		if (strcmp(sportelliStatsPtr[i].service, service) == 0){
			spCount += 1;
		}
	}	
	
	for (int i = 0; i < nofWorkerSeats; i++){
		if (strcmp(sportelliStatsPtr[i].service, service) == 0){
			sportelliStatsPtr[i].ratio += 1/(float)spCount;
		}
	}	

	if (release_sem(semId, 0) == -1){
		printf("failed to release sportelli stats shm sem\n");
		return -1;
	}
	//detach da statistiche
	if (shmdt(sportelliStatsPtr) == -1){
		printf("failed to detach sportelli stats shm\n");
		return -1;
	}
	printf("end update_operator_seat_ratio\n");
	return 0;
}


int update_waiting_time (int shmId, int semId, char *service, long elapsed){
	printf("start update_av_waiting_time\n");
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
			statisticsPtr->services[i].averageWaitingTimeDaily += elapsed/(double)statisticsPtr->services[i].usersServedDaily; 
			statisticsPtr->services[i].averageWaitingTimeSimulation += statisticsPtr->services[i].averageWaitingTimeDaily;
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
	printf("end update_av_waiting_time\n");
	return 0;
}


int update_service_duration (int shmId, int semId, char *service, int temp){
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
			statisticsPtr->services[i].averageServiceDurationDaily += temp/(float)statisticsPtr->services[i].usersServedDaily;	
			statisticsPtr->services[i].averageServiceDurationSimulation += statisticsPtr->services[i].averageServiceDurationDaily;
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

int create_stats_file (char *dailyStats, char *extraDailyStats, char *operatorSportelloStats, char *totalStats, char *extraTotalStats){
	printf("start create_daily_stat_file\n");
	FILE *dailyFile = fopen(dailyStats, "w+");
	FILE *extraDailyFile = fopen(extraDailyStats, "w+");
	FILE *operatorRatioFile = fopen(operatorSportelloStats, "w+");
	FILE *totalFile = fopen(totalStats, "w+");
	FILE *extraTotalFile = fopen(extraTotalStats, "w+");
	if (dailyFile == NULL || extraDailyFile == NULL || operatorRatioFile == NULL || totalFile == NULL || extraTotalFile == NULL){
		printf("failed to open daily stats file\n");
		return -1;
	}
	
	// header daily stats
	fprintf(dailyFile, "Day,Service,Av_users_served,Av_provided,Av_not_provided,Av_wait_time,Av_serv_duration\n");
	fprintf(extraDailyFile, "Day,Active_operators,Av_breaks\n");
	// header total stats
	fprintf(totalFile,"Service,Users_served,Provided,Not_provided,Av_wait_time,Av_serv_duration\n");
	fprintf(extraTotalFile,"Active_operators,Total_breaks\n");

	// header operato ratio
	fprintf(operatorRatioFile, "Day,Sportello,Pid,Service,Ratio\n");

	fclose(dailyFile);
	fclose(extraDailyFile);
	fclose(operatorRatioFile);
	fclose(totalFile);
	fclose(extraTotalFile);
	printf("end create_daily_stat_file\n");
	return 0;
}

int dump_daily_stats (char *filename, char *extraFilename, int shmId, int semId, int day){
	printf("start dump_daily_stats\n");
	FILE *file = fopen(filename, "a");
	FILE *extraFile = fopen(extraFilename, "a");
	if (file == NULL || extraFile == NULL){
		printf("failed to open daily stats file\n");
		return -1;
	}
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
	
	for(int i = 0; i < SERVICES; i++){
		fprintf(file, "%d,%s,%.2f,%.2f,%.2f,%.2f,%.2f\n", day, statisticsPtr->services[i].serviceType,
						statisticsPtr->services[i].averageDailyUsersServed, 
						statisticsPtr->services[i].averageDailyServicesProvided,
						statisticsPtr->services[i].averageDailyServicesNotProvided, 
						statisticsPtr->services[i].averageWaitingTimeDaily, 
						statisticsPtr->services[i].averageServiceDurationDaily);


	}
	
	fprintf(extraFile, "%d,%d,%.2f\n", day, statisticsPtr->activeOperatorsDaily, statisticsPtr->averageBreaksDaily);
	
	
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
	fclose(file);
	fclose(extraFile);
	printf("end dump_daily_stats\n");
	return 0;
}


int dump_operator_daily_ratio (char *filename, int shmId, int semId, int day, int nofWorkerSeats){
	printf("start dump_operator_daily_ratio\n");
	FILE *file = fopen(filename, "a");
	if (file == NULL){
		printf("failed to open daily stats file\n");
		return -1;
	}
	if (reserve_sem(semId, 0) == -1){
		printf("failed to reserve sportelli stats shm sem\n");
		return -1;
	}
	SportelloStatAdtPtr sportelliStatsPtr = shmat(shmId, NULL, SHM_RND);
	if (sportelliStatsPtr == (void*)-1){
		printf("failed to attach sportelli stats shm\n");
		return -1;
	}
	fprintf(file, "%d,", day);
	for (int i = 0; i < nofWorkerSeats; i++){
		if (i == 0){
			fprintf(file, "%d,%d,%s,%.2f\n",i+1, sportelliStatsPtr[i].pid, sportelliStatsPtr[i].service, sportelliStatsPtr[i].ratio);
		}else{
			fprintf(file, "%s,%d,%d,%s,%.2f\n", "", i+1, sportelliStatsPtr[i].pid, sportelliStatsPtr[i].service, sportelliStatsPtr[i].ratio);
		}
	}


	if (release_sem(semId, 0) == -1){
		printf("failed to release sportelli stats shm sem\n");
		return -1;
	}
	//detach da statistiche
	if (shmdt(sportelliStatsPtr) == -1){
		printf("failed to detach sportelli stats shm\n");
		return -1;
	}
	fclose(file);
	printf("end dump_operator_daily_ratio\n");
	return 0;
}


int dump_total_stats (char *filename, char *extraFilename, int shmId, int semId){
	printf("start dump_total_stats\n");
	FILE *file = fopen(filename, "a");
	FILE *extraFile = fopen(extraFilename, "a");
	if (file == NULL || extraFile == NULL){
		printf("failed to open daily stats file\n");
		return -1;
	}
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
		fprintf(file, "%s,%d,%d,%d,%.2f,%.2f\n", statisticsPtr->services[i].serviceType,
							statisticsPtr->services[i].totalUsersServed,
							statisticsPtr->services[i].totalServicesProvided,
							statisticsPtr->services[i].totalServicesNotProvided,
							statisticsPtr->services[i].averageWaitingTimeSimulation,
							statisticsPtr->services[i].averageServiceDurationSimulation);

	}
	
	fprintf(extraFile, "%d,%d\n", statisticsPtr->activeOperatorsSimulation, statisticsPtr->totalBreaksSimulation);
	

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
	fclose(file);
	fclose(extraFile);

	printf("end dump_total_stats\n");
	return 0;
}
