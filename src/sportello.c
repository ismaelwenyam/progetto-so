#include <stdio.h>
#include <unistd.h>


#include "simerr.h"
#include "logapi.h"
#include "msgapi.h"


int main (int argc, char **argv) {
	slog(SPORTELLO, "sportello.pid.%d", getpid());	
	slog(SPORTELLO, "sportello.pid.%d.init initialization", getpid());
	int msgQueueId;
	if ((msgQueueId = msgget(PROCESS_TO_DIRECTOR_MSG_KEY, 0)) == -1){
		slog(SPORTELLO, "sportello.pid.%d.msgget.failed!", getpid());
		err_exit(strerror(errno));
	}
	slog(SPORTELLO, "sportello.pid.%d.msgget.process_to_director.ok!", getpid());
	slog(SPORTELLO, "sportello.pid.%d.msgget.requesting role for day...", getpid());
	ProcessInfoAdt pia;
	pia.pid = getpid();
	strcpy(pia.mtext, ROLE);
	MsgAdt msg;
	msg.mtype = SPORTELLO_GROUP;
	msg.piAdt = pia;
	if (msgsnd(msgQueueId, &msg, sizeof(msg) - sizeof(long), 0) == -1){
		slog(SPORTELLO, "sportello.pid.%d.msgsnd.failed!", getpid());
		err_exit(strerror(errno));
	}
	
	if (msgrcv(msgQueueId, &msg, sizeof(msg) - sizeof(long), getpid(), 0) == -1){
		slog(SPORTELLO, "sportello.pid.%d.msgrcv.failed to receive role!", getpid());
		err_exit(strerror(errno));
	}	
	
	slog(SPORTELLO, "sportello.pid.%d.msgrcv.role for day: %s", getpid(), msg.piAdt.mtext);
	
	







}
