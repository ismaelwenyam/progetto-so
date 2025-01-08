#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>



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

bool eod = false;

void signalHandler(int sig){
	slog(OPERATORE, "operatore.pid.%d.signal intercepted", getpid());
	if (sig == SIGTERM){
		eod = true;
		signal(sig, signalHandler);
	}
}

int main (int argc, char **argv){
	ConfigurationAdt configuration = get_config();
	slog(OPERATORE, "operatore.pid.%d", getpid());	
	slog(OPERATORE, "operatore.pid.%d.start initialization...", getpid());
	int operatoreSemId, serviceMsgqId, statsShmSemId, servicesShmSemId, dirMsgQueueId;
	int servicesShmId, statisticsShmId;
	int utenteOperatoreSemId;
	int sportelliShmSemId, sportelliShmId;
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
	//
	if ((sportelliShmSemId = semget(SPORTELLI_SHM_SEM_KEY, 0, 0)) == -1){
		slog(EROGATORE, "erogatore_ticket.pid.%d.semget.services_shm_sem.failed!", getpid());
		err_exit(strerror(errno));
	}	
	
	if ((sportelliShmId = shmget(SPORTELLI_SHARED_MEMORY_KEY, 0, 0)) == -1){
		slog(EROGATORE, "erogatore_ticket.pid.%d.shmget.service_shared_memory.failed!", getpid());
		err_exit(strerror(errno));
	}
	
	// richiesta funzione a direttore
	char role [MSG_LEN];	
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
	// creare semaforo privato per l'utente
	if ((utenteOperatoreSemId = semget(IPC_PRIVATE, 1, S_IWUSR | S_IRUSR)) == -1){
		slog(OPERATORE, "operatore.pid.%d.semget.ipc_private.failed!", getpid());
		err_exit(strerror(errno));
	}
	if (init_sem_available(utenteOperatoreSemId, 0) == -1){
		slog(OPERATORE, "operatore.pid.%d.init_sem_available.utente_operatore_sem.failed", getpid());
		err_exit(strerror(errno));
	}
	int deskSemId, deskSemun;
	slog(OPERATORE, "operatore.pid.%d.msgrcv.role: %s", getpid(), msgBuff.payload.msg);
	strcpy(role, msgBuff.payload.msg);
	int days = 0;
	pid_t pid;
	SportelloAdtPtr sportelliPtr;
	slog(OPERATORE, "operatore.pid.%d.completed initialization", getpid());
	if (release_sem(operatoreSemId, 0) == -1){
		slog(OPERATORE, "operatore.pid.%d.release_sem.operatore_sem.failed", getpid());
		err_exit(strerror(errno));
	}
	while (days < configuration.simDuration){
		
		if (reserve_sem(operatoreSemId, 1) == -1){
			slog(OPERATORE, "operatore.pid.%d.release_sem.operatore_sem.failed", getpid());
			err_exit(strerror(errno));
		}
		slog(OPERATORE, "operatore.pid.%d.day: %d", getpid(), days+1);
		pid = fork();
		if (pid == -1){
			slog(OPERATORE, "operatore.pid.%d.fork.failed!", getpid());
			err_exit(strerror(errno));
		}		
		if (pid == 0){		/* child code */
			signal(SIGTERM, signalHandler);
			slog(OPERATORE, "operatore.child.pid.%d.looking for desk", getpid());	
			if (reserve_sem(sportelliShmSemId, 0) == -1){
				slog(OPERATORE, "operatore.child.pid.%d.reserve_sem.sportelli shm sem.failed", getpid());
				err_exit(strerror(errno));
			}	
			slog(OPERATORE, "operatore.child.pid.%d.reserve_sem.sportelli shm sem.ok", getpid());
			errno = 0;
			sportelliPtr = shmat(sportelliShmId, NULL, SHM_RND);
			if (sportelliPtr == (void*)-1){
				slog(OPERATORE, "operatore.child.pid.%d.shmat.sportelli shm.failed!", getpid());
				if (release_sem(sportelliShmSemId, 0) == -1){
					slog(OPERATORE, "operatore.pid.%d.release_sem.sportelli_shm_sem.failed", getpid());
					err_exit(strerror(errno));
				}
				err_exit(strerror(errno));
			}
			slog(OPERATORE, "operatore.child.pid.%d.updating sportello with info...", getpid());
			bool sportelloFound = false;
			bool sportelloAvailable = false;
			for (int i = 0; i < configuration.nofWorkerSeats && !sportelloFound; i++){
				if (strcmp(sportelliPtr[i].serviceName, role) != 0) continue;
				//TODO migliorare la scelta dello sportello
				sportelloFound = true;
				if (sportelliPtr[i].deskAvailable){
					sportelloAvailable = true;
					sportelliPtr[i].operatorPid = getpid();
				}
				sportelliPtr[i].workerDeskSemId = utenteOperatoreSemId;
				sportelliPtr[i].workerDeskSemun = 0;
				deskSemId = sportelliPtr[i].deskSemId;
				deskSemun = sportelliPtr[i].deskSemun;
			}
			if (!sportelloFound){
				slog(OPERATORE, "operatore.child.pid.%d.sportello not found for service: %s", getpid(), msgBuff.payload.msg);
				if (release_sem(sportelliShmSemId, 0) == -1){
					slog(OPERATORE, "operatore.child.pid.%d.release_sem.services shm sem", getpid());
					err_exit(strerror(errno));
				}
				if (release_sem(operatoreSemId, 2) == -1){
					slog(OPERATORE, "operatore.pid.%d.reserve_sem.operatore sem.semun:2.failed", getpid());
					err_exit(strerror(errno));
				}
				slog(OPERATORE, "operatore.child.pid.%d.release_sem.sportelli_shm_sem", getpid());
				exit(0);
			}
				
			
			if (release_sem(sportelliShmSemId, 0) == -1){
				if (release_sem(operatoreSemId, 2) == -1){
					slog(OPERATORE, "operatore.pid.%d.reserve_sem.operatore sem.semun:2.failed", getpid());
					err_exit(strerror(errno));
				}
				slog(OPERATORE, "operatore.child.pid.%d.release_sem.services shm sem", getpid());
				err_exit(strerror(errno));
			}
			
			slog(OPERATORE, "operatore.child.pid.%d.release_sem.services shm sem", getpid());
			if (shmdt(sportelliPtr) == -1){
				slog(OPERATORE, "operatore.child.pid.%d.shmdt.failed!", getpid());
				err_exit(strerror(errno));
			}
			
			slog(OPERATORE, "operatore.child.pid.%d.updated services with info", getpid());
			if (reserve_sem(deskSemId, deskSemun) == -1){
				if (release_sem(operatoreSemId, 2) == -1){
					slog(OPERATORE, "operatore.pid.%d.reserve_sem.operatore sem.semun:2.failed", getpid());
					err_exit(strerror(errno));
				}
				slog(OPERATORE, "operatore.child.pid.%d.reserve_sem.deskSemId: %d - deskSemun: %d", getpid(), deskSemId, deskSemun);
				err_exit(strerror(errno));
			}
			slog(OPERATORE, "operatore.child.pid.%d.reserved.sportello.%d.semun.%d", getpid(), deskSemId, deskSemun);
			if (!sportelloAvailable){
				slog(OPERATORE, "operatore.child.pid.%d.sportello.%d.semun.%d.not available at the moment", getpid(), deskSemId, deskSemun);
				//TODO accedere alla memoria condivisa dei sportelli ed aggiornare l'operatorId
				// ricordarsi di rilasciare operatoreSemId, 2
			}
				
			if (release_sem(operatoreSemId, 2) == -1){
				slog(OPERATORE, "operatore.pid.%d.reserve_sem.operatore sem.semun:2.failed", getpid());
				err_exit(strerror(errno));
			}
			while (!eod){
				slog(OPERATORE, "operatore.child.pid.%d.waiting services requests", getpid());
				if (msgrcv(serviceMsgqId, &msgBuff, sizeof(msgBuff) - sizeof(long), getpid(), 0) == -1){
					slog(OPERATORE, "operatore.child.pid.%d.msgrcv.service request.failed!", getpid());
					if (errno == EINTR){
						slog(OPERATORE, "operatore.child.pid.%d.EINTR.due to eod", getpid());
					}
					if (release_sem(deskSemId, deskSemun) == -1){
						slog(OPERATORE, "operatore.child.pid.%d.release_sem.deskSemId: %d - deskSemun: %d", getpid(), deskSemId, deskSemun);
						err_exit(strerror(errno));
					}
					slog(OPERATORE, "operatore.child.pid.%d.release_sem.sportello.%d.semun.%d", getpid(), deskSemId, deskSemun);
					err_exit(strerror(errno));
				}
				slog(OPERATORE, "operatore.child.pid.%d.received.service req from user: %d service: %s", getpid(), msgBuff.payload.senderPid, msgBuff.payload.msg);
				//TODO aggiorna le statistiche
				slog(OPERATORE, "operatore.child.pid.%d.updating stats for services: %s", getpid(), msgBuff.payload.msg);
				

			}
			


			if (release_sem(deskSemId, deskSemun) == -1){
				slog(OPERATORE, "operatore.child.pid.%d.release_sem.deskSemId: %d - deskSemun: %d", getpid(), deskSemId, deskSemun);
				err_exit(strerror(errno));
			}
			slog(OPERATORE, "operatore.child.pid.%d.release_sem.sportello.%d.semun.%d", getpid(), deskSemId, deskSemun);
			
			exit(EXIT_SUCCESS);
		}
					/* parent code */
		//gestire l'uccisione del figlio	
		if (reserve_sem(operatoreSemId, 3) == -1){
			slog(OPERATORE, "operatore.pid.%d.reserve_sem.operatore sem.semun:2.failed", getpid());
			err_exit(strerror(errno));
		}
		slog(OPERATORE, "operatore.pid.%d.received eod", getpid());
		if (kill(pid, SIGTERM) == -1){
			slog(OPERATORE, "operatore.pid.%d.kill.child pid.%d.failed!", getpid(), pid);
			err_exit(strerror(errno));
		}	
		slog(OPERATORE, "operatore.pid.%d.killed child.pid.%d", getpid(), pid);
		wait(NULL);
		slog(OPERATORE, "operatore.pid.%d.terminated.child.pid.%d", getpid(), pid);
		if (release_sem(operatoreSemId, 4) == -1){
			slog(OPERATORE, "operatore.pid.%d.release_sem.operatore sem", getpid());
			err_exit(strerror(errno));
		}

		days++;
	}	
	
	
}
