#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>



#include "simerr.h"
#include "logapi.h"
#include "ticket.h"
#include "semapi.h"
#include "msgapi.h"
#include "shmapi.h"
#include "simulation_configuration.h"
#include "simulation_stats.h"

#define P_PAUSE_MIN	0
#define P_PAUSE_MAX	10

int main (int argc, char **argv){
	ConfigurationAdt configuration = get_config();
	slog(OPERATORE, "operatore.pid.%d", getpid());	
	slog(OPERATORE, "operatore.pid.%d.start initialization...", getpid());
	int operatoreSemId, serviceMsgqId, statsShmSemId, servicesShmSemId, dirMsgQueueId;
	int servicesShmId, statisticsShmId;
	if ((serviceMsgqId = msgget(SERVICE_MESSAGE_QUEUE_KEY, 0)) == -1){
		slog(OPERATORE, "operatore.pid.%d.msgget.ipc_creat.services message queue.failed!", getpid());
		err_exit(strerror(errno));
	}
	if ((dirMsgQueueId = msgget(DIR_MESSAGE_QUEUE_KEY, 0)) == -1){
		slog(OPERATORE, "operatore.pid.%d.msgget.dir msg queue.failed!", getpid());
		err_exit(strerror(errno));
	}
	if ((statsShmSemId = semget(STATS_SHM_SEM_KEY, 0, 0)) == -1){
		slog(OPERATORE, "operatore.pid.%d.semget.statistics shared memory sem.failed!", getpid());
		err_exit(strerror(errno)); 
	}
	if ((statisticsShmId = shmget(STATISTICS_SHARED_MEMORY, 0, 0)) == -1){
		slog(DIRETTORE, "operatore.pid.%d.shmget.statistics shared memory failed", getpid());
		err_exit(strerror(errno));
	}
	if ((servicesShmSemId = semget(STATS_SHM_SEM_KEY, 0, 0)) == -1){
		slog(OPERATORE, "operatore.pid.%d.semget.services shared memory sem.failed!", getpid());
		err_exit(strerror(errno)); 
	}
	if ((servicesShmId = shmget(SERVICE_SHARED_MEMORY, 0, 0)) == -1){
		slog(OPERATORE, "operatore.pid.%d.shmget.services shared memory failed", getpid());
		err_exit(strerror(errno));
	}
	if ((operatoreSemId = semget(OPERATORE_SYNC_SEM, 0, 0)) == -1){
		slog(OPERATORE, "operatore.pid.%d.semget.operatore sync sem.failed!", getpid());
		err_exit(strerror(errno)); 
	}
	
	//TODO richiesta funzione a direttore
	char text [MSG_LEN];	
	MsgBuff msgBuff;
	msgBuff.mtype = OPERATORE_GROUP;
	msgBuff.payload.senderPid = getpid();
	if (msgsnd(dirMsgQueueId, &msgBuff, sizeof(msgBuff) - sizeof(long), 0) == -1){
		slog(OPERATORE, "operatore.pid.%d.msgsnd.requesting role for simulation.failed!", getpid());
		err_exit(strerror(errno));
	}
	if (msgrcv(dirMsgQueueId, &msgBuff, sizeof(msgBuff) - sizeof(long), getpid(), 0) == -1){
		slog(OPERATORE, "operatore.pid.%d.msgsnd.requesting role for simulation.failed!", getpid());
		err_exit(strerror(errno));
	}
	slog(OPERATORE, "operatore.pid.%d.msgrcv.role: %s", getpid(), msgBuff.payload.msg);
	int days = 0;
	slog(OPERATORE, "operatore.pid.%d.completed initialization", getpid());
	//slog(EROGATORE, "erogatore_ticket.pid.%d.waiting for simulation to start", getpid());	
	if (release_sem(operatoreSemId, 0) == -1){
			//TODO log error message and exit
	}
	while (days < configuration.simDuration){
		
		if (reserve_sem(operatoreSemId, 1) == -1){
			//TODO log error message and exit
		}
		slog(OPERATORE, "operatore.pid.%d.day: %d", getpid(), days+1);


		
		if (release_sem(operatoreSemId, 2) == -1){
			//TODO log error message and exit
		}

		days++;
	}	
	
	
}
