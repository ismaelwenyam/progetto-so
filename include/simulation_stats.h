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
	float averageWaitingTimeSimulation;			/* il tempo medio di attesa degli utenti nella simulazione */
	float averageWaitingTimeDaily;			/* il tempo medio di attesa degli utenti nella giornata */
	float averageServiceDurationSimulation;			/* il tempo medio di erogazione dei servizi nella simulazione */
	float averageServiceDurationDaily;			/* il tempo medio di erogazione dei servizi nella giornata */
};
	

typedef struct statistics StatisticsAdt, *StatisticsAdtPtr;
struct statistics {
	ServiceStat services[SERVICES];
	int activeOperatorsDaily;			/* il numero di operatori attivi durante la giornata */
	int activeOperatorsSimulation;			/* il numero di operatori attivi durante la simulazione */
	float averageBreaksDaily;			/* il numero medio di pause effettuate nella giornata */
	int totalBreaksSimulation;			/* totale di pause effettuate durante la simulazione */
	float operatorToCounterRatio;			/* il rapporto fra operatori disponibili e sportelli esistenti, per ogni sportello per ogni giornata */
};


int init_statistics (int shmId, int semId, const char **services);
int print_stats (int shmId, int semId, int day);
int reset_statistics (int shmId, int semId);
int add_operator_to_gen_stat (int shmId, int semId);
int add_pause_to_gen_stat (int shmId, int semId);
int update_service_stat (int shmId, int semId, char *service, int serviceProvided);
int update_user_served_stat (int shmId, int semId, char *service);

#endif
