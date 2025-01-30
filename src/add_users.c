#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "msgapi.h"
#include "simerr.h"

int main(int argc, char **argv)
{
	int dirMsgQueueId;
	if ((dirMsgQueueId = msgget(DIR_MESSAGE_QUEUE_KEY, 0)) == -1)
	{
		printf("add_users exe failed to msgget director msg queue\n");
		err_exit(strerror(errno));
	}

	MsgBuff msgBuff;
	msgBuff.mtype = DIRETTORE_GROUP;
	msgBuff.payload.senderPid = getpid();
	strcpy(msgBuff.payload.msg, ADD);

	if (msgsnd(dirMsgQueueId, &msgBuff, sizeof(msgBuff) - sizeof(long), 0) == -1)
	{
		printf("add_users exe failed to send msg to director\n");
		err_exit(strerror(errno));
	}

	printf("add_users exe pid %d requested director to add users\n", getpid());
}
