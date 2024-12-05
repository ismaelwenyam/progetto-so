#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/msg.h>

#include "simerr.h"
#include "ticket.h"
#include "semapi.h"
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
		

int main (int argc, char **argv){
	if (argc < 3){
		usage();
		err_exit("erogatore ticket - Invalid arguments");
	}
	int shmId = atoi(argv[1]);
	int semId = atoi(argv[2]);
	// reserve semafore to access shared mem
	if (reserve_sem(semId, 0) == -1){
		printf("error.erogatore_ticket.reserve_sem\n");
		err_exit(strerror(errno));
	}
	
	TicketAdtPtr availableServices;		
	errno = 0;
	availableServices = shmat(shmId, NULL, SHM_RND);	
	if (errno != 0){
		printf("error.erogatore_ticket.shmat\n");
		err_exit(strerror(errno));
	}

	#ifdef DEBUG
	//TODO stampa i servizi disponibili per il giorno
	#endif

	// create message queue	
	//Al giorno i+1 decidere se creare nuovamente la msgQueue o mantenere la coda del giorno i-1
	int msgQueueId;
	if ((msgQueueId = msgget(MSG_QUEUE_KEY, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR)) == -1){
		printf("error.erogatore_ticket.mssget\n");
		err_exit(strerror(errno));
	}

	//start server
	pid_t pid;
	TicketRequestAdt ticketRequest;
	#ifdef DEBUG
	//TODO printf("server erogatore_ticket up and running\n");
	#endif
	while (1) {
		if (msgrcv(msgQueueId, &ticketRequest, sizeof(ticketRequest.ticket) - sizeof(ticketRequest.mtype), 0, 0) == -1){
			printf("error.erogatore_ticket.msgrcv\n");
			// server don't have to stop
			continue;
		}
		pid = fork();
		if (pid == -1){			
			printf("error.erogatore_ticket.fork\n");	
			// TODO if the fork doesn't work the parent has to answer
			continue;
		}
		if (pid == 0){			/* child code */
			//TODO 6 is the number of services, try make it more dynamic
			for(size_t i = 0; i < NUMBER_OF_SERVICES; i++){
				if(strcmp(ticketRequest.ticket.servizio, availableServices[i].servizio) == 0){
					// service available for the day
					ticketRequest.ticket.tempario = availableServices[i].tempario;
					ticketRequest.ticket.serviceAvailable = availableServices[i].serviceAvailable;
					if (msgsnd(msgQueueId, &ticketRequest, sizeof(ticketRequest.ticket) - sizeof(long), IPC_NOWAIT) == -1){
						printf("error.erogatore_ticket.msgsnd\n");
						break;
					}
				}
			}
			exit(EXIT_SUCCESS);
		}
		
	}
	
	
}
