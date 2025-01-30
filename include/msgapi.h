#ifndef MSGAPI_H
#define MSGAPI_H

#include <sys/msg.h>

#define DIR_MESSAGE_QUEUE_KEY 11111
#define TICKET_MESSAGE_QUEUE_KEY 11112
#define SERVICE_MESSAGE_QUEUE_KEY 11113

//groups
#define DIRETTORE_GROUP 1
#define EROGATORE_GROUP 2
#define SPORTELLO_GROUP 3
#define OPERATORE_GROUP 4
#define UTENTE_GROUP 5

//message code
#define MSG_LEN 128
#define ROLE "role"
#define ADD "add"
#define START_OF_SIMULATION "sos"
#define START_OF_DAY "sod"
#define END_OF_DAY "eod"
#define RECOVER "rec"
#define END_OF_SIMULATION "eos"
#define PAUSE "pause"
#define OK "ok"

typedef struct payload Payload;
struct payload {
	pid_t senderPid;
	char msg[MSG_LEN];
	int temp;
	double elapsed;
};

typedef struct msgBuff MsgBuff;
struct msgBuff {
	long mtype;
	Payload payload;
};

int rm_msgq (int msgqId);


#endif

