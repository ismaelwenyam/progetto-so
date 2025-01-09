#ifndef SPORTELLO_API_H
#define SPORTELLO_API_H

#define SERVICE_LEN 4

typedef struct sportello SportelloAdt, *SportelloAdtPtr;
struct sportello {
        char serviceName[SERVICE_LEN];  
	pid_t sportelloPid;
	pid_t operatorPid;		/* pid dell'operatore */
	short deskAvailable;
	int deskSemId;		/* id del semaforo su cui l'operatore lavora generato da ogni sportello*/
	int deskSemun;		/* semun del semaforo su cui l'operatore lavora */
	int workerDeskSemId;	/* id del semaforo su cui l'utente si mette in fila */
	int workerDeskSemun;	/* semun */
};

#endif
