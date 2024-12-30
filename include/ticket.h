#ifndef TICKET_H
#define TICKET_H

#include <stdbool.h>

#define MSG_QUEUE_KEY 11111
#define TICKET_LEN 4

typedef struct sportello Sportello, *SportelloPtr;
struct sportello {
	pid_t operatorPid;		/* pid dell'operatore */
	bool deskAvailable;
	int deskSemId;		/* id del semaforo su cui l'operatore lavora generato da ogni sportello*/
	int deskSemun;		/* semun del semaforo su cui l'operatore lavora */
	int workerDeskSemId;	/* id del semaforo su cui l'utente si mette in fila */
	int workerDeskSemun;	/* semun */
	SportelloPtr nextSportello;
};

typedef struct sportelloSet SportelloSetAdt, *SportelloSetAdtPtr;
struct sportelloSet {
	int size;
	SportelloPtr front;
};

Sportello mkSportello(pid_t, bool, int, int, int, int);
bool dsSportello(SportelloPtr);

SportelloSetAdt mkSportelloSet ();
bool dsSportelloSetAdt (SportelloSetAdtPtr);
bool addSportello (SportelloSetAdtPtr, SportelloPtr);
bool removeSportello (SportelloSetAdtPtr, pid_t);

typedef struct ticket TicketAdt, *TicketAdtPtr;
struct ticket {
	char servizio[TICKET_LEN];
	int tempario;
	bool serviceAvailable;
	bool eod;
	int index;
	Sportello sportello;
};

typedef struct ticketRequest TicketRequestAdt, *TicketRequestAdtPtr;
struct ticketRequest {
	long mtype;
	TicketAdt ticket;
};

#endif
