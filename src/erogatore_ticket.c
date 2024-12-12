#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/msg.h>

#include "simerr.h"
#include "logapi.h"
#include "ticket.h"
#include "semapi.h"
#include "shmapi.h"
#include "msgapi.h"
#include "services.h"


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

void draft () {
        int semId, shmId;
	if ((semId = semget(DIRETTORE_TO_EROGATORE_SEM_KEY, 0, 0)) == -1){
		printf("error: erogatore_ticket.semget\n");
		err_exit(strerror(errno));
	}
	if ((shmId = shmget(DIRETTORE_TO_EROGATORE_SHM_KEY, 0, 0)) == -1){
		printf("error: erogatore_ticket.shmget\n");
		err_exit(strerror(errno));
	}
	// reserve semafore to access shared mem
	if (reserve_sem(semId, 0) == -1){
		printf("error: erogatore_ticket.reserve_sem\n");
		err_exit(strerror(errno));
	}
	
	TicketAdtPtr availableServices;		
	errno = 0;
	availableServices = shmat(shmId, NULL, SHM_RND);	
	if (errno != 0){
		printf("error: erogatore_ticket.shmat\n");
		err_exit(strerror(errno));
	}	

	//#ifdef DEBUG
	//TODO stampa i servizi disponibili per il giorno
	for (size_t i = 0; i < NUMBER_OF_SERVICES; i++){
		printf("service: %s - temp: %d - available: %d\n", availableServices[i].servizio, availableServices[i].tempario, availableServices[i].serviceAvailable);
	}
	//#endif

	// create message queue	
	//Al giorno i+1 decidere se creare nuovamente la msgQueue o mantenere la coda del giorno i-1
	int msgQueueId;
	if ((msgQueueId = msgget(MSG_QUEUE_KEY, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR)) == -1){
		printf("error.erogatore_ticket.msgget\n");
		err_exit(strerror(errno));
	}

	//start server
	pid_t pid;
	TicketRequestAdt ticketRequest;
	#ifdef DEBUG
	//TODO printf("server erogatore_ticket up and running\n");
	#endif
	while (1) {
		if (msgrcv(msgQueueId, &ticketRequest, sizeof(ticketRequest) - sizeof(long), 0, 0) == -1){
			printf("error: %s - erogatore_ticket.msgrcv\n", strerror(errno));
			// server don't have to stop
			continue;
		}
		printf("erogatore_ticket.requested service: %s\n", ticketRequest.ticket.servizio);
		pid = fork();
		if (pid == -1){			
			printf("error.erogatore_ticket.fork\n");	
			// TODO if the fork doesn't work the parent has to answer
			continue;
		}
		if (pid == 0){			/* child code */
			for(size_t i = 0; i < NUMBER_OF_SERVICES; i++){
				if(strcmp(ticketRequest.ticket.servizio, availableServices[i].servizio) == 0){
					// service available for the day
					ticketRequest.ticket.tempario = availableServices[i].tempario;
					ticketRequest.ticket.serviceAvailable = availableServices[i].serviceAvailable;
					if (msgsnd(msgQueueId, &ticketRequest, sizeof(ticketRequest) - sizeof(long), IPC_NOWAIT) == -1){
						printf("error.erogatore_ticket.msgsnd\n");
						break;
					}
				}
			}
			exit(EXIT_SUCCESS);
		}
		
	}
	
	// release semafore
	if (release_sem(semId, 0) == -1){
		printf("error: erogatore_ticket.reserve_sem\n");
		err_exit(strerror(errno));
	}
	
	if (shmdt(availableServices) == -1){
		printf("error: erogatore_ticket.shmdt\n");
		err_exit(strerror(errno));
	}
	



}
		

int main (int argc, char **argv){
	slog(EROGATORE, "erogatore_ticket.pid.%d", getpid());	
	slog(EROGATORE, "erogatore_ticket.pid.%d.init initialization", getpid());
	int msgQueueId, erogatoreSemId;
	if ((msgQueueId = msgget(PROCESS_TO_DIRECTOR_MSG_KEY, 0)) == -1){
		slog(EROGATORE, "erogatore_ticket.pid.%d.msgget.failed!", getpid());
		err_exit(strerror(errno));
	}
	slog(EROGATORE, "erogatore_ticket.pid.%d.msgget.process_to_director.ok!", getpid());
	if ((erogatoreSemId = semget(EROGATORE_SEM_KEY, 0, 0)) == -1){
		slog(EROGATORE, "erogatore_ticket.pid.%d.semget.failed!", getpid());
		err_exit(strerror(errno));
	}
	slog(EROGATORE, "erogatore_ticket.pid.%d.semget.erogatore_sem.ok!", getpid());
	slog(EROGATORE, "erogatore_ticket.pid.%d.initialization.done!", getpid());
	ProcessInfoAdt pia;
	pia.pid = getpid();
	strcpy(pia.mtext, READY);
	MsgAdt msg;
	msg.mtype = 1;
	msg.piAdt = pia;
	if (msgsnd(msgQueueId, &msg, sizeof(msg) - sizeof(long), 0) == -1){
		slog(EROGATORE, "erogatore.pid.%d.msgsnd.failed!", getpid());
		err_exit(strerror(errno));
	}
	slog(EROGATORE, "erogatore_ticket.pid.%d.msgsnd.director notified status:ready.done!", getpid());
	slog(EROGATORE, "erogatore_ticket.pid.%d.reserving_sem...\n", getpid());
	if (reserve_sem(erogatoreSemId, 0) == -1){
		slog(EROGATORE, "erogatore.pid.%d.reserve_sem.failed!", getpid());
		err_exit(strerror(errno));
	}
	slog(EROGATORE, "erogatore.pid.%d.started", getpid());	
	
}
