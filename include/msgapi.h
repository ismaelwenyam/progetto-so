#ifndef MSGAPI_H
#define MSGAPI_H

#include <sys/msg.h>

#define PROCESS_TO_DIRECTOR_MSG_KEY 11111
#define READY "ready"
#define ROLE "role"
#define MSG_LEN 256

//groups
#define EROGATORE_GROUP 1
#define SPORTELLO_GROUP 2
#define OPERATORE_GROUP 3
#define UTENTE_GROUP 4

typedef struct msg MsgAdt, *MsgAdtPtr;
typedef struct processInfo ProcessInfoAdt, *ProcessInfoAdtPtr;

struct processInfo {
	int pid;
	char mtext[MSG_LEN];
};

struct msg {
	long mtype;		/* groups - 1-> erogatore - 2-> sportello - 3-> operatore - 4-> utente */
	ProcessInfoAdt piAdt;
};

#endif
