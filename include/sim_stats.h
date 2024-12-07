#ifndef SIM_STATS_H
#define SIM_STATS_H

typedef struct statistics StatisticsAdt, *StatisticsAdtPtr;
struct statistics {
	int totalUsersServed;		/* il numero di utenti serviti totali nella simulazione */
	int averageDailyUsersServed;		/* il numero di utenti serviti in media al giorno */
	int totalServicesProvided;			/* il numero di servizi erogati totali nella simulazione */
	int totalServicesNotProvided;			/* il numero di servizi non erogati totali nella simulazione */
	float averageDailyServicesProvided;			/* il numero di servizi erogati in media al giorno */
	float averageDailyServicesNotProvided;			/* il numero di servizi non erogati in media al giorno */
	float averageWaitingTimeSimulation;			/* il tempo medio di attesa degli utenti nella simulazione */
	float averageWaitingTimeDaily;			/* il tempo medio di attesa degli utenti nella giornata */
	float averageServiceDurationSimulation;			/* il tempo medio di erogazione dei servizi nella simulazione */
	float averageServiceDurationDaily;			/* il tempo medio di erogazione dei servizi nella giornata */
	int activeOperatorsDaily;			/* il numero di operatori attivi durante la giornata */
	int activeOperatorsSimulation;			/* il numero di operatori attivi durante la simulazione */
	float averageBreaksDaily;			/* il numero medio di pause effettuate nella giornata */
	int totalBreaksSimulation			/* totale di pause effettuate durante la simulazione */
	int operatorToCounterRatio			/* il rapporto fra operatori disponibili e sportelli esistenti, per ogni sportello per ogni giornata */
};

#endif
