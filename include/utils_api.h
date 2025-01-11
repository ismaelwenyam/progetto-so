#ifndef UTILS_API_H
#define UTILS_API_H
void print_services_in_shm (int shmId, int semId);
void print_sportelli_in_shm (int shmId, int semId, int nofWorkerSeats);
int delete_ipc_resources (int id, char *resourceType);

#endif
