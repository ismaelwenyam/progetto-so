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
#define ROLE "role"

typedef struct msgbuf MsgBuf;




#endif

