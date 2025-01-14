#ifndef SIMULATION_STATS_H
#define SIMULATION_STATS_H

#define SERVICE_LEN 4 
#define SERVICES 6

typedef struct serviceStat ServiceStat;
struct serviceStat {
	char serviceType[SERVICE_LEN];
	int totalUsersServed;		/* il numero di utenti serviti totali nella simulazione */
	float averageDailyUsersServed;		/* il numero di utenti serviti in media al giorno */
	int totalServicesProvided;			/* il numero di servizi erogati totali nella simulazione */
	int totalServicesNotProvided;			/* il numero di servizi non erogati totali nella simulazione */
	float averageDailyServicesProvided;			/* il numero di servizi erogati in media al giorno */
	float averageDailyServicesNotProvided;			/* il numero di servizi non erogati in media al giorno */
	double averageWaitingTimeSimulation;			/* il tempo medio di attesa degli utenti nella simulazione */
	double averageWaitingTimeDaily;			/* il tempo medio di attesa degli utenti nella giornata */
	float averageServiceDurationSimulation;			/* il tempo medio di erogazione dei servizi nella simulazione */
	float averageServiceDurationDaily;			/* il tempo medio di erogazione dei servizi nella giornata */
	//Not to be printed
	int usersServedDaily;
	//long waitingTimeDaily;
	//int servicesProvidedDaily;
};
	

typedef struct statistics StatisticsAdt, *StatisticsAdtPtr;
struct statistics {
	ServiceStat services[SERVICES];
	int activeOperatorsDaily;			/* il numero di operatori attivi durante la giornata */
	int activeOperatorsSimulation;			/* il numero di operatori attivi durante la simulazione */
	float averageBreaksDaily;			/* il numero medio di pause effettuate nella giornata */
	int totalBreaksSimulation;			/* totale di pause effettuate durante la simulazione */
};

typedef struct sportelloStat SportelloStatAdt, *SportelloStatAdtPtr;
struct sportelloStat {
	char service[SERVICE_LEN];
	int pid;
	float ratio;
};

int init_statistics (int shmId, int semId, const char **services);
int print_stats (int shmId, int semId, int ssshmId, int sssemId, int day, int nofWorkerSeats);
int reset_statistics (int shmId, int semId);
int add_operator_to_gen_stat (int shmId, int semId);
int add_pause_stat (int shmId, int semId);
int update_service_stat (int shmId, int semId, char *service, int serviceProvided);
int update_user_served_stat (int shmId, int semId, char *service, int nofUsers);
int update_operator_seat_ratio (int shmId, int semId, char *service, int nofWorkerSeats);
int update_waiting_time (int shmId, int semId, char *service, long elapsed);
int create_stats_file (char *filename, char *extraFilename, char *opRatioFilename, char *totalFilename, char *extraTotalFilename);
int dump_daily_stats(char *filename, char *extraFilename, int shmId, int semId, int day);
int dump_operator_daily_ratio (char *filename, int shmId, int semId, int day, int nofWorkerSeats);
#endif
