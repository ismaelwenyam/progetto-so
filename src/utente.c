#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>


#include "simerr.h"
#include "logapi.h"
#include "services.h"
#include "ticket.h"
#include "semapi.h"
#include "shmapi.h"
#include "msgapi.h"
#include "simulation_configuration.h"

int main (int argc, char **argv){
	srand(time(NULL) + getpid());
	slog(UTENTE, "utente.pid.%d", getpid());	
	slog(UTENTE, "utente.pid.%d.start initialization...", getpid());
	ConfigurationAdt configuration = get_config();
	char *services[] = {IRP, ILR, PVB, PBP, APF, AOB};
	int resourceCreationSemId, utenteSemId, ticketsMsgQueueId, serviceMsgqId;	
	if ((resourceCreationSemId = semget(RESOURCE_CREATION_SYNC_SEM_KEY, 0, 0)) == -1){
		slog(UTENTE, "utente.pid.%d.semget.erogatore_sync_sem.failed!", getpid());
		err_exit(strerror(errno));
	}
	if (reserve_sem(resourceCreationSemId, 0) == -1){
		slog(UTENTE, "utente.pid.%d.reserve_sem.resource creation sem.failed!", getpid());
		err_exit(strerror(errno));
	}	
	if ((ticketsMsgQueueId = msgget(TICKET_MESSAGE_QUEUE_KEY, 0)) == -1){
		slog(UTENTE, "utente.pid.%d.msgget.tickets message queue.failed!", getpid());
		err_exit(strerror(errno));
	}
	if (release_sem(resourceCreationSemId, 0) == -1){
		slog(UTENTE, "utente.pid.%d.release_sem.resource creation sem.failed!", getpid());
		err_exit(strerror(errno));
	}	
	if ((serviceMsgqId = msgget(SERVICE_MESSAGE_QUEUE_KEY, 0)) == -1){
		slog(OPERATORE, "operatore.pid.%d.msgget.ipc_creat.services message queue.failed!", getpid());
		err_exit(strerror(errno));
	}
	if ((utenteSemId = semget(UTENTE_SYNC_SEM, 0, 0)) == -1){
		slog(UTENTE, "utente.pid.%d.semget.utente_sync_sem.failed!", getpid());
		err_exit(strerror(errno));
	}

	int days = 0;
	int range, pServ;
	int nRequests = configuration.nRequests, requests;
	if (nRequests > NUMBER_OF_SERVICES){
		slog(UTENTE, "utente.pid.%d.nRequests > total available services.setting nRequests to max", getpid());
		nRequests = NUMBER_OF_SERVICES;
	}
	char **servicesList = malloc(sizeof(char) * 4 * nRequests);
	if (servicesList == NULL){
		slog(UTENTE, "utente.pid.%d.malloc.failed", getpid());
		err_exit(strerror(errno));
	}
	TicketRequestAdt ticketRequest;
	TicketAdt ticket;
	slog(UTENTE, "utente.pid.%d.live waiting simulation to start", getpid());
	MsgBuff msgBuff;
	slog(UTENTE, "utente.pid.%d.completed initialization", getpid());
	if (release_sem(utenteSemId, 0) == -1){
			slog(UTENTE, "utente.pid.%d.release_sem.utente_sync_sem.semun.0.failed!", getpid());
			err_exit(strerror(errno));
	}
	while (days < configuration.simDuration){
		//slog(UTENTE, "utente.pid.%d.waiting start of day: %d", getpid(), days+1);
		if (reserve_sem(utenteSemId, 1) == -1){
			slog(UTENTE, "utente.pid.%d.utenteSem.reserve_sem(%d, 1).failed!", getpid(), utenteSemId);
			err_exit(strerror(errno));
		}
		slog(UTENTE, "utente.pid.%d.day: %d", getpid(), days+1);
		range = (configuration.pServMin + configuration.pServMax) / 2 - 1;
		pServ = configuration.pServMin + rand() % (configuration.pServMax - configuration.pServMin);
		slog(UTENTE, "utente.pid.%d.pserv:%d", getpid(), pServ);
		if (pServ < range){
			slog(UTENTE, "utente.pid.%d.not going to post office", getpid());
			if (release_sem(utenteSemId, 2) == -1){
				slog(UTENTE, "utente.pid.%d.utenteSem.release_sem(%d, 2).failed", getpid(), utenteSemId);
				err_exit(strerror(errno));
			}
			days++;
			continue;
		}
		slog(UTENTE, "utente.pid.%d.going to post office", getpid());
		
		// scelta della lista di servizi di cui l'utente necessita
		requests = rand() % nRequests + 1;
		for (int i = 0; i < requests; i++){
			int serviceChoice = rand() % NUMBER_OF_SERVICES;
			servicesList[i] = services[serviceChoice];
		}
		slog(UTENTE, "utente.pid.%d.desired services", getpid());
		for (int i = 0; i < requests; i++){
			slog(UTENTE, "%s", servicesList[i]);
		}
		//TODO stabilire orario in cui recarsi all'ufficio postale
	
		// richiedere tickets per i servizi	
		for (int i = 0; i < requests; i++){
			msgBuff.mtype = UTENTE_GROUP;
			msgBuff.payload.senderPid = getpid();
			strcpy(msgBuff.payload.msg, servicesList[i]);
			slog(UTENTE, "utente.pid.%d.sending ticket request for %s...", getpid(), servicesList[i]);
			
			if (msgsnd(ticketsMsgQueueId, &msgBuff, sizeof(msgBuff) - sizeof(long), 0) == -1){
				slog(UTENTE, "utente.pid.%d.msgsnd.ticket request.failed!", getpid());
				if (errno == EIDRM){
					slog(UTENTE, "utente.pid.%d.tickets msg queue removed.exiting", getpid());
					err_exit(strerror(errno));
				}
				slog(UTENTE, "utente.pid.%d.proceding to request next service ticket", getpid());
				continue;
			}
			slog(UTENTE, "utente.pid.%d.sent ticket request for %s", getpid(), servicesList[i]);
			if (msgrcv(ticketsMsgQueueId, &ticketRequest, sizeof(ticketRequest) - sizeof(long), getpid(), 0) == -1){
				slog(UTENTE, "utente.pid.%d.msgrcv.ticket request for service: %s.failed!", servicesList[i], getpid());
				continue;
			}
			slog(UTENTE, "utente.pid.%d.ticket received", getpid());
			if (ticketRequest.ticket.eod){
				slog(UTENTE, "utente.pid.%d.received: eod", getpid());
				break;
			}
			if (!ticketRequest.ticket.serviceAvailable){
				slog(UTENTE, "utente.pid.%d.service %s not available on day %d", getpid(), servicesList[i], days + 1);
				continue;
			}
			slog(UTENTE, "utente.pid.%d.received ticket for service %s", getpid(), servicesList[i]);
			Sportello sportello = ticketRequest.ticket.sportello;	
			// Il tempo di erogazione del servizio viene calcolato dal momento in cui l'utente si mette in fila.
			// TODO calcolare tempo si start e inserirlo nelle statistiche (tenendo conto del parametro n_nano_secs) (tempo di attesa, erogazione)
			if (reserve_sem(sportello.workerDeskSemId, sportello.workerDeskSemun) == -1){
				slog(UTENTE, 
					"utente.pid.%d.couldn't get in queue of sportello(operatore sem).semid.%d.semun.%d", 
					getpid(), sportello.workerDeskSemId, sportello.workerDeskSemun);
				if (requests > 1)
					slog(UTENTE, "utente.pid.%d.proceding to request next service ticket", getpid());
				// TODO calcolare tempo di end e inserirlo nelle statistiche (tempo di attesa)
				continue;
			}
			// TODO calcolare tempo di end e inserirlo nelle statistiche (tempo di attesa)
			msgBuff.mtype = sportello.operatorPid;
			msgBuff.payload.senderPid = getpid();
			strcpy(msgBuff.payload.msg, servicesList[i]);
			slog(UTENTE, "utente.pid.%d.sending request to operato", getpid());
			if (msgsnd(serviceMsgqId, &msgBuff, sizeof(msgBuff) - sizeof(long), 0) == -1){
				slog(UTENTE, "utente.pid.%d.msgsnd.service msgq to operator %d.failed!", getpid(), sportello.operatorPid);
				// TODO calcolare tempo di end e inserirlo nelle statistiche (tempo di erogazione servizio)
				continue;
			} 
			
			if (msgrcv(serviceMsgqId, &msgBuff, sizeof(msgBuff) - sizeof(long), getpid(), 0) == -1){
				slog(UTENTE, "utente.pid.%d.msgrcv.service msgq from operato %d.failed!", getpid(), sportello.operatorPid);
				// TODO calcolare tempo di end e inserirlo nelle statistiche (tempo di erogazione servizio)
				continue;
			} 
			
			// TODO calcolare tempo di end e inserirlo nelle statistiche (tempo di erogazione servizio)
		}

		if (release_sem(utenteSemId, 2) == -1){
			slog(UTENTE, "utente.pid.%d.utenteSem.release_sem(%d, 2).failed", getpid(), utenteSemId);
			err_exit(strerror(errno));
		}
		days++;
	}
	free(servicesList);
	
	
}
