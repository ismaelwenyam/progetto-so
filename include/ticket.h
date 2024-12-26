#ifndef TICKET_H
#define TICKET_H

#include <stdbool.h>

#define MSG_QUEUE_KEY 11111
#define TICKET_SIZE 4

typedef struct sportello Sportello, *SportelloPtr;
struct sportello {
	bool deskAvailable;
	int workerDeskSemId;
	int workerDeskSemun;
	int userDeskSemId;
	int userDeskSemun;
};

typedef struct ticket TicketAdt, *TicketAdtPtr;
struct ticket {
	char servizio[TICKET_SIZE];
	int tempario;
	bool serviceAvailable;
	SportelloPtr sportelli;
};

typedef struct ticketRequest TicketRequestAdt, *TicketRequestAdtPtr;
struct ticketRequest {
	long mtype;
	TicketAdt ticket;
};

#endif
