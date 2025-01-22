#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>

#include "simerr.h"
#include "logapi.h"
#include "service_api.h"
#include "ticket.h"
#include "semapi.h"
#include "shmapi.h"
#include "msgapi.h"
#include "simulation_configuration.h"
#include "simulation_stats.h"

int sim_status(int configurationSemId, int simDuration, int explodeThreshold);

int main(int argc, char **argv)
{
	srand(time(NULL) + getpid());
	slog(UTENTE, "utente.pid.%d", getpid());
	slog(UTENTE, "utente.pid.%d.start initialization...", getpid());
	int configurationSemId;
	if ((configurationSemId = semget(CONFIGURATION_SEM_KEY, 0, 0)) == -1)
	{
		slog(UTENTE, "utente.pid.%d.semget.configuration sem.failed", getpid());
		err_exit(strerror(errno));
	}
	ConfigurationAdt configuration = get_config();
	// char *services[] = {IRP, ILR, PVB, PBP, APF, AOB};
	int resourceCreationSemId, utenteSemId, ticketsMsgQueueId, serviceMsgqId;
	int servicesShmSemId, servicesShmId;
	if ((resourceCreationSemId = semget(RESOURCE_CREATION_SYNC_SEM_KEY, 0, 0)) == -1)
	{
		slog(UTENTE, "utente.pid.%d.semget.erogatore_sync_sem.failed!", getpid());
		err_exit(strerror(errno));
	}
	if (reserve_sem(resourceCreationSemId, 0) == -1)
	{
		slog(UTENTE, "utente.pid.%d.reserve_sem.resource creation sem.failed!", getpid());
		err_exit(strerror(errno));
	}
	if ((ticketsMsgQueueId = msgget(TICKET_MESSAGE_QUEUE_KEY, 0)) == -1)
	{
		slog(UTENTE, "utente.pid.%d.msgget.tickets message queue.failed!", getpid());
		err_exit(strerror(errno));
	}
	if (release_sem(resourceCreationSemId, 0) == -1)
	{
		slog(UTENTE, "utente.pid.%d.release_sem.resource creation sem.failed!", getpid());
		err_exit(strerror(errno));
	}
	if ((serviceMsgqId = msgget(SERVICE_MESSAGE_QUEUE_KEY, 0)) == -1)
	{
		slog(UTENTE, "operatore.pid.%d.msgget.ipc_creat.services message queue.failed!", getpid());
		err_exit(strerror(errno));
	}
	if ((utenteSemId = semget(UTENTE_SYNC_SEM, 0, 0)) == -1)
	{
		slog(UTENTE, "utente.pid.%d.semget.utente_sync_sem.failed!", getpid());
		err_exit(strerror(errno));
	}

	if ((servicesShmSemId = semget(SERVICES_SHM_SEM_KEY, 0, 0)) == -1)
	{
		slog(UTENTE, "utente.pid.%d.semget.services_shm_sem.failed!", getpid());
		err_exit(strerror(errno));
	}

	if ((servicesShmId = shmget(SERVICE_SHARED_MEMORY, 0, 0)) == -1)
	{
		slog(UTENTE, "utente.pid.%d.shmget.service_shared_memory.failed!", getpid());
		err_exit(strerror(errno));
	}

	int days = 0;
	int range, pServ;
	int nRequests = configuration.nRequests, requests;
	if (nRequests > NUMBER_OF_SERVICES)
	{
		slog(UTENTE, "utente.pid.%d.nRequests > total available services.setting nRequests to max", getpid());
		nRequests = NUMBER_OF_SERVICES;
	}
	slog(UTENTE, "utente.pid.%d.nRequests: %d", getpid(), nRequests);
	char **servicesList = (char **)malloc(sizeof(char *) * nRequests);
	if (servicesList == NULL)
	{
		slog(UTENTE, "utente.pid.%d.malloc.failed", getpid());
		err_exit(strerror(errno));
	}
	TicketRequestAdt ticketRequest;
	TicketAdt ticket;
	MsgBuff msgBuff;
	ServizioAdtPtr servicesPtr;
	SportelloAdt sportello;
	struct timespec start, end;
	long elapsedTime;
	bool endOfSimulation = false;
	slog(UTENTE, "utente.pid.%d.completed initialization", getpid());
	if (release_sem(utenteSemId, 0) == -1)
	{
		slog(UTENTE, "utente.pid.%d.release_sem.utente_sync_sem.semun.0.failed!", getpid());
		err_exit(strerror(errno));
	}
	while (1)
	{
		if (reserve_sem(utenteSemId, 1) == -1)
		{
			slog(UTENTE, "utente.pid.%d.utenteSem.reserve_sem(%d, 1).failed!", getpid(), utenteSemId);
			err_exit(strerror(errno));
		}
		slog(UTENTE, "utente.pid.%d.day: %d", getpid(), days + 1);
		range = (configuration.pServMin + configuration.pServMax) / 2 - 1;
		pServ = configuration.pServMin + rand() % (configuration.pServMax - configuration.pServMin);
		slog(UTENTE, "utente.pid.%d.pserv:%d", getpid(), pServ);
		if (pServ < range)
		{
			slog(UTENTE, "utente.pid.%d.not going to post office", getpid());
			if (reserve_sem(configurationSemId, 1) == -1)
			{
				slog(UTENTE, "utente.pid.%d.failed to reserve config sem", getpid());
				err_exit(strerror(errno));
			}
			slog(UTENTE, "utente.pid.%d.reserved config sem.semdid: %d - semun: %d", getpid(), configurationSemId, 1);
			if (get_timeout(configurationSemId) >= configuration.simDuration || get_explode(configurationSemId) >= configuration.explodeThreshold)
			{
				if (release_sem(configurationSemId, 1) == -1)
				{
					slog(UTENTE, "utente.pid.%d.failed to release config sem", getpid());
					err_exit(strerror(errno));
				}
				slog(UTENTE, "utente.pid.%d.released config sem.semdid: %d - semun: %d", getpid(), configurationSemId, 1);
				if (release_sem(utenteSemId, 2) == -1)
				{
					slog(UTENTE, "utente.pid.%d.utenteSem.release_sem(%d, 2).failed", getpid(), utenteSemId);
					err_exit(strerror(errno));
				}
				slog(UTENTE, "utente.pid.%d.release_sem.utent_sem:%d.semun:2", getpid(), utenteSemId);

				break;
			}
			if (release_sem(configurationSemId, 1) == -1)
			{
				slog(UTENTE, "utente.pid.%d.failed to release config sem", getpid());
				err_exit(strerror(errno));
			}
			slog(UTENTE, "utente.pid.%d.released config sem.semdid: %d - semun: %d", getpid(), configurationSemId, 1);
			if (release_sem(utenteSemId, 2) == -1)
			{
				slog(UTENTE, "utente.pid.%d.utenteSem.release_sem(%d, 2).failed", getpid(), utenteSemId);
				err_exit(strerror(errno));
			}
			slog(UTENTE, "utente.pid.%d.release_sem.utent_sem:%d.semun:2", getpid(), utenteSemId);

			days++;
			continue;
		}
		slog(UTENTE, "utente.pid.%d.going to post office", getpid());

		if (reserve_sem(servicesShmSemId, 0) == -1)
		{
			slog(UTENTE, "utente.pid.%d.reserve_sem.services shm sem.failed!", getpid());
			err_exit(strerror(errno));
		}
		slog(UTENTE, "utente.pid.%d.reserved sem for services shared memory", getpid());
		servicesPtr = shmat(servicesShmId, NULL, SHM_RND);
		if (servicesPtr == (void *)-1)
		{
			slog(UTENTE, "utente.pid.%d.shmat.services shm.failed", getpid());
			err_exit(strerror(errno));
		}

		// scelta della lista di servizi di cui l'utente necessita
		requests = rand() % nRequests + 1;
		int notAvailableServicesCount = 0;
		slog(UTENTE, "utente.pid.%d.services number requested: %d", getpid(), requests);
		for (int i = 0, j = 0; i < requests; i++)
		{
			int serviceChoice = rand() % NUMBER_OF_SERVICES;
			printf("test i: %d - j: %d user - serviceChoice: %d - service: %s\n", i, j, serviceChoice, servicesPtr[serviceChoice].name);
			if (servicesPtr[serviceChoice].available)
			{
				printf("added: %s\n", strcpy(servicesList[j], servicesPtr[serviceChoice].name));
				j++;
			}
			else
			{
				slog(UTENTE, "utente.pid.%d.service %s not available today", getpid(), servicesPtr[serviceChoice].name);
				notAvailableServicesCount += 1;
			}
		}
		printf("not available services count : %d\n", notAvailableServicesCount);
		requests -= notAvailableServicesCount;
		if (release_sem(servicesShmSemId, 0) == -1)
		{
			slog(UTENTE, "utente.pid.%d.release_sem.services shm sem.failed!", getpid());
			err_exit(strerror(errno));
		}
		slog(UTENTE, "utente.pid.%d.released sem for services shared memory", getpid());
		if (shmdt(servicesPtr) == -1)
		{
			slog(UTENTE, "utente.pid.%d.shmdt.services shm.failed!", getpid());
			err_exit(strerror(errno));
		}

		slog(UTENTE, "utente.pid.%d.desired services.requests: %d", getpid(), requests);
		for (int i = 0; i < requests; i++)
		{
			slog(UTENTE, "index: %d - %s", i, servicesList[i]);
		}
		// TODO stabilire orario in cui recarsi all'ufficio postale

		// richiedere tickets per i servizi
		for (int i = 0; i < requests; i++)
		{
			msgBuff.mtype = EROGATORE_GROUP;
			msgBuff.payload.senderPid = getpid();
			strcpy(msgBuff.payload.msg, servicesList[i]);
			slog(UTENTE, "utente.pid.%d.sending ticket request for %s...", getpid(), servicesList[i]);

			if (msgsnd(ticketsMsgQueueId, &msgBuff, sizeof(msgBuff) - sizeof(long), 0) == -1)
			{
				slog(UTENTE, "utente.pid.%d.msgsnd.ticket request.failed!", getpid());
				if (errno == EIDRM)
				{
					slog(UTENTE, "utente.pid.%d.tickets msg queue removed.exiting", getpid());
					err_exit(strerror(errno));
				}
				slog(UTENTE, "utente.pid.%d.proceding to request next service ticket", getpid());
				continue;
			}
			slog(UTENTE, "utente.pid.%d.sent ticket request for %s", getpid(), servicesList[i]);
			if (msgrcv(ticketsMsgQueueId, &ticketRequest, sizeof(ticketRequest) - sizeof(long), getpid(), 0) == -1)
			{
				slog(UTENTE, "utente.pid.%d.msgrcv.ticket request for service: %s.failed!", servicesList[i], getpid());
				continue;
			}
			slog(UTENTE, "utente.pid.%d.ticket received", getpid());
			if (ticketRequest.ticket.eod)
			{
				slog(UTENTE, "utente.pid.%d.received.response from erogatore: eod", getpid());
				break;
			}
			slog(UTENTE, "utente.pid.%d.received ticket for service: %s", getpid(), servicesList[i]);
			sportello.operatorPid = ticketRequest.ticket.sp.operatorPid;
			sportello.workerDeskSemId = ticketRequest.ticket.sp.workerDeskSemId;
			sportello.workerDeskSemun = ticketRequest.ticket.sp.workerDeskSemun;
			slog(UTENTE, "utente.pid.%d.sportello: operatorPid: %d - workerDeskSemId: %d - workerDeskSemun: %d", getpid(), sportello.operatorPid, sportello.workerDeskSemId, sportello.workerDeskSemun);
			if (sportello.operatorPid == 0)
			{
				slog(UTENTE, "utente.pid.%d.service: %s available but no operator executing it", getpid(), servicesList[i]);
				continue;
			}
			// Il tempo di erogazione del servizio viene calcolato dal momento in cui l'utente si mette in fila.
			// TODO calcolare tempo si start e inserirlo nelle statistiche (tenendo conto del parametro n_nano_secs) (tempo di attesa) - citare CHATGPT
			clock_gettime(CLOCK_MONOTONIC, &start);
			if (reserve_sem(sportello.workerDeskSemId, sportello.workerDeskSemun) == -1)
			{
				slog(UTENTE,
					 "utente.pid.%d.couldn't get in queue of sportello(operatore sem).semid.%d.semun.%d",
					 getpid(), sportello.workerDeskSemId, sportello.workerDeskSemun);
				break;
				// TODO calcolare tempo di end e inserirlo nelle statistiche (tempo di attesa)
			}
			// TODO remove sleep
			sleep(1);
			// TODO calcolare tempo si start e inserirlo nelle statistiche (tenendo conto del parametro n_nano_secs) (tempo di attesa)
			clock_gettime(CLOCK_MONOTONIC, &end);
			elapsedTime = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
			slog(UTENTE, "utente.pid.%d.elapsed time: %.2ld", getpid(), elapsedTime);
			slog(UTENTE, "utente.pid.%d.reserve_sem.%d.semun.%d", getpid(), sportello.workerDeskSemId, sportello.workerDeskSemun);
			// TODO calcolare tempo di end e inserirlo nelle statistiche (tempo di attesa)
			msgBuff.mtype = sportello.operatorPid;
			msgBuff.payload.senderPid = getpid();
			strcpy(msgBuff.payload.msg, servicesList[i]);
			msgBuff.payload.temp = ticketRequest.ticket.se.temp;
			msgBuff.payload.elapsed = elapsedTime;
			slog(UTENTE, "utente.pid.%d.sending service: %s request to operatore", getpid(), msgBuff.payload.msg);
			if (msgsnd(serviceMsgqId, &msgBuff, sizeof(msgBuff) - sizeof(long), 0) == -1)
			{
				slog(UTENTE, "utente.pid.%d.msgsnd.service msgq to operatore %d.failed!", getpid(), sportello.operatorPid);
				continue;
			}
			slog(UTENTE, "utente.pid.%d.sent service to operatore.pid.%d", getpid(), sportello.operatorPid);
			strcpy(msgBuff.payload.msg, "");
			if (kill(sportello.operatorPid, 0) == 0)
			{
				slog(UTENTE, "utente.pid.%d.waiting response from operatore.pid.%d", getpid(), sportello.operatorPid);
				if (msgrcv(serviceMsgqId, &msgBuff, sizeof(msgBuff) - sizeof(long), getpid(), 0) == -1)
				{
					slog(UTENTE, "utente.pid.%d.failed to receive confirm from operator: %d", getpid(), sportello.operatorPid);
					err_exit(strerror(errno));
				}
			}

			// TODO attende il tempario
			if (release_sem(sportello.workerDeskSemId, sportello.workerDeskSemun) == -1)
			{
				slog(UTENTE,
					 "utente.pid.%d.couldn't release sportello(operatore sem).semid.%d.semun.%d",
					 getpid());
				if (requests > 1)
				{
					slog(UTENTE, "utente.pid.%d.proceding to request next service ticket", getpid());
					continue;
				}
				break;
			}
			if (strcmp(msgBuff.payload.msg, END_OF_DAY) == 0 || strcmp(msgBuff.payload.msg, "") == 0)
			{
				slog(UTENTE, "utente.pid.%d.received eod from operator.updating explode", getpid());
				if (reserve_sem(configurationSemId, 1) == -1)
				{
					slog(UTENTE, "utente.pid.%d.failed to reserve config sem", getpid());
					err_exit(strerror(errno));
				}
				slog(UTENTE, "utente.pid.%d.reserved config sem.semdid: %d - semun: %d", getpid(), configurationSemId, 1);
				if (update_explode(configurationSemId, 1) == -1)
				{
					if (release_sem(configurationSemId, 1) == -1)
					{
						slog(UTENTE, "utente.pid.%d.failed to release config sem", getpid());
						err_exit(strerror(errno));
					}
					slog(UTENTE, "utente.pid.%d.released config sem.semdid: %d - semun: %d", getpid(), configurationSemId, 1);

					slog(UTENTE, "utente.pid.%d.failed to update explode", getpid());
					err_exit(strerror(errno));
				}
				if (release_sem(configurationSemId, 1) == -1)
				{
					slog(UTENTE, "utente.pid.%d.failed to release config sem", getpid());
					err_exit(strerror(errno));
				}
				slog(UTENTE, "utente.pid.%d.released config sem.semdid: %d - semun: %d", getpid(), configurationSemId, 1);

				// TODO aggiornare explode.
				break;
			}
		}

		if (sim_status(configurationSemId, configuration.simDuration, configuration.explodeThreshold) == 1)
		{
			if (release_sem(utenteSemId, 2) == -1)
			{
				slog(UTENTE, "utente.pid.%d.utenteSem.release_sem(%d, 2).failed", getpid(), utenteSemId);
				err_exit(strerror(errno));
			}
			slog(UTENTE, "utente.pid.%d.release_sem.utent_sem:%d.semun:2", getpid(), utenteSemId);
			slog(UTENTE, "utente.%d.simulation over", getpid());
			break;
		}

		if (release_sem(utenteSemId, 2) == -1)
		{
			slog(UTENTE, "utente.pid.%d.utenteSem.release_sem(%d, 2).failed", getpid(), utenteSemId);
			err_exit(strerror(errno));
		}
		days++;
	}
	// slog(UTENTE, "utente.%d.simulation over", getpid());
	// TODO also free in errors
	free(servicesList);
}

int sim_status(int configurationSemId, int simDuration, int explodeThreshold)
{
	if (reserve_sem(configurationSemId, 1) == -1)
	{
		slog(UTENTE, "utente.pid.%d.failed to reserve config sem", getpid());
		err_exit(strerror(errno));
	}
	slog(UTENTE, "utente.pid.%d.reserved config sem.semdid: %d - semun: %d", getpid(), configurationSemId, 1);
	if (get_timeout(configurationSemId) >= simDuration || get_explode(configurationSemId) >= explodeThreshold)
	{
		if (release_sem(configurationSemId, 1) == -1)
		{
			slog(UTENTE, "utente.pid.%d.failed to release config sem", getpid());
			err_exit(strerror(errno));
		}
		slog(UTENTE, "utente.pid.%d.released config sem.semdid: %d - semun: %d", getpid(), configurationSemId, 1);
		return 1;
	}
	if (release_sem(configurationSemId, 1) == -1)
	{
		slog(UTENTE, "utente.pid.%d.failed to release config sem", getpid());
		err_exit(strerror(errno));
	}
	slog(UTENTE, "utente.pid.%d.released config sem.semdid: %d - semun: %d", getpid(), configurationSemId, 1);
	return 0;
}