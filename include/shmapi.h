#ifndef SHMAPI_H
#define SHMAPI_H

#include <sys/shm.h>

#define SERVICE_SHARED_MEMORY 11111
#define STATISTICS_SHARED_MEMORY 11112
#define SPORTELLI_SHARED_MEMORY_KEY 11113
#define SPORTELLI_STATS_SHARED_MEMORY_KEY 11114

int rm_shm(int shmId);

#endif
