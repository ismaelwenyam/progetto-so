#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>


#include "simerr.h"
#include "services.h"
#include "ticket.h"
#include "semapi.h"
#include "shmapi.h"
#include "msgapi.h"
#include "config_sim.h"

void draft () {
        char *services[] = {IRP, ILR, PVB, PBP, APF, AOB};
	ConfigurationAdt configuration = get_config();
	printf("utente.%d.configuration.p_serv_min: %d\n", getpid(), configuration.pServMin);
	printf("utente.%d.configuration.p_serv_max: %d\n", getpid(), configuration.pServMax);
	printf("utente.%d.configuration.n_requests: %d\n", getpid(), configuration.nRequests);
	srand(time(NULL));
	// decidere se andare o meno all'ufficio postale
	int range = (configuration.pServMin + configuration.pServMax) / 2 - 1;
	int choice = rand() % configuration.pServMax + configuration.pServMin;
	printf("utente.%d.going to office? : %s\n", getpid(), choice >= range? "true" : "false");
	if (choice < range){
		//TODO user not going to post office go to end of the day
		exit(EXIT_FAILURE); // only for testing purpose
	}
	int nRequests = rand() % configuration.nRequests + 1;
	char **servicesList = malloc(sizeof(char) * 4 * nRequests);
	if (servicesList == NULL){
		printf("utente.pid.%d.malloc\n", getpid());
		err_exit(strerror(errno));
	}
	// decidere di quale servizio ha bisogno
	for (int i = 0; i < nRequests; i++){
		int serviceChoice = rand() % NUMBER_OF_SERVICES;
		servicesList[i] = services[serviceChoice];
	}
	printf("Desired services from user\n");	
	for (int i = 0; i < nRequests; i++){
		printf("%s\n", servicesList[i]);
	}
	// TODO scegliere l'orario

	// Fare richiesta all'erogatore per ticket
	int msgQueueId;
	if ((msgQueueId = msgget(MSG_QUEUE_KEY, 0)) == -1){
		printf("error: utente.%d.msgget\n", getpid());
		err_exit(strerror(errno));
	}
	for (int i = 0; i < nRequests; i++){
	
		TicketAdt ticket;
		strcpy(ticket.servizio, servicesList[i]);
		
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
	
		printf("utente.%d.service: %s available: %s temp: %d\n", getpid(), ticketRequest.ticket.servizio, ticketRequest.ticket.serviceAvailable ? "true" : "false", ticketRequest.ticket.tempario);
		// Recarsi allo sportello e attendere il proprio turno
	}
	
	









	//TODO free servicesList at the end of the day




}

int main (int argc, char **argv){
	log_time();
	printf("utente.pid.%d\n", getpid());	
	log_time();
	printf("utente.pid.%d.start initialization...\n", getpid());
	int msgQueueId;
	if ((msgQueueId = msgget(PROCESS_TO_DIRECTOR_MSG_KEY, 0)) == -1){
		log_time();
		printf("utente.%d.msgget.ipcr_creat.failed!\n", getpid());
	}
	//TODO remove
	sleep(3);
	ProcessInfoAdt pia;
	pia.pid = getpid();
	strcpy(pia.mtext, READY);
	MsgAdt msg;
	msg.mtype = 4;
	msg.piAdt = pia;
	if (msgsnd(msgQueueId, &msg, sizeof(msg) - sizeof(long), 0) == -1){
		printf("erogatore.pid.%d.msgsnd.failed!\n", getpid());
		err_exit(strerror(errno));
	}
	log_time();
	printf("utente.pid.%d.started\n", getpid());	
}
