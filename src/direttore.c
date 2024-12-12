#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>


#include "simerr.h"
#include "logapi.h"
#include "services.h"
#include "ticket.h"
#include "semapi.h"
#include "shmapi.h"
#include "msgapi.h"
#include "config_sim.h"



void create_erogatore_ticket();
void create_sportello(int nofWorkerSeat);
void create_operatore(int nofWorkers);
void create_utente(int nofUsers);

void print_config (ConfigurationAdt configuration){
	slog(DIRETTORE, "simulation configuration");
	slog(DIRETTORE, "NOF_WORKER_SEATS %d", configuration.nofWorkerSeats);
	slog(DIRETTORE, "NOF_WORKERS %d", configuration.nofWorkers);
	slog(DIRETTORE, "NOF_USERS %d", configuration.nofUsers);
	slog(DIRETTORE, "SIM_DURATION %d", configuration.simDuration);
	slog(DIRETTORE, "N_NANO_SECS %d", configuration.nNanoSecs);
	slog(DIRETTORE, "NOF_PAUSE %d", configuration.nofPause);
	slog(DIRETTORE, "P_SERV_MIN %d", configuration.pServMin);
	slog(DIRETTORE, "P_SERV_MAX %d", configuration.pServMax);
	slog(DIRETTORE, "EXPLODE_THRESHOLD %d", configuration.explodeThreshold);
	slog(DIRETTORE, "N_REQUESTS %d", configuration.nRequests);
}

void draft () {
	slog(DIRETTORE, "logs from direttore");
	slog(EROGATORE, "logs from erogatore");
	slog(SPORTELLO, "logs from sportello");
	slog(OPERATORE, "logs from operatore");
	slog(UTENTE, "logs from utente");
        const char *services[] = {IRP, ILR, PVB, PBP, APF, AOB};
	ConfigurationAdt configuration = get_config();
	print_config(configuration);
	int direttoreErogatoreShmId, direttoreErogatoreSemId;
	//creazione memoria condivisa tra direttore e erogatore ticket
	if ((direttoreErogatoreShmId = shmget(DIRETTORE_TO_EROGATORE_SHM_KEY, sizeof(TicketAdt) * NUMBER_OF_SERVICES, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR)) == -1){
		slog(DIRETTORE, "error: direttore.shmget");
		err_exit(strerror(errno));
	}
	//creazione semaforo tra direttore e erogatore ticket
	if ((direttoreErogatoreSemId = semget(DIRETTORE_TO_EROGATORE_SEM_KEY, 1, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR)) == -1){
		slog(DIRETTORE, "error: direttore.semget");
		err_exit(strerror(errno));
	}
	if (init_sem_in_use(direttoreErogatoreSemId, 0) == -1){
		slog(DIRETTORE, "error: direttore.reserve_sem");
		err_exit(strerror(errno));
	}
	errno = 0;
	TicketAdtPtr tickets = shmat(direttoreErogatoreShmId, NULL, SHM_RND);
	if (errno != 0){
		slog(DIRETTORE, "error: direttore.tickets");
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
		printf("error: direttore.release_sem");
		err_exit(strerror(errno));
	}
	
	if (shmdt(tickets) == -1){
		printf("error: direttore.shmdt");
		err_exit(strerror(errno));
	}



}

int main (int argc, char **argv){	
	slog(DIRETTORE, "logs from direttore");
	slog(EROGATORE, "logs from erogatore");
	slog(SPORTELLO, "logs from sportello");
	slog(OPERATORE, "logs from operatore");
	slog(UTENTE, "logs from utente");
	slog(DIRETTORE, "direttore.pid.%d.start", getpid());
	ConfigurationAdt configuration = get_config();
	print_config(configuration);
	slog(DIRETTORE, "direttore.pid.%d.initialization", getpid());
	int erogatoreSemId;
	if ((erogatoreSemId = semget(EROGATORE_SEM_KEY, 1, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR)) == -1){
		slog(DIRETTORE, "direttore.pid.%d.semget.erogatore_sem.failed!", getpid());
		err_exit(strerror(errno));
	}	
	slog(DIRETTORE, "direttore.pid.%d.semget.erogatore_sem.ipcr_creat.done!", getpid());
	if (init_sem_in_use(erogatoreSemId, 0) == -1){
		slog(DIRETTORE, "direttore.pid.%d.init_sem_in_use.erogatore_sem.failed!", getpid());
		err_exit(strerror(errno));
	}
	slog(DIRETTORE, "direttore.pid.%d.initialize erogatore_sem in use.done!", getpid());
	int msgQueueId, processToWait = 1 ;//+ configuration.nofUsers + configuration.nofWorkers;
	if ((msgQueueId = msgget(PROCESS_TO_DIRECTOR_MSG_KEY, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR)) == -1){
		slog(DIRETTORE, "direttore.%d.msgget.ipcr_creat.failed!\n", getpid());
		err_exit(strerror(errno));
	}
	slog(DIRETTORE, "direttore.pid.%d.msgget.px_to_director msgqueue.ipcr_creat.done!", getpid());
	/* TODO 
	fase di inizializzazione
	*/
	// creazione dei processi
	create_erogatore_ticket();
	//create_operatore(configuration.nofWorkers);
	//create_utente(configuration.nofUsers);
	//
	slog(DIRETTORE, "direttore.pid.%d.processes creation.done!", getpid());
	int counter = 0;
	MsgAdt msg;
	char group[24];
	while (counter < processToWait){
		if (msgrcv(msgQueueId, &msg, sizeof(msg) - sizeof(long), 0, 0) == -1){
			slog(DIRETTORE, "direttore.pid.%d.msgrcv.failed!", getpid());
			err_exit(strerror(errno));
		}		
		if (msg.mtype == EROGATORE_GROUP)
			strcpy(group, "erogatore");
		else if (msg.mtype == SPORTELLO_GROUP)
			strcpy(group, "sportello");
		else if (msg.mtype == OPERATORE_GROUP)
			strcpy(group, "operatore");
		else 
			strcpy(group, "utente");

		slog(DIRETTORE, "direttore.pid.%d.msgrcv.from group: %s.pid.%d.msg: %s", getpid(), group, msg.piAdt.pid, msg.piAdt.mtext);
		counter++;
	}
	int days = 0;
	struct timespec t = {0, configuration.nNanoSecs * 1440};
	create_sportello(configuration.nofWorkerSeats);
	
	while (days < configuration.simDuration){	
		// start simulation
		// assign role to worker seats
		int i = 0;
		slog(DIRETTORE, "direttore.pid.%d.assigning role to worker seats...", getpid());
		while (i < configuration.nofWorkerSeats){
			if (msgrcv(msgQueueId, &msg, sizeof(msg) - sizeof(long), SPORTELLO_GROUP, 0) == -1){
				slog(DIRETTORE, "direttore.pid.%d.msgrcv.from group: sportello.failed!", getpid());
				err_exit(strerror(errno));
			}		
			msg.mtype = msg.piAdt.pid;
			strcpy(msg.piAdt.mtext, "TEST");	
			if (msgsnd(msgQueueId, &msg, sizeof(msg) - sizeof(long), 0) == -1){
				slog(DIRETTORE, "direttore.pid.%d.msgsnd.failed assign role to: %ld", getpid(), msg.mtype);	
				err_exit(strerror(errno));
			}
			i++;
		}
		// set all sems to proceed aka 1

		// start erogatore	
		if (release_sem(erogatoreSemId, 0) == -1){
			slog(DIRETTORE, "direttore.pid.%d.release_sem.erogatore_sem.failed!", getpid());
			err_exit(strerror(errno));
		}
		//nanosleep

		#ifdef DEBUG
			slog(DIRETTORE, "direttore.pid.%d.nanosleeping.%d", getpid(), configuration.nNanoSecs);
		#endif
		puts("");
		nanosleep(&t, NULL);	
		//print daily statistics
		slog(DIRETTORE, "day %d statitics", days + 1);
		// check explode threshold
		days++;
	}
	//timeout reached || over exlpode_threshold
	slog(DIRETTORE, "simulation statitics\n");
}

void create_erogatore_ticket () {
	slog(DIRETTORE, "direttore.pid.%d.create_erogatore_ticket", getpid());
	pid_t pid;
	int waitStatus;
	pid = fork();
	if (pid == -1){
		slog(DIRETTORE, "direttore.pid.%d.create_sportello.fork.failed!.interupting simulation", getpid());
		err_exit(strerror(errno));
	}
	if (pid == 0 ){
		slog(DIRETTORE, "direttore.child.pid.%d.create_erogatore_ticket", getpid());
		char *const argv[] = {"./erogatore_ticket", NULL};
		char *const env[] = { NULL };
		if (execve("./erogatore_ticket", argv, env) == -1){
			err_exit(strerror(errno));
		}
		exit(EXIT_SUCCESS);
	}
	/* wait(&waitStatus);	
	if (!WIFEXITED(waitStatus)){
		log_time();
		printf("creation of erogatore_ticket failed interupting simulation\n");
		exit(EXIT_FAILURE);
	}
	*/
}

void create_sportello (int nofWorkerSeats){
	slog(DIRETTORE, "direttore.pid.%d.create_sportello.nofWorkerSeats:%d", getpid(), nofWorkerSeats);
	pid_t pid;
	int waitStatus;
	int counter = 0;
	while (counter < nofWorkerSeats){
		pid = fork();
		if (pid == -1){
			// TODO continuare o stoppare l'intera simulazione?
			slog(DIRETTORE, "direttore.pid.%d.create_sportello.fork.failed!.interupting simulation", getpid());
			err_exit(strerror(errno));
		}
		if (pid == 0){
			slog(DIRETTORE, "direttore.child.pid.%d.create_sportello", getpid());
			char *const argv[] = {"./sportello", NULL};
			char *const env[] = { NULL };
			if (execve("./sportello", argv, env) == -1){
				// TODO continuare o stoppare l'intera simulazione?
				slog(DIRETTORE, "direttore.child.pid.%d.create_sportello.execve.failed!", getpid());
				err_exit(strerror(errno));
			}
			exit(EXIT_SUCCESS);
		}
		counter++;
	}
	/*
	int notCreatedSeats = 0;
	while (counter > 0){
		wait(&waitStatus);
		if (!WIFEXITED(waitStatus)){
			printf("direttore.pid.%d.create_sportello.sportello.no created!.reason: %s\n", getpid(), strsignal(waitStatus));
			notCreatedSeats++;
		}

		counter--;
	}
	
	if (notCreatedSeats >= nofWorkerSeats - 1){
		printf("direttore.child.%d.create_sportello.not enough seats created.interupting simulation\n", getpid());

	}
	*/
	
}

void create_operatore(int nofWorkers){
	slog(DIRETTORE, "direttore.pid.%d.create_operatore.nofWorkers:%d", getpid(), nofWorkers);
	pid_t pid;
	int waitStatus;
	int counter = 0;
	while (counter < nofWorkers){
		pid = fork();
		if (pid == -1){
			// TODO continuare o stoppare l'intera simulazione?
			slog(DIRETTORE, "direttore.pid.%d.create_operatore.fork.failed!.interupting simulation", getpid());
			err_exit(strerror(errno));
		}
		if (pid == 0){
			slog(DIRETTORE, "direttore.child.pid.%d.create_operatore", getpid());
			char *const argv[] = {"./operatore", NULL};
			char *const env[] = { NULL };
			if (execve("./operatore", argv, env) == -1){
				// TODO continuare o stoppare l'intera simulazione?
				slog(DIRETTORE, "direttore.child.pid.%d.operatore.execve.failed!", getpid());
				err_exit(strerror(errno));
			}
			exit(EXIT_SUCCESS);
		}
		counter++;
	}
	int notCreatedOperators = 0;
	while (counter > 0){
		wait(&waitStatus);
		if (!WIFEXITED(waitStatus)){
			slog(DIRETTORE, "direttore.pid.%d.create_operatore.operatore.no created!.reason: %s", getpid(), strsignal(waitStatus));
			notCreatedOperators++;
		}

		counter--;
	}
	
	if (notCreatedOperators >= nofWorkers- 1){
		slog(DIRETTORE, "direttore.child.%d.create_operatore.not enough operators created.interupting simulation", getpid());

	}

}

void create_utente(int nofUsers){
	slog(DIRETTORE, "direttore.pid.%d.create_utente.nofUsers:%d", getpid(), nofUsers);
	pid_t pid;
	int waitStatus;
	int counter = 0;
	while (counter < nofUsers){
		pid = fork();
		if (pid == -1){
			// TODO continuare o stoppare l'intera simulazione?
			slog(DIRETTORE, "direttore.pid.%d.create_utente.fork.failed!.interupting simulation", getpid());
			err_exit(strerror(errno));
		}
		if (pid == 0){
			slog(DIRETTORE, "direttore.child.pid.%d.create_utente", getpid());
			char *const argv[] = {"./utente", NULL};
			char *const env[] = { NULL };
			if (execve("./utente", argv, env) == -1){
				// TODO continuare o stoppare l'intera simulazione?
				slog(DIRETTORE, "direttore.child.pid.%d.utente.execve.failed!", getpid());
				err_exit(strerror(errno));
			}
			exit(EXIT_SUCCESS);
		}
		counter++;
	}
	while (counter > 0){
		wait(&waitStatus);
		if (!WIFEXITED(waitStatus)){
			slog(DIRETTORE, "direttore.pid.%d.create_utente.utente.not created!.reason: %s", getpid(), strsignal(waitStatus));
		}
		counter--;
	}

}

