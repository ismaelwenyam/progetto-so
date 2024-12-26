#ifndef SEMAPI_H
#define SEMAPI_H

#include <sys/sem.h>

#define RESOURCE_CREATION_SYNC_SEM_KEY 11110
#define EROGATORE_SYNC_SEM 11111
#define UTENTE_SYNC_SEM 11112
#define OPERATORE_SYNC_SEM 11113
#define SPORTELLO_SYNC_SEM 11114
#define STATS_SHM_SEM_KEY 11115
#define SERVICES_SHM_SEM_KEY 11116

union semun {
	int              val;    /* Value for SETVAL */
        struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
        unsigned short  *array;  /* Array for GETALL, SETALL */
        struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                           (Linux-specific) */
};

int init_sem_available(int semId, int semNum);
int init_sem_in_use(int semId, int semNum);
int reserve_sem(int semId, int semNum);
int release_sem(int semId, int semNum);

#endif
