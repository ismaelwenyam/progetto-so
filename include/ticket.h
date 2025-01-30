#ifndef TICKET_H
#define TICKET_H

#include <stdbool.h>
#include "sportello_api.h"
#include "service_api.h"

#define MSG_QUEUE_KEY 11111

typedef struct ticket TicketAdt;
struct ticket
{
	ServizioAdt se;
	bool eod;
	SportelloAdt sp;
};

typedef struct ticketRequest TicketRequestAdt, *TicketRequestAdtPtr;
struct ticketRequest
{
	long mtype;
	TicketAdt ticket;
};

#endif
