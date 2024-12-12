#include <stdio.h>
#include <unistd.h>



#include "simerr.h"
#include "ticket.h"
#include "semapi.h"
#include "msgapi.h"
#include "config_sim.h"
#include "sim_stats.h"

void draft () {
        ConfigurationAdt configuration = get_config();
	printf("operatore.%d.configuration.nof_pause: %d\n", getpid(), configuration.nofPause);
	// compete to find desk

}

int main (int argc, char **argv){
	log_time();
	printf("operatore.pid.%d\n", getpid());	
	log_time();
	printf("operatore.pid.%d.init initialization\n", getpid());
	int msgQueueId;
	if ((msgQueueId = msgget(PROCESS_TO_DIRECTOR_MSG_KEY, 0)) == -1){
		log_time();
		printf("operatore.%d.msgget.ipcr_creat.failed!\n", getpid());
	}
	sleep(7);
	MsgAdt msg;
	msg.mtype = getpid();
	strcpy(msg.piAdt.mtext, READY);
	if (msgsnd(msgQueueId, &msg, sizeof(msg) - sizeof(long), 0) == -1){
		printf("operatore.pid.%d.msgsnd.failed!\n", getpid());
		err_exit(strerror(errno));
	}
	pause();
	log_time();
	printf("operatore.pid.%d.started\n", getpid());	
	
	
	








}
