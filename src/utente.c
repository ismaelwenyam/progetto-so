#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>


#include "simerr.h"
#include "services.h"
#include "ticket.h"
#include "semapi.h"
#include "shmapi.h"
#include "config_sim.h"


int main (int argc, char **argv){
	const char *services[] = {IRP, ILR, PVB, PBP, APF, AOB};
	ConfigurationAdt configuration = get_config();
	printf("utente.configuration.PservMin: %d\n", configuration.pServMin);
	printf("utente.configuration.PservMax: %d\n", configuration.pServMax);
	srand(time(NULL));
	int choice = rand() % configuration.pServMax + configuration.pServMin;
	printf("utente.choice: %d\n", choice);
	printf("utente.office? : %s\n", choice >= (configuration.pServMin + configuration.pServMax) / 2 - 1 ? "true" : "false");
	int serviceChoice = rand() % NUMBER_OF_SERVICES;
	printf("utente.service.choice: %s\n", services[serviceChoice]);
	
	// decidere se andare o meno all'ufficio postale
	// decidere di quale servizio ha bisogno
	// scegliere l'orario
	// Fare richiesta all'erogatore per ticket
	int msgQueueId;
	if ((msgQueueId = msgget(MSG_QUEUE_KEY, 0)) == -1){
		printf("error: utente.%d.msgget\n", getpid());
		err_exit(strerror(errno));
	}
	
	TicketAdt ticket;
	strcpy(ticket.servizio, argv[1]);
		
	TicketRequestAdt ticketRequest;
	ticketRequest.mtype = getpid();
	ticketRequest.ticket = ticket;
	
	if (msgsnd(msgQueueId, &ticketRequest, sizeof(ticketRequest) - sizeof(long), IPC_NOWAIT) == -1){
		printf("error: utente.%d.msgsnd\n", getpid());
		err_exit(strerror(errno));
	}
	
	if (msgrcv(msgQueueId, &ticketRequest, sizeof(ticketRequest) - sizeof(long), getpid(), 0) == -1){
		printf("error: utente.%d.msgrcv\n", getpid());
		err_exit(strerror(errno));
	}
	
	printf("utente.%d.service: %s available: %s temp: %d\n", getpid(), argv[1], ticketRequest.ticket.serviceAvailable ? "true" : "false", ticketRequest.ticket.tempario);
	
	
	// Recarsi allo sportello e attendere il proprio turno












}
