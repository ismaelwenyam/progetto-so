#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "simerr.h"
#include "logapi.h"
#include "service_api.h"
#include "sportello_api.h"
#include "simulation_stats.h"
#include "semapi.h"
#include "shmapi.h"
#include "msgapi.h"
#include "simulation_configuration.h"
#include "utils_api.h"

void create_erogatore_ticket();
void create_sportello(int nofWorkerSeat);
void create_operatore(int nofWorkers);
void create_utente(int nofUsers);
int writeServicesInShm(int shmId, int semId);
int delete_ipc_resources(int id, char *resourceType);

void print_config(ConfigurationAdt configuration)
{
	slog(DIRETTORE, "simulation configuration");
	slog(DIRETTORE, "nof_worker_seats %d", configuration.nofWorkerSeats);
	slog(DIRETTORE, "nof_workers %d", configuration.nofWorkers);
	slog(DIRETTORE, "nof_users %d", configuration.nofUsers);
	slog(DIRETTORE, "sim_duration %d", configuration.simDuration);
	slog(DIRETTORE, "n_nano_secs %d", configuration.nNanoSecs);
	slog(DIRETTORE, "nof_pause %d", configuration.nofPause);
	slog(DIRETTORE, "p_serv_min %d", configuration.pServMin);
	slog(DIRETTORE, "p_serv_max %d", configuration.pServMax);
	slog(DIRETTORE, "explode_threshold %d", configuration.explodeThreshold);
	slog(DIRETTORE, "n_requests %d", configuration.nRequests);
	slog(DIRETTORE, "n_new_users %d", configuration.nRequests);
	puts("");
}

void print_logs()
{
	slog(DIRETTORE, "logs from direttore");
	slog(EROGATORE, "logs from erogatore");
	slog(SPORTELLO, "logs from sportello");
	slog(OPERATORE, "logs from operatore");
	slog(UTENTE, "logs from utente");
	puts("");
}

const char *services[] = {IRP, ILR, PVB, PBP, APF, AOB};

int main(int argc, char **argv)
{
	srand(time(NULL) + getpid());
	print_logs();
	slog(DIRETTORE, "direttore.pid.%d.started", getpid());
	int configurationSemId;
	if ((configurationSemId = semget(CONFIGURATION_SEM_KEY, 2, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR)) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.semget.configuration sem.failed", getpid());
		err_exit(strerror(errno));
	}
	if (init_sem_available(configurationSemId, 0) == -1 || init_sem_available(configurationSemId, 1) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.init_sem_available.config sem.failed!", getpid());
		err_exit(strerror(errno));
	}

	ConfigurationAdt configuration = get_config();
	if (configuration.simDuration <= 0)
	{
		slog(DIRETTORE, "direttore.pid.%d.simDuration = %d.aborting simulation", getpid(), configuration.simDuration);
		err_exit("simDuration must be greater than 0");
	}
	print_config(configuration);
	if (reset_explode(configurationSemId) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.failed to reset config_explode", getpid());
		err_exit(strerror(errno));
	}

	// TODO replace with params from config
	if (create_stats_file("./daily_stats.csv", "./extra_daily_stats.csv", "./operator_ratio.csv", "./total_stats.csv", "./extra_total_stats.csv") == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.failed to create csv stats file", getpid());
		slog(DIRETTORE, "direttore.pid.%d.simulation proceeding without stats files", getpid());
	}
	int resourceCreationSemId, ticketsMsgQueueId;
	int erogatoreSemId, utenteSemId, operatoreSemId, sportelloSemId;
	int statsShmSemId, servicesShmSemId;
	int servicesShmId, statisticsShmId;
	int sportelliShmSemId, sportelliShmId;
	int sportelliStatShmSemId, sportelliStatShmId;
	int dirMsgQueueId, serviceMsgqId;
	slog(DIRETTORE, "direttore.pid.%d.setup.start", getpid());
	if ((resourceCreationSemId = semget(RESOURCE_CREATION_SYNC_SEM_KEY, 1, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR)) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.semget.resource creation sync sem.failed", getpid());
		err_exit(strerror(errno));
	}
	slog(DIRETTORE, "direttore.pid.%d.resourceCreationSemId.%d", getpid(), resourceCreationSemId);
	slog(DIRETTORE, "direttore.pid.%d.semget.ipc_creat.resource creation sync sem.ok!", getpid());
	if (init_sem_available(resourceCreationSemId, 0) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.init_sem_available.resource creation sync sem.failed!", getpid());
		err_exit(strerror(errno));
	}

	// creazione semaforo per sincronizzazione erogatore
	slog(DIRETTORE, "direttore.pid.%d.creating erogatore_ticket_sync_sem...", getpid());
	if ((erogatoreSemId = semget(EROGATORE_SYNC_SEM, 4, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR)) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.semget.erogatore_sync_sem.failed", getpid());
		err_exit(strerror(errno));
	}
	slog(DIRETTORE, "direttore.pid.%d.creating erogatore_ticket_sync_sem.ok!", getpid());
	if (init_sem_in_use(erogatoreSemId, 0) == -1 || init_sem_in_use(erogatoreSemId, 1) == -1 || init_sem_in_use(erogatoreSemId, 2) || init_sem_in_use(erogatoreSemId, 3) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.init_sem_in_use(4).erogatoresem.failed!", getpid());
		err_exit(strerror(errno));
	}

	// creazione semaforo per sincronizzazione utente
	slog(DIRETTORE, "direttore.pid.%d.creating utente_sync_sem...", getpid());
	if ((utenteSemId = semget(UTENTE_SYNC_SEM, 3, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR)) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.semget.utente_sync_sem.failed", getpid());
		err_exit(strerror(errno));
	}
	slog(DIRETTORE, "direttore.pid.%d.creating utente_sync_sem.ok!", getpid());
	if (init_sem_in_use(utenteSemId, 0) == -1 || init_sem_in_use(utenteSemId, 1) == -1 || init_sem_in_use(utenteSemId, 2) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.init_sem_in_use(3).utentesem.failed!", getpid());
		err_exit(strerror(errno));
	}

	// creazione semaforo per sincronizzazione operatore
	slog(DIRETTORE, "direttore.pid.%d.creating operatore_sync_sem...", getpid());
	if ((operatoreSemId = semget(OPERATORE_SYNC_SEM, 5, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR)) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.semget.operatore_sync_sem.failed", getpid());
		err_exit(strerror(errno));
	}
	slog(DIRETTORE, "direttore.pid.%d.creating operatore_sync_sem.ok!", getpid());
	if (init_sem_in_use(operatoreSemId, 0) == -1 || init_sem_in_use(operatoreSemId, 1) == -1 || init_sem_in_use(operatoreSemId, 2) == -1 || init_sem_in_use(operatoreSemId, 3) == -1 || init_sem_in_use(operatoreSemId, 4) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.init_sem_in_use(5).operatoresem.failed!", getpid());
		err_exit(strerror(errno));
	}
	// creazione della coda di messaggi per i processi di tipo operatore
	if ((serviceMsgqId = msgget(SERVICE_MESSAGE_QUEUE_KEY, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR)) == -1)
	{
		slog(OPERATORE, "operatore.pid.%d.msgget.ipc_creat.services message queue.failed!", getpid());
		err_exit(strerror(errno));
	}

	// creazione semaforo per sincronizzazione sportello
	slog(DIRETTORE, "direttore.pid.%d.creating sportello_sync_sem...", getpid());
	if ((sportelloSemId = semget(SPORTELLO_SYNC_SEM, 3, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR)) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.semget.sportello_sync_sem.failed", getpid());
		err_exit(strerror(errno));
	}
	slog(DIRETTORE, "direttore.pid.%d.creating sportello_sync_sem.ok!", getpid());
	if (init_sem_in_use(sportelloSemId, 0) == -1 || init_sem_in_use(sportelloSemId, 1) == -1 || init_sem_in_use(sportelloSemId, 2) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.init_sem_in_use(2).sportellosem.failed!", getpid());
		err_exit(strerror(errno));
	}

	// creazione memoria condivisa per servizi
	slog(DIRETTORE, "direttore.pid.%d.creating sem for accessing services shared memory", getpid());
	if ((servicesShmSemId = semget(SERVICES_SHM_SEM_KEY, 1, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR)) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.semget.ipc_creat.sem access services shm", getpid());
		err_exit(strerror(errno));
	}
	slog(DIRETTORE, "direttore.pid.%d.services shm sem id: %d", getpid(), servicesShmSemId);
	slog(DIRETTORE, "direttore.pid.%d.semget.ipc_creat.sem access services shm.ok!", getpid());
	if (init_sem_in_use(servicesShmSemId, 0) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.init_sem_in_use.servicesShmSem.failed!", getpid());
		err_exit(strerror(errno));
	}

	slog(DIRETTORE, "direttore.pid.%d.creating shared memory for services...", getpid());
	if ((servicesShmId = shmget(SERVICE_SHARED_MEMORY, sizeof(ServizioAdt) * NUMBER_OF_SERVICES, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR)) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.shmget.services shared memory failed", getpid());
		err_exit(strerror(errno));
	}
	slog(DIRETTORE, "direttore.pid.%d.created services shared memory id : %d.ok!", getpid(), servicesShmId);
	// funzione per l' inserimento dei servizi
	if (writeServicesInShm(servicesShmId, servicesShmSemId) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.writeServicesInShm.failed!", getpid());
		err_exit(strerror(errno));
	}
	slog(DIRETTORE, "direttore.pid.%d.writeServicesInShm.ok", getpid());

	// creazione memoria condivisa per sportelli
	slog(DIRETTORE, "direttore.pid.%d.creating sem for accessing sportelli shared memory", getpid());
	if ((sportelliShmSemId = semget(SPORTELLI_SHM_SEM_KEY, 1, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR)) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.semget.ipc_creat.sem access sportelli shm", getpid());
		err_exit(strerror(errno));
	}
	slog(DIRETTORE, "direttore.pid.%d.semget.ipc_creat.sem access sportelli shm.ok!", getpid());
	if (init_sem_available(sportelliShmSemId, 0) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.init_sem_available.sportelli_shm_sem.failed!", getpid());
		err_exit(strerror(errno));
	}

	slog(DIRETTORE, "direttore.pid.%d.creating shared memory for services...", getpid());
	if ((sportelliShmId = shmget(SPORTELLI_SHARED_MEMORY_KEY, sizeof(SportelloAdt) * configuration.nofWorkerSeats, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR)) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.shmget.sportelli shared memory failed", getpid());
		err_exit(strerror(errno));
	}
	slog(DIRETTORE, "direttore.pid.%d.created sportelli shared memory id: %d.ok!", getpid(), sportelliShmId);

	// creazione memoria condivisa per statistiche
	slog(DIRETTORE, "direttore.pid.%d.creating sem for accessing statistics shared memory", getpid());
	if ((statsShmSemId = semget(STATS_SHM_SEM_KEY, 1, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR)) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.semget.ipc_creat.sem access stats shm.failed!", getpid());
		err_exit(strerror(errno));
	}
	slog(DIRETTORE, "direttore.pid.%d.semget.ipc_creat.sem access stats shm.ok!", getpid());
	if (init_sem_available(statsShmSemId, 0) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.init_sem_available.statsShmSem.failed!", getpid());
		err_exit(strerror(errno));
	}

	slog(DIRETTORE, "direttore.pid.%d.creating shared memory for statistics...", getpid());
	if ((statisticsShmId = shmget(STATISTICS_SHARED_MEMORY, sizeof(StatisticsAdt), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR)) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.shmget.statistics shared memory failed", getpid());
		err_exit(strerror(errno));
	}
	slog(DIRETTORE, "direttore.pid.%d.created statistics shared memory id: %d.ok!", getpid(), statisticsShmId);
	if (init_statistics(statisticsShmId, statsShmSemId, services) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.init_statistics.failed", getpid());
		err_exit(strerror(errno));
	}
	slog(DIRETTORE, "direttore.pid.%d.init_statistics.ok", getpid());

	// creazione memoria condivisa per statistiche sportelli
	slog(DIRETTORE, "direttore.pid.%d.creating sem for accessing sportelli stats shm", getpid());
	if ((sportelliStatShmSemId = semget(SPORTELLI_STATS_SHM_SEM_KEY, 1, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR)) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.semget.ipc_creat.sem access sportelli stats shm.failed!", getpid());
		err_exit(strerror(errno));
	}
	slog(DIRETTORE, "direttore.pid.%d.semget.ipc_creat.sem access sportelli stats shm.ok!", getpid());
	if (init_sem_available(sportelliStatShmSemId, 0) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.init_sem_available.sportelli stats shm sem.failed!", getpid());
		err_exit(strerror(errno));
	}
	slog(DIRETTORE, "direttore.pid.%d.creating shared memory for sportelli statistics...", getpid());
	if ((sportelliStatShmId = shmget(SPORTELLI_STATS_SHARED_MEMORY_KEY, sizeof(SportelloStatAdt) * configuration.nofWorkerSeats, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR)) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.shmget.sportelli statistics shared memory failed", getpid());
		err_exit(strerror(errno));
	}
	slog(DIRETTORE, "direttore.pid.%d.created sportelli statistics shared memory id: %d.ok!", getpid(), sportelliStatShmId);

	// creazione coda di messaggi direttore

	slog(DIRETTORE, "direttore.pid.%d.creating.director message queue", getpid());
	if ((dirMsgQueueId = msgget(DIR_MESSAGE_QUEUE_KEY, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR)) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.msgget.create director message queue.failed", getpid());
		err_exit(strerror(errno));
	}

	slog(DIRETTORE, "direttore.pid.%d.create director message queue.ok!", getpid());

	if ((ticketsMsgQueueId = msgget(TICKET_MESSAGE_QUEUE_KEY, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR)) == -1)
	{
		slog(EROGATORE, "erogatore_ticket.pid.%d.msgget.ipc_creat.tickets message queue.failed!", getpid());
		err_exit(strerror(errno));
	}

	// creazione delle risorse erogatore, utente, operatore, sportello
	slog(DIRETTORE, "direttore.pid.%d.creazione risorse", getpid());
	pid_t pid;
	pid = fork();
	if (pid == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.fork failed!", getpid());
		err_exit(strerror(errno));
	}

	if (pid == 0)
	{ /* child code */
		create_erogatore_ticket();
		create_sportello(configuration.nofWorkerSeats);
		create_operatore(configuration.nofWorkers);
		create_utente(configuration.nofUsers);
		slog(DIRETTORE, "direttore.child.pid.%d.creazione risorse.ok!", getpid());
		exit(EXIT_SUCCESS);
	}

	/* parent code */

	// attesa richiesta di funzioni da parte dei operatori
	MsgBuff msgBuff;
	int counter = 0;
	char text[MSG_LEN];
	while (counter < configuration.nofWorkers)
	{
		if (msgrcv(dirMsgQueueId, &msgBuff, sizeof(msgBuff) - sizeof(long), OPERATORE_GROUP, 0) == -1)
		{
			slog(DIRETTORE, "direttore.pid.%d.msgrcv.from operatore group.failed!", getpid());
			err_exit(strerror(errno));
		}
		// garantisce che per ogni servizio vi e un operatore che la fornisce.
		if (counter < NUMBER_OF_SERVICES)
		{
			strcpy(msgBuff.payload.msg, services[counter]);
		}
		else
		{
			int servizio = rand() % NUMBER_OF_SERVICES;
			strcpy(msgBuff.payload.msg, services[servizio]);
		}
		msgBuff.mtype = msgBuff.payload.senderPid;
		msgBuff.payload.senderPid = getpid();
		if (msgsnd(dirMsgQueueId, &msgBuff, sizeof(msgBuff) - sizeof(long), 0) == -1)
		{
			slog(DIRETTORE, "direttore.pid.%d.msgsnd.to operatore group.failed!", getpid());
			err_exit(strerror(errno));
		}

		counter++;
	}
	// ensures that workers have completed init
	slog(DIRETTORE, "direttore.pid.%d.waiting workers to complete init", getpid());
	for (int i = 0; i < configuration.nofWorkers; i++)
	{
		if (reserve_sem(operatoreSemId, 0) == -1)
		{
			// TODO handle errors
		}
	}
	// ensures that erogatore have completed init
	slog(DIRETTORE, "direttore.pid.%d.waiting erogatore to complete init", getpid());
	if (reserve_sem(erogatoreSemId, 0) == -1)
	{
		// TODO handle errors
	}
	// ensures that users have completed init
	slog(DIRETTORE, "direttore.pid.%d.waiting users to complete init", getpid());
	for (int i = 0; i < configuration.nofUsers; i++)
	{
		if (reserve_sem(utenteSemId, 0) == -1)
		{
			// TODO handle errors
		}
	}
	int days = 0;
	int users = configuration.nofUsers;
	SportelloAdtPtr sportelliPtr;
	ServizioAdtPtr servicesPtr;
	// StatisticsAdt statisticsPtr;
	SportelloStatAdtPtr sportelliStatsPtr;
	while (days < configuration.simDuration)
	{
		slog(DIRETTORE, "direttore.pid.%d.day: %d", getpid(), days + 1);
		if (reserve_sem(servicesShmSemId, 0) == -1)
		{
			slog(DIRETTORE, "direttore.pid.%d.reserve_sem.services shm sem.failed!", getpid());
			err_exit(strerror(errno));
		}

		// TODO decidere quali servizi rendere disponibile per il giorno i
		servicesPtr = shmat(servicesShmId, NULL, SHM_RND);
		if (servicesPtr == (void *)-1)
		{
			slog(DIRETTORE, "direttore.pid.%d.shmat.services shm.failed!", getpid());
			err_exit(strerror(errno));
		}
		slog(DIRETTORE, "direttore.pid.%d.accessed services shm", getpid());
		int servicesNotAvailable = 0;
		for (int i = 0; i < NUMBER_OF_SERVICES; i++)
		{
			if (servicesNotAvailable >= 5)
			{
				servicesPtr[i].available = 1;
			}
			else
			{
				int s = rand() % NUMBER_OF_SERVICES;
				s = s >= 2 ? 1 : 0;
				servicesNotAvailable += s == 0 ? 1 : 0;
				servicesPtr[i].available = s;
			}
		}

		// assegnazione funzioni agli sportelli
		if (reserve_sem(sportelliShmSemId, 0) == -1)
		{
			slog(DIRETTORE, "direttore.pid.%d.reserve_sem.sportelli shm sem.failed!", getpid());
			err_exit(strerror(errno));
		}
		sportelliPtr = shmat(sportelliShmId, NULL, SHM_RND);
		if (sportelliPtr == (void *)-1)
		{
			slog(DIRETTORE, "direttore.pid.%d.shmat sportelli.failed", getpid());
			err_exit(strerror(errno));
		}

		if (reserve_sem(sportelliStatShmSemId, 0) == -1)
		{
			slog(DIRETTORE, "direttore.pid.%d.reserve_sem.sportelli stats shm sem.failed!", getpid());
			err_exit(strerror(errno));
		}

		sportelliStatsPtr = shmat(sportelliStatShmId, NULL, SHM_RND);
		if (sportelliStatsPtr == (void *)-1)
		{
			slog(DIRETTORE, "direttore.pid.%d.shmat sportelli stat.failed", getpid());
			err_exit(strerror(errno));
		}

		slog(DIRETTORE, "direttore.pid.%d.waiting sportelli request of role", getpid());
		for (int i = 0; i < configuration.nofWorkerSeats; i++)
		{
			if (msgrcv(dirMsgQueueId, &msgBuff, sizeof(msgBuff) - sizeof(long), SPORTELLO_GROUP, 0) == -1)
			{
				slog(DIRETTORE, "direttore.pid.%d.msgrcv.from sportello group.failed!", getpid());
				err_exit(strerror(errno));
			}
			slog(DIRETTORE, "direttore.pid.%d.received request role from sportello.pid.%d", getpid(), msgBuff.payload.senderPid);
			int servizio;
			for (;;)
			{
				servizio = rand() % NUMBER_OF_SERVICES;
				slog(DIRETTORE, "direttore.pid.%d.servizio: %d", getpid(), servizio);
				if (servicesPtr[servizio].available)
				{
					slog(DIRETTORE, "direttore.pid.%d.service: %s.available", getpid(), servicesPtr[servizio].name);
					strcpy(msgBuff.payload.msg, servicesPtr[servizio].name);
					msgBuff.mtype = msgBuff.payload.senderPid;
					msgBuff.payload.senderPid = getpid();
					if (msgsnd(dirMsgQueueId, &msgBuff, sizeof(msgBuff) - sizeof(long), 0) == -1)
					{
						slog(DIRETTORE, "direttore.pid.%d.msgsnd.to sportello group.failed!", getpid());
						err_exit(strerror(errno));
					}
					slog(DIRETTORE, "direttore.pid.%d.sent to role %s to sportello.pid.%d", getpid(), msgBuff.payload.msg, msgBuff.mtype);
					strcpy(sportelliPtr[i].serviceName, servicesPtr[servizio].name);
					sportelliPtr[i].sportelloPid = msgBuff.mtype;
					sportelliPtr[i].operatorPid = 0;
					sportelliPtr[i].deskAvailable = 1;
					sportelliPtr[i].deskSemId = 0;
					sportelliPtr[i].deskSemun = 0;
					sportelliPtr[i].workerDeskSemId = 0;
					sportelliPtr[i].workerDeskSemun = 0;
					// sportello stats init
					strcpy(sportelliStatsPtr[i].service, servicesPtr[servizio].name);
					sportelliStatsPtr[i].pid = msgBuff.mtype;
					sportelliStatsPtr[i].ratio = 0.0;
					break;
				}
			}
		}
		// rilasciare semaforo
		if (release_sem(servicesShmSemId, 0) == -1)
		{
			slog(DIRETTORE, "direttore.pid.%d.release_sem.services shm sem.failed!", getpid());
			err_exit(strerror(errno));
		}
		// staccare la memoria condivisa
		if (shmdt(servicesPtr) == -1)
		{
			slog(DIRETTORE, "direttore.pid.%d.shmdt.services shm.failed!", getpid());
			err_exit(strerror(errno));
		}

		if (release_sem(sportelliShmSemId, 0) == -1)
		{
			slog(SPORTELLO, "direttore.pid.%d.release_sem.sportelli shm sem.failed!", getpid());
			err_exit(strerror(errno));
		}
		if (shmdt(sportelliPtr) == -1)
		{
			slog(SPORTELLO, "direttore.pid.%d.shmdt.sportelli shm.failed!", getpid());
			return -1;
		}

		if (release_sem(sportelliStatShmSemId, 0) == -1)
		{
			slog(SPORTELLO, "direttore.pid.%d.release_sem.sportelli stat shm sem.failed!", getpid());
			err_exit(strerror(errno));
		}
		if (shmdt(sportelliStatsPtr) == -1)
		{
			slog(SPORTELLO, "direttore.pid.%d.shmdt.sportelli stat shm.failed!", getpid());
			return -1;
		}

		// ensures that worker seats updates sportelli shm
		for (int i = 0; i < configuration.nofWorkerSeats; i++)
		{
			if (reserve_sem(sportelloSemId, 0) == -1)
			{
				slog(DIRETTORE, "direttore.pid.%d.reserve_sem.sportello sem.failed!", getpid());
				err_exit(strerror(errno));
			}
		}
		slog(DIRETTORE, "direttore.pid.%d.all sportello.updated sportelli shm", getpid());

		// ensures that workers start working on start of day
		for (int i = 0; i < configuration.nofWorkers; i++)
		{
			if (release_sem(operatoreSemId, 1) == -1)
			{
				slog(DIRETTORE, "direttore.pid.%d.release_sem.operatore sem.failed!", getpid());
				err_exit(strerror(errno));
			}
		}
		slog(DIRETTORE, "direttore.pid.%d.operatori started day", getpid());

		slog(DIRETTORE, "direttore.pid.%d.waiting operatori to update sportelli", getpid());
		// ensures that workers updates sportelli shm
		for (int i = 0; i < configuration.nofWorkers; i++)
		{
			if (reserve_sem(operatoreSemId, 2) == -1)
			{
				slog(DIRETTORE, "direttore.pid.%d.release_sem.operatore sem.failed!", getpid());
				err_exit(strerror(errno));
			}
		}
		slog(DIRETTORE, "direttore.pid.%d.all operatori.updated sportelli shm", getpid());
		print_services_in_shm(servicesShmId, servicesShmSemId);
		print_sportelli_in_shm(sportelliShmId, sportelliShmSemId, configuration.nofWorkerSeats);

		// ensures that erogatore start working on start of day
		slog(DIRETTORE, "direttore.pid.%d.releasing erogatore to start day", getpid());
		if (release_sem(erogatoreSemId, 1) == -1)
		{
			slog(DIRETTORE, "direttore.pid.%d.release_sem.erogatore sem.failed!", getpid());
			err_exit(strerror(errno));
		}
		slog(DIRETTORE, "direttore.pid.%d.released erogatore", getpid());

		if (reserve_sem(configurationSemId, 1) == -1)
		{
			slog(DIRETTORE, "direttore.pid.%d.failed to reserve config sem", getpid());
			err_exit(strerror(errno));
		}
		slog(DIRETTORE, "direttore.pid.reserve config sem.semid: %d - semun: 1", getpid(), configurationSemId);

		// ensures that users start on start of day
		for (int i = 0; i < users; i++)
		{
			if (release_sem(utenteSemId, 1) == -1)
			{
				slog(DIRETTORE, "direttore.pid.%d.release_sem.utente sem.failed!", getpid());
				err_exit(strerror(errno));
			}
		}

		slog(DIRETTORE, "direttore.pid.%d.sleeping 10s...", getpid());
		sleep(1);
		slog(DIRETTORE, "direttore.pid.%d.wake up", getpid());
		if (update_timeout(configurationSemId, days + 1) == -1)
		{
			slog(DIRETTORE, "direttore.pid.%d.failed to update timeout", getpid());
			err_exit(strerror(errno));
		}
		slog(DIRETTORE, "direttore.pid.%d.updated timeout", getpid());
		if (release_sem(configurationSemId, 1) == -1)
		{
			slog(DIRETTORE, "direttore.pid.%d.failed to release config sem", getpid());
			err_exit(strerror(errno));
		}
		slog(DIRETTORE, "direttore.pid.%d.released config sem.semid: %d - semun: 1", getpid(), configurationSemId);

		// allow erogatore to kill its child
		if (release_sem(erogatoreSemId, 3) == -1)
		{
			slog(DIRETTORE, "direttore.pid.%d.release_sem.erogatore sem.failed!", getpid());
			err_exit(strerror(errno));
		}
		slog(DIRETTORE, "direttore.pid.%d.released sem.erogatore_sem.semun:3", getpid());

		slog(DIRETTORE, "direttore.pid.%d.waiting erogatore to complete day", getpid());
		// ensure that erogatore has completed the day
		if (reserve_sem(erogatoreSemId, 2) == -1)
		{
			slog(DIRETTORE, "direttore.pid.%d.reserve_sem.erogatore sem.failed!", getpid());
			err_exit(strerror(errno));
		}

		slog(DIRETTORE, "direttore.pid.%d.notifying worker of eod...", getpid());
		// notify workers of end of day
		for (int i = 0; i < configuration.nofWorkers; i++)
		{
			if (release_sem(operatoreSemId, 3) == -1)
			{
				slog(DIRETTORE, "direttore.pid.%d.release_sem.operatore sem.failed!", getpid());
				err_exit(strerror(errno));
			}
		}
		slog(DIRETTORE, "direttore.pid.%d.notified worker of eod", getpid());
		slog(DIRETTORE, "direttore.pid.%d.waiting operatori to complete day...", getpid());
		// ensure that workers has completed the day
		for (int i = 0; i < configuration.nofWorkers; i++)
		{
			if (reserve_sem(operatoreSemId, 4) == -1)
			{
				slog(DIRETTORE, "direttore.pid.%d.reserve_sem.operatore sem.failed!", getpid());
				err_exit(strerror(errno));
			}
		}
		slog(DIRETTORE, "direttore.pid.%d.operatori completed day", getpid());

		// ensure that users has completed the day
		slog(DIRETTORE, "direttore.pid.%d.waiting users at desk to complete day", getpid());
		for (int i = 0; i < users; i++)
		{
			if (reserve_sem(utenteSemId, 2) == -1)
			{
				slog(DIRETTORE, "direttore.pid.%d.reserve_sem.utente sem.failed!", getpid());
				err_exit(strerror(errno));
			}
		}
		slog(DIRETTORE, "direttore.pid.%d.waiting sportelli on sem: %d semun: 1 to init sportello", getpid(), sportelloSemId);
		for (int i = 0; i < configuration.nofWorkerSeats; i++)
		{
			if (release_sem(sportelloSemId, 1) == -1)
			{
				slog(DIRETTORE, "direttore.pid.%d.reserve_sem.sportello sem.failed!", getpid());
				err_exit(strerror(errno));
			}
		}

		slog(DIRETTORE, "direttore.pid.%d.waiting sportelli on sem: %d semun: 2 to complete day", getpid(), sportelloSemId);
		for (int i = 0; i < configuration.nofWorkerSeats; i++)
		{
			if (reserve_sem(sportelloSemId, 2) == -1)
			{
				slog(DIRETTORE, "direttore.pid.%d.reserve_sem.sportello sem.failed!", getpid());
				err_exit(strerror(errno));
			}
		}
		slog(DIRETTORE, "direttore.pid.%d.sportelli completed day", getpid());

		if (reserve_sem(servicesShmSemId, 0) == -1)
		{
			slog(DIRETTORE, "direttore.pid.%d.reserve_sem.services shm sem.failed!", getpid());
			err_exit(strerror(errno));
		}
		slog(DIRETTORE, "direttore.pid.%d.daily stats", getpid());
		print_stats(statisticsShmId, statsShmSemId, sportelliStatShmId, sportelliStatShmSemId, days + 1, configuration.nofWorkerSeats);
		dump_daily_stats("./daily_stats.csv", "./extra_daily_stats.csv", statisticsShmId, statsShmSemId, days + 1);
		dump_operator_daily_ratio("./operator_ratio.csv", sportelliStatShmId, sportelliStatShmSemId, days + 1, configuration.nofWorkerSeats);
		reset_statistics(statisticsShmId, statsShmSemId);

		if (release_sem(servicesShmSemId, 0) == -1)
		{
			slog(DIRETTORE, "direttore.pid.%d.release_sem.services shm sem.failed!", getpid());
			err_exit(strerror(errno));
		}
		if (msgrcv(dirMsgQueueId, &msgBuff, sizeof(msgBuff) - sizeof(long), DIRETTORE_GROUP, IPC_NOWAIT) == -1)
		{
			if (errno != ENOMSG)
			{
				slog(DIRETTORE, "direttore.pid.%d.msgrcv.from DIRETTORE_GROUP group.failed!", getpid());
				err_exit(strerror(errno));
			}
			slog(DIRETTORE, "direttore.pid.%d.msgrcv.director didn't receive increase of users", getpid());
		}
		else
		{
			if (strcmp(msgBuff.payload.msg, ADD) == 0)
			{
				slog(DIRETTORE, "direttore.pid.%d.received instruction from %d to increase users", getpid(), msgBuff.payload.senderPid);
				create_utente(configuration.nNewUsers);
				users += configuration.nNewUsers;
				slog(DIRETTORE, "direttore.pid.%d.updated users count: %d", getpid(), users);
			}
		}
		if (reserve_sem(configurationSemId, 1) == -1)
		{
			slog(DIRETTORE, "direttore.pid.%d.failed to reserve config sem", getpid());
			err_exit(strerror(errno));
		}
		slog(DIRETTORE, "direttore.pid.reserve config sem.semid: %d - semun: 1", getpid(), configurationSemId);

		if (get_explode(configurationSemId) >= configuration.explodeThreshold)
		{
			if (release_sem(configurationSemId, 1) == -1)
			{
				slog(DIRETTORE, "direttore.pid.%d.failed to release config sem", getpid());
				err_exit(strerror(errno));
			}
			slog(DIRETTORE, "direttore.pid.%d.released config sem.semid: %d - semun: 1", getpid(), configurationSemId);

			break;
		}

		if (release_sem(configurationSemId, 1) == -1)
		{
			slog(DIRETTORE, "direttore.pid.%d.failed to release config sem", getpid());
			err_exit(strerror(errno));
		}
		slog(DIRETTORE, "direttore.pid.%d.released config sem.semid: %d - semun: 1", getpid(), configurationSemId);

		days++;
	}
	// TODO remove for gentle resources cancellation
	dump_total_stats("./total_stats.csv", "./extra_total_stats.csv", statisticsShmId, statsShmSemId);
	slog(DIRETTORE, "direttore.pid.%d.removing ipc resources", getpid());
	delete_ipc_resources(resourceCreationSemId, "sem");
	delete_ipc_resources(erogatoreSemId, "sem");
	delete_ipc_resources(utenteSemId, "sem");
	delete_ipc_resources(operatoreSemId, "sem");
	delete_ipc_resources(sportelloSemId, "sem");
	delete_ipc_resources(servicesShmSemId, "sem");
	delete_ipc_resources(statsShmSemId, "sem");
	delete_ipc_resources(sportelliShmSemId, "sem");
	delete_ipc_resources(sportelliStatShmSemId, "sem");
	delete_ipc_resources(configurationSemId, "sem");
	delete_ipc_resources(servicesShmId, "shm");
	delete_ipc_resources(statisticsShmId, "shm");
	delete_ipc_resources(sportelliShmId, "shm");
	delete_ipc_resources(sportelliStatShmId, "shm");
	delete_ipc_resources(dirMsgQueueId, "msg");
	delete_ipc_resources(serviceMsgqId, "msg");
	delete_ipc_resources(ticketsMsgQueueId, "msg");
	if (days >= configuration.simDuration)
	{
		printf("SIMULATION ENDED FOR TIMEOUT\n");
	}
	else
	{
		printf("SIMULATION ENDED FOR EXPLODE THRESHOLD\n");
	}
}

void create_erogatore_ticket()
{
	slog(DIRETTORE, "direttore.pid.%d.create_erogatore_ticket", getpid());
	pid_t pid;
	int waitStatus;
	pid = fork();
	if (pid == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.create_sportello.fork.failed!.interupting simulation", getpid());
		err_exit(strerror(errno));
	}
	if (pid == 0)
	{
		slog(DIRETTORE, "direttore.child.pid.%d.create_erogatore_ticket", getpid());
		char *const argv[] = {"./erogatore_ticket", NULL};
		char *const env[] = {NULL};
		if (execve("./erogatore_ticket", argv, env) == -1)
		{
			err_exit(strerror(errno));
		}
		exit(EXIT_SUCCESS);
	}
}

void create_sportello(int nofWorkerSeats)
{
	slog(DIRETTORE, "direttore.pid.%d.create_sportello.nofWorkerSeats:%d", getpid(), nofWorkerSeats);
	pid_t pid;
	int counter = 0;
	while (counter < nofWorkerSeats)
	{
		pid = fork();
		if (pid == -1)
		{
			// TODO continuare o stoppare l'intera simulazione?
			slog(DIRETTORE, "direttore.pid.%d.create_sportello.fork.failed!.interupting simulation", getpid());
			err_exit(strerror(errno));
		}
		if (pid == 0)
		{
			slog(DIRETTORE, "direttore.child.pid.%d.create_sportello", getpid());
			char *const argv[] = {"./sportello", NULL};
			char *const env[] = {NULL};
			if (execve("./sportello", argv, env) == -1)
			{
				// TODO continuare o stoppare l'intera simulazione?
				slog(DIRETTORE, "direttore.child.pid.%d.create_sportello.execve.failed!", getpid());
				err_exit(strerror(errno));
			}
			exit(EXIT_SUCCESS);
		}
		counter++;
	}
}

void create_operatore(int nofWorkers)
{
	slog(DIRETTORE, "direttore.pid.%d.create_operatore.nofWorkers:%d", getpid(), nofWorkers);
	pid_t pid;
	int waitStatus;
	int counter = 0;
	while (counter < nofWorkers)
	{
		pid = fork();
		if (pid == -1)
		{
			// TODO continuare o stoppare l'intera simulazione?
			slog(DIRETTORE, "direttore.pid.%d.create_operatore.fork.failed!.interupting simulation", getpid());
			err_exit(strerror(errno));
		}
		if (pid == 0)
		{
			slog(DIRETTORE, "direttore.child.pid.%d.create_operatore", getpid());
			char *const argv[] = {"./operatore", NULL};
			char *const env[] = {NULL};
			if (execve("./operatore", argv, env) == -1)
			{
				// TODO continuare o stoppare l'intera simulazione?
				slog(DIRETTORE, "direttore.child.pid.%d.operatore.execve.failed!", getpid());
				err_exit(strerror(errno));
			}
			exit(EXIT_SUCCESS);
		}
		counter++;
	}
}

void create_utente(int nofUsers)
{
	slog(DIRETTORE, "direttore.pid.%d.create_utente.nofUsers:%d", getpid(), nofUsers);
	pid_t pid;
	int waitStatus;
	int counter = 0;
	while (counter < nofUsers)
	{
		pid = fork();
		if (pid == -1)
		{
			// TODO continuare o stoppare l'intera simulazione?
			slog(DIRETTORE, "direttore.pid.%d.create_utente.fork.failed!.interupting simulation", getpid());
			err_exit(strerror(errno));
		}
		if (pid == 0)
		{
			slog(DIRETTORE, "direttore.child.pid.%d.create_utente", getpid());
			char *const argv[] = {"./utente", NULL};
			char *const env[] = {NULL};
			if (execve("./utente", argv, env) == -1)
			{
				// TODO continuare o stoppare l'intera simulazione?
				slog(DIRETTORE, "direttore.child.pid.%d.utente.execve.failed!", getpid());
				err_exit(strerror(errno));
			}
			exit(EXIT_SUCCESS);
		}
		counter++;
	}
}

int writeServicesInShm(int shmId, int semId)
{
	slog(DIRETTORE, "direttore.pid.%d.writeServicesInShm.start", getpid());
	ServizioAdtPtr servicesPtr = shmat(shmId, NULL, SHM_RND);
	if (servicesPtr == (void *)-1)
	{
		slog(DIRETTORE, "direttore.pid.%d.shmat.services shm.failed!", getpid());
		return -1;
	}
	for (size_t i = 0; i < NUMBER_OF_SERVICES; i++)
	{
		if (strcmp(services[i], IRP) == 0)
		{
			strcpy(servicesPtr[i].name, IRP);
			servicesPtr[i].temp = 10;
			servicesPtr[i].available = 1;
		}
		else if (strcmp(services[i], ILR) == 0)
		{
			strcpy(servicesPtr[i].name, ILR);
			servicesPtr[i].temp = 8;
			servicesPtr[i].available = 1;
		}
		else if (strcmp(services[i], PVB) == 0)
		{
			strcpy(servicesPtr[i].name, PVB);
			servicesPtr[i].temp = 6;
			servicesPtr[i].available = 1;
		}
		else if (strcmp(services[i], PBP) == 0)
		{
			strcpy(servicesPtr[i].name, PBP);
			servicesPtr[i].temp = 8;
			servicesPtr[i].available = 1;
		}
		else if (strcmp(services[i], APF) == 0)
		{
			strcpy(servicesPtr[i].name, APF);
			servicesPtr[i].temp = 20;
			servicesPtr[i].available = 1;
		}
		else
		{
			strcpy(servicesPtr[i].name, AOB);
			servicesPtr[i].temp = 20;
			servicesPtr[i].available = 1;
		}
	}
	if (release_sem(semId, 0) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.release_sem.services shm sem.failed!", getpid());
		return -1;
	}

	if (shmdt(servicesPtr) == -1)
	{
		slog(DIRETTORE, "direttore.pid.%d.shmdt.services shm.failed!", getpid());
		return -1;
	}
	slog(DIRETTORE, "direttore.pid.%d.writeServicesInShm.end", getpid());
	return 0;
}
