#ifndef SEMAPI_H
#define SEMAPI_H

#include <sys/sem.h>

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
