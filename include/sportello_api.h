#ifndef SPORTELLO_API_H
#define SPORTELLO_API_H

#define SERVICE_LEN 4

typedef struct sportello SportelloAdt, *SportelloAdtPtr;
struct sportello
{
	char serviceName[SERVICE_LEN];
	pid_t sportelloPid;
	pid_t operatorPid; /* pid dell'operatore */
	short deskAvailable;
	int deskSemId; /* semaforo generato dallo sportello, su cui l'operatore fa la reserve*/
	int deskSemun;
	int workerDeskSemId; /* semaforo generato dall'operatore su cui l'utente si mette in fila */
	int workerDeskSemun;
};

#endif
