#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>


#include "simerr.h"
#include "services.h"
#include "ticket.h"
#include "semapi.h"

const char *services[] = {IRP, ILR, PVB, PBP, APF, AOB};

int main (int argc, char **argv){
	int direttoreErogatoreShmId, direttoreErogatoreSemId;
	//creazione memoria condivisa tra direttore e erogatore ticket
	if ((direttoreErogatoreShmId = shmget(IPC_PRIVATE, sizeof(TicketAdt) * NUMBER_OF_SERVICES , IPC_CREAT | S_IWUSR | S_IRUSR)) == -1){
		printf("error: direttore.shmget\n");
		err_exit(strerror(errno));
	}
	//creazione semaforo tra direttore e erogatore ticket
	if ((direttoreErogatoreSemId = semget(IPC_PRIVATE, 1, IPC_CREAT | S_IWUSR | S_IRUSR)) == -1){
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
		//TODO Make serviceAvailable random and temp +- 50%
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
	for (size_t i = 0; i < NUMBER_OF_SERVICES; i++){
		printf("service: %s - temp: %d - available: %d\n", tickets[i].servizio, tickets[i].tempario, tickets[i].serviceAvailable);
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
	static char *newargv[] = {"0", "1"};
        static char *newenviron[] = { NULL };
	if (execve("./erogatore_ticket", newargv, newenviron) == -1){
		printf("error occured %s\n", strerror(errno));
	}	
}

