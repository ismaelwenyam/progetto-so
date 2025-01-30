#include <stdio.h>
#include <string.h>

#include "simulation_stats.h"
#include "semapi.h"
#include "shmapi.h"

int init_statistics(int shmId, int semId, const char **services)
{
	// reserve semaforo per accesso ad shm statistiche
	if (reserve_sem(semId, 0) == -1)
	{
		printf("failed to stats shm sem\n");
		return -1;
	}
	// accesso a statistiche
	StatisticsAdtPtr statisticsPtr = shmat(shmId, NULL, SHM_RND);
	if (statisticsPtr == (void *)-1)
	{
		printf("failed to attach stats shm\n");
		return -1;
	}
	// inizializzazione delle statistichie
	statisticsPtr->activeOperatorsDaily = 0;
	statisticsPtr->activeOperatorsSimulation = 0;
	statisticsPtr->averageBreaksDaily = 0.0;
	statisticsPtr->totalBreaksSimulation = 0;
	for (int i = 0; i < SERVICES; i++)
	{
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
	// release semaforo accesso statistiche
	if (release_sem(semId, 0) == -1)
	{
		printf("failed to release stats shm sem\n");
		return -1;
	}
	// detach da statistiche
	if (shmdt(statisticsPtr) == -1)
	{
		printf("failed to detach stats shm\n");
		return -1;
	}
	return 0;
}

int print_stats(int shmId, int semId, int ssshmId, int sssemId, int day, int nofWorkerSeats)
{
	if (reserve_sem(semId, 0) == -1)
	{
		printf("failed to reserve stats shm sem\n");
		return -1;
	}
	StatisticsAdtPtr statisticsPtr = shmat(shmId, NULL, SHM_RND);
	if (statisticsPtr == (void *)-1)
	{
		printf("failed to attach stats shm\n");
		return -1;
	}

	if (reserve_sem(sssemId, 0) == -1)
	{
		printf("failed to reserve sportelli stats shm sem\n");
		return -1;
	}
	SportelloStatAdtPtr sportelliStatsPtr = shmat(ssshmId, NULL, SHM_RND);
	if (sportelliStatsPtr == (void *)-1)
	{
		printf("failed to attach sportelli stats shm\n");
		return -1;
	}

	puts("");
	printf("******************** DAILY STATS - day: %d ********************\n", day);
	for (int i = 0; i < SERVICES; i++)
	{
		printf("SERVICE: %s\n", statisticsPtr->services[i].serviceType);
		printf("%s %s %s %s %s\n", "av_users_served", "av_services_provided",
			   "av_services_not_provided",
			   "av_waiting_time",
			   "av_duration");
		printf("%15.2f %20.2f %24.2f %15.2f %11.2f\n", statisticsPtr->services[i].averageDailyUsersServed,
			   statisticsPtr->services[i].averageDailyServicesProvided,
			   statisticsPtr->services[i].averageDailyServicesNotProvided,
			   statisticsPtr->services[i].averageWaitingTimeDaily,
			   statisticsPtr->services[i].averageServiceDurationDaily);
	}
	puts("");
	puts("");
	printf("******************** EXTRA DAILY STATS - day: %d ********************\n", day);
	printf("%s %s\n", "active_operators", "av_breaks");
	printf("%16d %9.2f\n", statisticsPtr->activeOperatorsDaily,
		   statisticsPtr->averageBreaksDaily);
	puts("");

	puts("");
	char *services[6] = {"", "", "", "", "", ""};
	printf("******************** OPERATOR TO SPORTELLO RATIO - day: %d ********************\n", day);
	printf("%s %s\n", "Service", "Ratio");
	for (int i = 0; i < nofWorkerSeats; i++)
	{
		int alreadyPrinted = 0;

		// Controlla se il servizio è già stato stampato
		for (int j = 0; j < 6; j++)
		{
			if (strcmp(sportelliStatsPtr[i].service, services[j]) == 0)
			{
				alreadyPrinted = 1;
				break;
			}
		}
		// Se non è stato stampato, stampalo e salvalo nell'array
		if (!alreadyPrinted)
		{
			printf("%7s %5.2f\n", sportelliStatsPtr[i].service, sportelliStatsPtr[i].ratio);
			// Trova uno slot libero nell'array
			for (int j = 0; j < 6; j++)
			{
				if (strcmp(services[j], "") == 0)
				{
					services[j] = sportelliStatsPtr[i].service;
					break;
				}
			}
		}
	}
	puts("");

	puts("");
	printf("******************** TOTAL STATS - day: %d ********************\n", day);
	for (int i = 0; i < SERVICES; i++)
	{
		printf("SERVICE: %s\n", statisticsPtr->services[i].serviceType);
		printf("%s %s %s %s %s\n", "users_served", "services_provided",
			   "services_not_provided",
			   "av_waiting_time",
			   "av_service_duration");
		printf("%12d %17d %21d %15.2f %19.2f\n", statisticsPtr->services[i].totalUsersServed,
			   statisticsPtr->services[i].totalServicesProvided,
			   statisticsPtr->services[i].totalServicesNotProvided,
			   statisticsPtr->services[i].averageWaitingTimeSimulation,
			   statisticsPtr->services[i].averageServiceDurationSimulation);
	}
	puts("");

	puts("");
	printf("******************** EXTRA TOTAL STATS - day: %d ********************\n", day);
	printf("%s %s\n", "active_operators", "total_breaks");
	printf("%16d %12d\n", statisticsPtr->activeOperatorsSimulation,
		   statisticsPtr->totalBreaksSimulation);
	puts("");
	puts("");
	if (release_sem(semId, 0) == -1)
	{
		printf("failed to release stats shm sem\n");
		return -1;
	}
	// detach da statistiche
	if (shmdt(statisticsPtr) == -1)
	{
		printf("failed to detach stats shm\n");
		return -1;
	}

	if (release_sem(sssemId, 0) == -1)
	{
		printf("failed to release sportelli stats shm sem\n");
		return -1;
	}
	// detach da statistiche
	if (shmdt(sportelliStatsPtr) == -1)
	{
		printf("failed to detach sportelli stats shm\n");
		return -1;
	}
	return 0;
}

int reset_statistics(int shmId, int semId)
{
	// reserve semaforo per accesso ad shm statistiche
	if (reserve_sem(semId, 0) == -1)
	{
		printf("failed to reserve stats shm sem\n");
		return -1;
	}
	// accesso a statistiche
	StatisticsAdtPtr statisticsPtr = shmat(shmId, NULL, SHM_RND);
	if (statisticsPtr == (void *)-1)
	{
		printf("failed to attach stats shm\n");
		return -1;
	}
	// inizializzazione delle statistichie
	statisticsPtr->activeOperatorsDaily = 0;
	statisticsPtr->averageBreaksDaily = 0.0;
	for (int i = 0; i < SERVICES; i++)
	{
		statisticsPtr->services[i].averageDailyUsersServed = 0.0;
		statisticsPtr->services[i].averageDailyServicesProvided = 0.0;
		statisticsPtr->services[i].averageDailyServicesNotProvided = 0.0;
		statisticsPtr->services[i].averageWaitingTimeDaily = 0.0;
		statisticsPtr->services[i].averageServiceDurationDaily = 0.0;
		statisticsPtr->services[i].usersServedDaily = 0;
	}
	// release semaforo accesso statistiche
	if (release_sem(semId, 0) == -1)
	{
		printf("failed to release stats shm sem\n");
		return -1;
	}
	// detach da statistiche
	if (shmdt(statisticsPtr) == -1)
	{
		printf("failed to detach stats shm\n");
		return -1;
	}
	return 0;
}

int add_operator_to_gen_stat(int shmId, int semId)
{
	if (reserve_sem(semId, 0) == -1)
	{
		printf("failed to reserve stats shm sem\n");
		return -1;
	}
	StatisticsAdtPtr statisticsPtr = shmat(shmId, NULL, SHM_RND);
	if (statisticsPtr == (void *)-1)
	{
		printf("failed to attach stats shm\n");
		return -1;
	}
	statisticsPtr->activeOperatorsDaily += 1;
	statisticsPtr->activeOperatorsSimulation += 1;
	if (release_sem(semId, 0) == -1)
	{
		printf("failed to release stats shm sem\n");
		return -1;
	}
	if (shmdt(statisticsPtr) == -1)
	{
		printf("failed to detach stats shm\n");
		return -1;
	}
	return 0;
}
int add_pause_stat(int shmId, int semId)
{
	if (reserve_sem(semId, 0) == -1)
	{
		printf("failed to reserve stats shm sem\n");
		return -1;
	}
	StatisticsAdtPtr statisticsPtr = shmat(shmId, NULL, SHM_RND);
	if (statisticsPtr == (void *)-1)
	{
		printf("failed to attach stats shm\n");
		return -1;
	}
	statisticsPtr->totalBreaksSimulation += 1;
	statisticsPtr->averageBreaksDaily += 1 / (float)statisticsPtr->activeOperatorsDaily;
	if (release_sem(semId, 0) == -1)
	{
		printf("failed to release stats shm sem\n");
		return -1;
	}
	if (shmdt(statisticsPtr) == -1)
	{
		printf("failed to detach stats shm\n");
		return -1;
	}
	return 0;
}

int update_service_stat(int shmId, int semId, char *service, int serviceProvided)
{
	if (reserve_sem(semId, 0) == -1)
	{
		printf("failed to reserve stats shm sem\n");
		return -1;
	}
	// accesso a statistiche
	StatisticsAdtPtr statisticsPtr = shmat(shmId, NULL, SHM_RND);
	if (statisticsPtr == (void *)-1)
	{
		printf("failed to attach stats shm\n");
		return -1;
	}

	for (int i = 0; i < SERVICES; i++)
	{
		if (strcmp(statisticsPtr->services[i].serviceType, service) == 0)
		{
			if (serviceProvided)
			{
				statisticsPtr->services[i].averageDailyServicesProvided += 1;
				statisticsPtr->services[i].totalServicesProvided += 1;
			}
			else
			{
				statisticsPtr->services[i].averageDailyServicesNotProvided += 1;
				statisticsPtr->services[i].totalServicesNotProvided += 1;
			}
			break;
		}
	}

	// release semaforo accesso statistiche
	if (release_sem(semId, 0) == -1)
	{
		printf("failed to release stats shm sem\n");
		return -1;
	}
	// detach da statistiche
	if (shmdt(statisticsPtr) == -1)
	{
		printf("failed to detach stats shm\n");
		return -1;
	}
	return 0;
}

// TODO criticita se il numero di utenti dovesse crescere
int update_user_served_stat(int shmId, int semId, char *service, int nofUsers)
{
	if (reserve_sem(semId, 0) == -1)
	{
		printf("failed to reserve stats shm sem\n");
		return -1;
	}
	// accesso a statistiche
	StatisticsAdtPtr statisticsPtr = shmat(shmId, NULL, SHM_RND);
	if (statisticsPtr == (void *)-1)
	{
		printf("failed to attach stats shm\n");
		return -1;
	}

	for (int i = 0; i < SERVICES; i++)
	{
		if (strcmp(statisticsPtr->services[i].serviceType, service) == 0)
		{
			statisticsPtr->services[i].usersServedDaily += 1;
			statisticsPtr->services[i].totalUsersServed += 1;
			statisticsPtr->services[i].averageDailyUsersServed += 1 / (float)nofUsers;
			break;
		}
	}

	// release semaforo accesso statistiche
	if (release_sem(semId, 0) == -1)
	{
		printf("failed to release stats shm sem\n");
		return -1;
	}
	// detach da statistiche
	if (shmdt(statisticsPtr) == -1)
	{
		printf("failed to detach stats shm\n");
		return -1;
	}
	return 0;
}

int update_operator_seat_ratio(int shmId, int semId, char *service, int nofWorkerSeats)
{

	if (reserve_sem(semId, 0) == -1)
	{
		printf("failed to reserve sportelli stats shm sem\n");
		return -1;
	}
	SportelloStatAdtPtr sportelliStatsPtr = shmat(shmId, NULL, SHM_RND);
	if (sportelliStatsPtr == (void *)-1)
	{
		printf("failed to attach sportelli stats shm\n");
		return -1;
	}
	//
	int spCount = 0;
	for (int i = 0; i < nofWorkerSeats; i++)
	{
		if (strcmp(sportelliStatsPtr[i].service, service) == 0)
		{
			spCount += 1;
		}
	}

	for (int i = 0; i < nofWorkerSeats; i++)
	{
		if (strcmp(sportelliStatsPtr[i].service, service) == 0)
		{
			sportelliStatsPtr[i].ratio += 1 / (float)spCount;
		}
	}

	if (release_sem(semId, 0) == -1)
	{
		printf("failed to release sportelli stats shm sem\n");
		return -1;
	}
	// detach da statistiche
	if (shmdt(sportelliStatsPtr) == -1)
	{
		printf("failed to detach sportelli stats shm\n");
		return -1;
	}
	return 0;
}

int update_waiting_time(int shmId, int semId, char *service, double elapsed)
{
	if (reserve_sem(semId, 0) == -1)
	{
		printf("failed to reserve stats shm sem\n");
		return -1;
	}
	// accesso a statistiche
	StatisticsAdtPtr statisticsPtr = shmat(shmId, NULL, SHM_RND);
	if (statisticsPtr == (void *)-1)
	{
		printf("failed to attach stats shm\n");
		return -1;
	}

	for (int i = 0; i < SERVICES; i++)
	{
		if (strcmp(statisticsPtr->services[i].serviceType, service) == 0)
		{
			statisticsPtr->services[i].averageWaitingTimeDaily += elapsed / (double)statisticsPtr->services[i].usersServedDaily;
			statisticsPtr->services[i].averageWaitingTimeSimulation += statisticsPtr->services[i].averageWaitingTimeDaily;
			break;
		}
	}

	// release semaforo accesso statistiche
	if (release_sem(semId, 0) == -1)
	{
		printf("failed to release stats shm sem\n");
		return -1;
	}
	// detach da statistiche
	if (shmdt(statisticsPtr) == -1)
	{
		printf("failed to detach stats shm\n");
		return -1;
	}
	return 0;
}

int update_service_duration(int shmId, int semId, char *service, int temp)
{
	if (reserve_sem(semId, 0) == -1)
	{
		printf("failed to reserve stats shm sem\n");
		return -1;
	}
	// accesso a statistiche
	StatisticsAdtPtr statisticsPtr = shmat(shmId, NULL, SHM_RND);
	if (statisticsPtr == (void *)-1)
	{
		printf("failed to attach stats shm\n");
		return -1;
	}

	for (int i = 0; i < SERVICES; i++)
	{
		if (strcmp(statisticsPtr->services[i].serviceType, service) == 0)
		{
			statisticsPtr->services[i].averageServiceDurationDaily += temp / (float)statisticsPtr->services[i].usersServedDaily;
			statisticsPtr->services[i].averageServiceDurationSimulation += statisticsPtr->services[i].averageServiceDurationDaily;
			break;
		}
	}

	// release semaforo accesso statistiche
	if (release_sem(semId, 0) == -1)
	{
		printf("failed to release stats shm sem\n");
		return -1;
	}
	// detach da statistiche
	if (shmdt(statisticsPtr) == -1)
	{
		printf("failed to detach stats shm\n");
		return -1;
	}
	return 0;
}

int create_stats_file(char *dailyStats, char *extraDailyStats, char *operatorSportelloStats, char *totalStats, char *extraTotalStats)
{
	printf("start create_daily_stat_file\n");
	FILE *dailyFile = fopen(dailyStats, "w+");
	FILE *extraDailyFile = fopen(extraDailyStats, "w+");
	FILE *operatorRatioFile = fopen(operatorSportelloStats, "w+");
	FILE *totalFile = fopen(totalStats, "w+");
	FILE *extraTotalFile = fopen(extraTotalStats, "w+");
	if (dailyFile == NULL || extraDailyFile == NULL || operatorRatioFile == NULL || totalFile == NULL || extraTotalFile == NULL)
	{
		printf("failed to open daily stats file\n");
		return -1;
	}

	// header daily stats
	fprintf(dailyFile, "Day,Service,Av_users_served,Av_provided,Av_not_provided,Av_wait_time,Av_serv_duration\n");
	fprintf(extraDailyFile, "Day,Active_operators,Av_breaks\n");
	// header total stats
	fprintf(totalFile, "Service,Users_served,Provided,Not_provided,Av_wait_time,Av_serv_duration\n");
	fprintf(extraTotalFile, "Active_operators,Total_breaks\n");

	// header operato ratio
	fprintf(operatorRatioFile, "Day,Service,Ratio\n");

	fclose(dailyFile);
	fclose(extraDailyFile);
	fclose(operatorRatioFile);
	fclose(totalFile);
	fclose(extraTotalFile);
	printf("end create_daily_stat_file\n");
	return 0;
}

int dump_daily_stats(char *filename, char *extraFilename, int shmId, int semId, int day)
{
	FILE *file = fopen(filename, "a");
	FILE *extraFile = fopen(extraFilename, "a");
	if (file == NULL || extraFile == NULL)
	{
		printf("failed to open daily stats file\n");
		return -1;
	}
	if (reserve_sem(semId, 0) == -1)
	{
		printf("failed to reserve stats shm sem\n");
		return -1;
	}
	// accesso a statistiche
	StatisticsAdtPtr statisticsPtr = shmat(shmId, NULL, SHM_RND);
	if (statisticsPtr == (void *)-1)
	{
		printf("failed to attach stats shm\n");
		return -1;
	}

	for (int i = 0; i < SERVICES; i++)
	{
		fprintf(file, "%d,%s,%.2f,%.2f,%.2f,%.2f,%.2f\n", day, statisticsPtr->services[i].serviceType,
				statisticsPtr->services[i].averageDailyUsersServed,
				statisticsPtr->services[i].averageDailyServicesProvided,
				statisticsPtr->services[i].averageDailyServicesNotProvided,
				statisticsPtr->services[i].averageWaitingTimeDaily,
				statisticsPtr->services[i].averageServiceDurationDaily);
	}

	fprintf(extraFile, "%d,%d,%.2f\n", day, statisticsPtr->activeOperatorsDaily, statisticsPtr->averageBreaksDaily);

	// release semaforo accesso statistiche
	if (release_sem(semId, 0) == -1)
	{
		printf("failed to release stats shm sem\n");
		return -1;
	}
	// detach da statistiche
	if (shmdt(statisticsPtr) == -1)
	{
		printf("failed to detach stats shm\n");
		return -1;
	}
	fclose(file);
	fclose(extraFile);
	return 0;
}

int dump_operator_daily_ratio(char *filename, int shmId, int semId, int day, int nofWorkerSeats)
{
	FILE *file = fopen(filename, "a");
	if (file == NULL)
	{
		printf("failed to open daily stats file\n");
		return -1;
	}
	if (reserve_sem(semId, 0) == -1)
	{
		printf("failed to reserve sportelli stats shm sem\n");
		return -1;
	}
	SportelloStatAdtPtr sportelliStatsPtr = shmat(shmId, NULL, SHM_RND);
	if (sportelliStatsPtr == (void *)-1)
	{
		printf("failed to attach sportelli stats shm\n");
		return -1;
	}
	fprintf(file, "%d,", day);
	char *services[6] = {"", "", "", "", "", ""};
	for (int i = 0; i < nofWorkerSeats; i++)
	{
		int alreadyPresent = 0;
		for (int j = 0; j < 6; j++)
		{
			if (strcmp(services[j], sportelliStatsPtr[i].service) == 0)
			{
				alreadyPresent = 1;
				break;
			}
		}
		if (!alreadyPresent)
		{
			if (i == 0)
			{
				fprintf(file, "%s,%.2f\n", sportelliStatsPtr[i].service, sportelliStatsPtr[i].ratio);
			}
			else
			{
				fprintf(file, "%s,%s,%.2f\n", "", sportelliStatsPtr[i].service, sportelliStatsPtr[i].ratio);
			}
			for (int j = 0; j < 6; j++)
			{
				if (strcmp(services[j], "") == 0)
				{
					services[j] = sportelliStatsPtr[i].service;
					break;
				}
			}
		}
	}

	if (release_sem(semId, 0) == -1)
	{
		printf("failed to release sportelli stats shm sem\n");
		return -1;
	}
	// detach da statistiche
	if (shmdt(sportelliStatsPtr) == -1)
	{
		printf("failed to detach sportelli stats shm\n");
		return -1;
	}
	fclose(file);
	return 0;
}

int dump_total_stats(char *filename, char *extraFilename, int shmId, int semId)
{
	FILE *file = fopen(filename, "a");
	FILE *extraFile = fopen(extraFilename, "a");
	if (file == NULL || extraFile == NULL)
	{
		printf("failed to open daily stats file\n");
		return -1;
	}
	if (reserve_sem(semId, 0) == -1)
	{
		printf("failed to reserve stats shm sem\n");
		return -1;
	}
	// accesso a statistiche
	StatisticsAdtPtr statisticsPtr = shmat(shmId, NULL, SHM_RND);
	if (statisticsPtr == (void *)-1)
	{
		printf("failed to attach stats shm\n");
		return -1;
	}

	for (int i = 0; i < SERVICES; i++)
	{
		fprintf(file, "%s,%d,%d,%d,%.2f,%.2f\n", statisticsPtr->services[i].serviceType,
				statisticsPtr->services[i].totalUsersServed,
				statisticsPtr->services[i].totalServicesProvided,
				statisticsPtr->services[i].totalServicesNotProvided,
				statisticsPtr->services[i].averageWaitingTimeSimulation,
				statisticsPtr->services[i].averageServiceDurationSimulation);
	}

	fprintf(extraFile, "%d,%d\n", statisticsPtr->activeOperatorsSimulation, statisticsPtr->totalBreaksSimulation);

	// release semaforo accesso statistiche
	if (release_sem(semId, 0) == -1)
	{
		printf("failed to release stats shm sem\n");
		return -1;
	}
	// detach da statistiche
	if (shmdt(statisticsPtr) == -1)
	{
		printf("failed to detach stats shm\n");
		return -1;
	}
	fclose(file);
	fclose(extraFile);
	return 0;
}
