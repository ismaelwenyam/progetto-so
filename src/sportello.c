#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>


#include "simerr.h"
#include "logapi.h"
#include "msgapi.h"
#include "shmapi.h"
#include "semapi.h"
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
		
		if (release_sem(servicesShmSemId, 0) == -1){
			//TODO log error message and exit
		}
		slog(SPORTELLO, "sportello.pid.%d.released sem to update services shm", getpid());
		
		if (reserve_sem(sportelloSemId, 0) == -1){

			//TODO 
		}

		days++;
	}
	
	
}
