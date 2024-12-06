#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>


#include "simerr.h"
#include "services.h"
#include "ticket.h"
#include "semapi.h"
#include "shmapi.h"
#include "config_sim.h"

const char *services[] = {IRP, ILR, PVB, PBP, APF, AOB};
ConfigurationAdt configuration;

void print_config (){
	printf("simulation configuration\n");
	printf("NOF_WORKER_SEATS %d\n", configuration.nofWorkerSeats);
	printf("NOF_WORKERS %d\n", configuration.nofWorkers);
	printf("NOF_USERS %d\n", configuration.nofUsers);
	printf("SIM_DURATION %d\n", configuration.simDuration);
	printf("N_NANO_SECS %d\n", configuration.nNanoSecs);
	printf("NOF_PAUSE %d\n", configuration.nofPause);
	printf("P_SERV_MIN %d\n", configuration.pServMin);
	printf("P_SERV_MAX %d\n", configuration.pServMax);
	printf("EXPLODE_THRESHOLD %d\n", configuration.explodeThreshold);
}

int main (int argc, char **argv){
	
	ConfigurationAdtPtr configPtr = init_config("config_sim.conf");
	if (configPtr == NULL){
		printf("error: direttore.init_config\n");
		err_exit("error initializing configuration");
	}
	configuration = *configPtr;
	print_config();
	int direttoreErogatoreShmId, direttoreErogatoreSemId;
	//creazione memoria condivisa tra direttore e erogatore ticket
	if ((direttoreErogatoreShmId = shmget(DIRETTORE_TO_EROGATORE_SHM_KEY, sizeof(TicketAdt) * NUMBER_OF_SERVICES, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR)) == -1){
		printf("error: direttore.shmget\n");
		err_exit(strerror(errno));
	}
	//creazione semaforo tra direttore e erogatore ticket
	if ((direttoreErogatoreSemId = semget(DIRETTORE_TO_EROGATORE_SEM_KEY, 1, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR)) == -1){
		printf("error: direttore.semget\n");
		err_exit(strerror(errno));
	}
	if (init_sem_in_use(direttoreErogatoreSemId, 0) == -1){
		printf("error: direttore.reserve_sem\n");
		err_exit(strerror(errno));
	}
	errno = 0;
	TicketAdtPtr tickets = shmat(direttoreErogatoreShmId, NULL, SHM_RND);
	if (errno != 0){
		printf("error: direttore.tickets\n");
		err_exit(strerror(errno));
	}
	for (size_t i = 0; i < NUMBER_OF_SERVICES; i++){
		if (strcmp(services[i], IRP) == 0){
			strcpy(tickets[i].servizio, IRP);
			tickets[i].tempario = 10;
			tickets[i].serviceAvailable = true;	
		}else if (strcmp(services[i], ILR) == 0){
			strcpy(tickets[i].servizio, ILR);
			tickets[i].tempario = 8;
			tickets[i].serviceAvailable = true;	
		}else if (strcmp(services[i], PVB) == 0){
			strcpy(tickets[i].servizio, PVB);
			tickets[i].tempario = 6;
			tickets[i].serviceAvailable = true;	
		}else if (strcmp(services[i], PBP) == 0){
			strcpy(tickets[i].servizio, PBP);
			tickets[i].tempario = 8;
			tickets[i].serviceAvailable = true;	
		}else if (strcmp(services[i], APF) == 0){
			strcpy(tickets[i].servizio, APF);
			tickets[i].tempario = 20;
			tickets[i].serviceAvailable = true;	
		}else{
			strcpy(tickets[i].servizio, AOB);
			tickets[i].tempario = 20;
			tickets[i].serviceAvailable = true;	
		}
	}	
	
	if (release_sem(direttoreErogatoreSemId, 0) == -1){
		printf("error: direttore.release_sem\n");
		err_exit(strerror(errno));
	}
	
	if (shmdt(tickets) == -1){
		printf("error: direttore.reserve_sem\n");
		err_exit(strerror(errno));
	}
	
	//TODO usare key nei rispettivi header	
	static char *newargv[] = { NULL };
        static char *newenviron[] = { NULL };
	if (execve("./erogatore_ticket", newargv, newenviron) == -1){
		printf("error occured %s\n", strerror(errno));
	}	
}

