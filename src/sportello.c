#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>


#include "simerr.h"
#include "logapi.h"
#include "msgapi.h"
#include "services.h"
#include "shmapi.h"
#include "semapi.h"
#include "ticket.h"
#include "simulation_configuration.h"


int main (int argc, char **argv) {
	ConfigurationAdt configuration = get_config();
	slog(SPORTELLO, "sportello.pid.%d", getpid());	
	slog(SPORTELLO, "sportello.pid.%d.init initialization", getpid());
	int dirMsgQueueId, sportelloSemId, operatoreSportelloSemId;
	int servicesShmId, servicesShmSemId;
	if ((dirMsgQueueId = msgget(DIR_MESSAGE_QUEUE_KEY, 0)) == -1){
		slog(SPORTELLO, "sportello.pid.%d.msgget.dir msg queue.failed!", getpid());
		err_exit(strerror(errno));
	}
	if ((sportelloSemId = semget(SPORTELLO_SYNC_SEM, 0, 0)) == -1){
		slog(SPORTELLO, "sportello.pid.%d.semget.sportello sync sem.failed!", getpid());
		err_exit(strerror(errno)); 
	}
	if ((operatoreSportelloSemId = semget(IPC_PRIVATE, 1, S_IWUSR | S_IRUSR)) == -1){
		slog(SPORTELLO, "sportello.pid.%d.semget.ipc_private.operatoreSportello sem.failed", getpid());
		err_exit(strerror(errno));
	}
	if (init_sem_available(operatoreSportelloSemId, 0) == -1){
		//TODO 
	}
	if ((servicesShmSemId = semget(STATS_SHM_SEM_KEY, 0, 0)) == -1){
		slog(SPORTELLO, "sportello.pid.%d.semget.services shared memory sem.failed!", getpid());
		err_exit(strerror(errno)); 
	}
	if ((servicesShmId = shmget(SERVICE_SHARED_MEMORY, 0, 0)) == -1){
		slog(SPORTELLO, "sportello.pid.%d.shmget.services shared memory failed", getpid());
		err_exit(strerror(errno));
	}

	int days = 0;
	MsgBuff msgBuff;
	SportelloPtr sportello;
	while (days < configuration.simDuration){
		
		slog(SPORTELLO, "sportello.pid.%d.requesting role", getpid());
		msgBuff.mtype = DIRETTORE_GROUP;
		msgBuff.payload.senderPid = getpid();
		strcpy(msgBuff.payload.msg, ROLE);
		if (msgsnd(dirMsgQueueId, &msgBuff, sizeof(msgBuff) - sizeof(long), 0) == -1){
			slog(SPORTELLO, "sportello.pid.%d.msgsnd.failed to request role for day", getpid());
			//TODO
			err_exit(strerror(errno));
		}
		if (msgrcv(dirMsgQueueId, &msgBuff, sizeof(msgBuff) - sizeof(long), getpid(), 0) == -1){
			slog(SPORTELLO, "sportello.pid.%d.msgrcv.failed to receive role for day", getpid());
			//TODO
			err_exit(strerror(errno));
		}
		slog(SPORTELLO, "sportello.pid.%d.received role: %s", getpid(), msgBuff.payload.msg);
		//aggiornare memoria condivisa con operatoreSportelloSemId 
		slog(SPORTELLO, "sportello.pid.%d.reserving sem to update services shm", getpid());
		if (reserve_sem(servicesShmSemId, 0) == -1){
			//TODO log error message and exit
		}
		
		slog(SPORTELLO, "sportello.pid.%d.updating services shm", getpid());
		errno = 0;
		TicketAdtPtr ticketsPtr = shmat(servicesShmId, NULL, SHM_RND);
		if (errno != 0){
			slog(SPORTELLO, "sportello.pid.%d.shmat.services shm.failed!", getpid());
			if (release_sem(servicesShmSemId, 0) == -1){
				//TODO log error message and exit
			}
			err_exit(strerror(errno));
		}
		slog(SPORTELLO, "sportello.pid.%d.updating services shm...", getpid());
		for (int i = 0; i < NUMBER_OF_SERVICES; i++){
			if (strcmp(ticketsPtr[i].servizio, msgBuff.payload.msg) == 0){
				sportello = mkSportello(0, true, operatoreSportelloSemId, 0, 0, 0);
				if (sportello == NULL){
					slog(SPORTELLO, "sportello.pid.%d.couldn't istantiate sportello", getpid());	
					if (release_sem(servicesShmSemId, 0) == -1){
						//TODO log error message and exit
					}
					err_exit(strerror(errno));
				}
				if (!addSportello(ticketsPtr[i].sportelli, sportello)){
					slog(SPORTELLO, "utente.pid.%d.failed to add sportello in set", getpid());
					if (release_sem(servicesShmSemId, 0) == -1){
						//TODO log error message and exit
					}
					err_exit(strerror(errno));
				}
				break;
			}
		}
		slog(SPORTELLO, "sportello.pid.%d.updated services", getpid());
		
		if (release_sem(servicesShmSemId, 0) == -1){
			//TODO log error message and exit
		}
		slog(SPORTELLO, "sportello.pid.%d.released sem to update services shm", getpid());
		
		if (shmdt(ticketsPtr) == -1){
			slog(DIRETTORE, "direttore.pid.%d.shmdt.services shm.failed!", getpid());
			return -1;
		}
		
		if (reserve_sem(sportelloSemId, 0) == -1){

			//TODO 
		}

		days++;
	}
	
	
}
