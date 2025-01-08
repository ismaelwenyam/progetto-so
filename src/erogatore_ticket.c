#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>

#include "simerr.h"
#include "logapi.h"
#include "ticket.h"
#include "semapi.h"
#include "shmapi.h"
#include "msgapi.h"
#include "simulation_configuration.h"


void print_available_services (int argc, char **argv){
	printf("|Services|Timing|\n");
	for (int i = 1; i < argc; i++){
		printf("%9s%7d\n", argv[i], i);
	}
	printf("________________\n");
}

void usage (){
	printf("Not enough arguments. i.e: ./erogatore_ticket [SHMID] [SEMID]\n");
}

bool eod = false;	/* end of day */

void erogatoreSignalHandler(int sig){
	slog(EROGATORE, "erogatore_ticket.pid.%d.signal intercepted", getpid());
	if (sig == SIGTERM){
		eod = true;
		signal(sig, erogatoreSignalHandler);
	}
}

int main (int argc, char **argv){
	ConfigurationAdt configuration = get_config();
	slog(EROGATORE, "erogatore_ticket.pid.%d", getpid());	
	slog(EROGATORE, "erogatore_ticket.pid.%d.init initialization", getpid());
	int resourceCreationSemId, erogatoreSemId, ticketsMsgQueueId;
	int servicesShmSemId, servicesShmId;
	int sportelliShmSemId, sportelliShmId;
	if ((resourceCreationSemId = semget(RESOURCE_CREATION_SYNC_SEM_KEY, 0, 0)) == -1){
		slog(EROGATORE, "erogatore_ticket.pid.%d.semget.erogatore_sync_sem.failed!", getpid());
		err_exit(strerror(errno));
	}
	slog(EROGATORE, "erogatore_ticket.pid.%d.resourceCreationSemId.%d", getpid(), resourceCreationSemId);
	if (reserve_sem(resourceCreationSemId, 0) == -1){
		slog(EROGATORE, "erogatore_ticket.pid.%d.reserve_sem.resource creation sem.failed!", getpid());
		err_exit(strerror(errno));
	}	
	if ((ticketsMsgQueueId = msgget(TICKET_MESSAGE_QUEUE_KEY, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR)) == -1){
		slog(EROGATORE, "erogatore_ticket.pid.%d.msgget.ipc_creat.tickets message queue.failed!", getpid());
		err_exit(strerror(errno));
	}
	slog(EROGATORE, "erogatore_ticket.pid.%d.tickets msg queue.created.ok!", getpid());
	if (release_sem(resourceCreationSemId, 0) == -1){
		slog(EROGATORE, "erogatore_ticket.pid.%d.release_sem.resource creation sem.failed!", getpid());
		err_exit(strerror(errno));
	}	

	if ((erogatoreSemId = semget(EROGATORE_SYNC_SEM, 0, 0)) == -1){
		slog(EROGATORE, "erogatore_ticket.pid.%d.semget.erogatore_sync_sem.failed!", getpid());
		err_exit(strerror(errno));
	}	
	
	if ((servicesShmSemId = semget(SERVICES_SHM_SEM_KEY, 0, 0)) == -1){
		slog(EROGATORE, "erogatore_ticket.pid.%d.semget.erogatore_sync_sem.failed!", getpid());
		err_exit(strerror(errno));
	}	
	
	if ((servicesShmId = shmget(SERVICE_SHARED_MEMORY, 0, 0)) == -1){
		slog(EROGATORE, "erogatore_ticket.pid.%d.shmget.service_shared_memory.failed!", getpid());
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

	int days = 0;
	pid_t pid;
	TicketRequestAdt ticketRequest;
	ServizioAdtPtr servicesPtr;
	SportelloAdtPtr sportelliPtr;
	MsgBuff msgBuff;
	slog(EROGATORE, "erogatore_ticket.pid.%d.completed initialization", getpid());
	if (release_sem(erogatoreSemId, 0) == -1){
		slog(EROGATORE, "erogatore_ticket.pid.%d.release_sem.erogatore_sync_sem.semun.0.failed!", getpid());
		err_exit(strerror(errno));
	}
	
	while (days < configuration.simDuration){
		if (reserve_sem(erogatoreSemId, 1) == -1){
			slog(EROGATORE, "erogatore_ticket.pid.%d.reserve_sem.erogatore_sync_sem.semun.1.failed!", getpid());
			err_exit(strerror(errno));
		}
		slog(EROGATORE, "erogatore_ticket.pid.%d.day: %d", getpid(), days+1);
		pid = fork();
		if (pid == -1){
			slog(EROGATORE, "erogatore_ticket.pid.%d.fork.failed!", getpid());
			err_exit(strerror(errno));
		}	
		if (pid == 0){			/* child code */
			signal(SIGTERM, erogatoreSignalHandler);
			while (!eod){
				slog(EROGATORE, "erogatore_ticket.child.pid.%d.waiting tickets request", getpid());
				if (msgrcv(ticketsMsgQueueId, &msgBuff, sizeof(msgBuff) - sizeof(long), UTENTE_GROUP, 0) == -1){
					slog(EROGATORE, "erogatore_ticket.child.pid.%d.msgrcv.tickets msg queue.failed!", getpid());
					if (errno == EINTR){
						//TODO
						ticketRequest.mtype = msgBuff.payload.senderPid;
						ticketRequest.ticket.eod = true; 
						if (msgsnd(ticketsMsgQueueId, &ticketRequest, sizeof(ticketRequest) - sizeof(long), 0) == -1){
							slog(EROGATORE, "erogatore_ticket.child.pid.%d.msgsnd.notifying user: eod.failed!", getpid());
							err_exit(strerror(errno));
						} 
						slog(EROGATORE, "erogatore_ticket.child.pid.%d.msgsnd.notified user: eod", getpid());
					}
					continue;
				}	
				slog(EROGATORE, "erogatore_ticket.child.pid.%d.msgrcv.received ticket request for: %s", getpid(), msgBuff.payload.msg);
				slog(EROGATORE, "erogatore_ticket.child.pid.%d.reserving sem for services shared memory...", getpid());	
				if (reserve_sem(servicesShmSemId, 0) == -1){
					slog(EROGATORE, "erogatore_ticket.child.pid.%d.reserve_sem.services shm sem.failed!", getpid());
					err_exit(strerror(errno));
				}
				slog(EROGATORE, "erogatore_ticket.child.pid.%d.reserved sem for services shared memory", getpid());	
				servicesPtr = shmat(servicesShmId, NULL, SHM_RND);
				if (servicesPtr == (void*)-1){
					slog(EROGATORE, "erogatore_ticket.child.pid.%d.shmat.services shm.failed", getpid());	
					err_exit(strerror(errno));
				}
				slog(EROGATORE, "erogatore_ticket.child.pid.%d.accessed services shared memory", getpid());	
				for (int i = 0; i < NUMBER_OF_SERVICES; i++){
					if (strcmp(servicesPtr[i].name, msgBuff.payload.msg) == 0){
						ticketRequest.ticket.se = servicesPtr[i];
						ticketRequest.ticket.eod = eod;
						break;
					}
				}
				//TODO impostare il tempario a +- 50% del valore iniziale
				if (release_sem(servicesShmSemId, 0) == -1){
					slog(EROGATORE, "erogatore_ticket.child.pid.%d.release_sem.services shm sem.failed!", getpid());
					err_exit(strerror(errno));
				}
				if (shmdt(servicesPtr) == -1){
					slog(EROGATORE, "erogatore_ticket.child.pid.%d.shmdt.services shm.failed!", getpid());
					return -1;
				}
				if (reserve_sem(sportelliShmSemId, 0) == -1){
					slog(EROGATORE, "erogatore.pid.%d.reserve_sem.sportelli_shm_sem.failed", getpid());
					err_exit(strerror(errno));
				}
				sportelliPtr = shmat(sportelliShmId, NULL, SHM_RND);
				if (sportelliPtr == (void*) -1){
					slog(EROGATORE, "erogatore.pid.%d.shmat.sportelli.failed", getpid());
					err_exit(strerror(errno));
				}	
				for (int i = 0; i < configuration.nofWorkerSeats; i++){
					if (strcmp(sportelliPtr[i].serviceName, msgBuff.payload.msg) == 0){
						ticketRequest.ticket.sp = sportelliPtr[i];
						break;	
					}
				}
				if (release_sem(sportelliShmSemId, 0) == -1){
					slog(EROGATORE, "erogatore.pid.%d.release_sem.sportelli_shm_sem.failed", getpid());
					err_exit(strerror(errno));
				}
				if (shmdt(sportelliPtr) == -1){
					slog(EROGATORE, "erogatore_ticket.child.pid.%d.shmdt.services shm.failed!", getpid());
					return -1;
				}
				
				ticketRequest.mtype = msgBuff.payload.senderPid;
				if (msgsnd(ticketsMsgQueueId, &ticketRequest, sizeof(ticketRequest) - sizeof(long), 0) == -1){
					slog(EROGATORE, "erogatore_ticket.child.pid.%d.msgsnd.tickets msg queue.failed!", getpid());
					err_exit(strerror(errno));
				} 
				slog(EROGATORE, "erogatore_ticket.child.pid.%d.msgsnd.ticket sent!", getpid());
			}
			slog(EROGATORE, "erogatore_ticket.child.pid.%d.end of day:%s", getpid(), eod ? "true" : "false");
			exit(EXIT_SUCCESS);
		}
						/* parent code */
		slog(EROGATORE, "erogatore_ticket.pid.%d.waiting end of day", getpid());
		if (reserve_sem(erogatoreSemId, 3) == -1){
			slog(EROGATORE, "erogatore_ticket.pid.%d.reserve_sem.erogatore_sync_sem.semun.3.failed!", getpid());
			err_exit(strerror(errno));
		}
		slog(EROGATORE, "erogatore_ticket.pid.%d.end of day received", getpid());
		if (kill(pid, SIGTERM) == 1){
			slog(EROGATORE, "erogatore_ticket.pid.%d.kill.child pid.%d.failed!", getpid(), pid);
			err_exit(strerror(errno));	
		}	
		slog(EROGATORE, "erogatore_ticket.pid.%d.kill.child pid.%d.ok", getpid(), pid);
		wait(NULL);

		if (release_sem(erogatoreSemId, 2) == -1){
			slog(EROGATORE, "erogatore_ticket.pid.%d.release_sem.erogatore_sync_sem.semun.2.failed!", getpid());
			err_exit(strerror(errno));
		}
		slog(EROGATORE, "erogatore_ticket.pid.%d.erogatoreSem.release_sem(%d, 1)", getpid(), erogatoreSemId);
		days++;
	}	
	if (msgctl(ticketsMsgQueueId, IPC_RMID, 0) == -1){
		slog(EROGATORE, "erogatore_ticket.pid.%d.msgctl.ipc_rmid.tickets msg queue.failed!", getpid());
		err_exit(strerror(errno));
	}
	slog(EROGATORE, "erogatore_ticket.pid.%d.tickets msg queue.removed!", getpid());
	
}
