#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "simerr.h"
#include "logapi.h"
#include "msgapi.h"
#include "service_api.h"
#include "sportello_api.h"
#include "shmapi.h"
#include "semapi.h"
#include "ticket.h"
#include "simulation_configuration.h"
#include "utils_api.h"

int main(int argc, char **argv)
{
	int configurationSemId;
	if ((configurationSemId = semget(CONFIGURATION_SEM_KEY, 0, 0)) == -1)
	{
		slog(SPORTELLO, "sportello.pid.%d.semget.configuration sem.failed", getpid());
		err_exit(strerror(errno));
	}
	ConfigurationAdt configuration = get_config();
	slog(SPORTELLO, "sportello.pid.%d", getpid());
	slog(SPORTELLO, "sportello.pid.%d.init initialization", getpid());
	int dirMsgQueueId, sportelloSemId, operatoreSportelloSemId;
	int sportelliShmSemId, sportelliShmId;
	if ((dirMsgQueueId = msgget(DIR_MESSAGE_QUEUE_KEY, 0)) == -1)
	{
		slog(SPORTELLO, "sportello.pid.%d.msgget.dir msg queue.failed!", getpid());
		err_exit(strerror(errno));
	}
	if ((sportelloSemId = semget(SPORTELLO_SYNC_SEM, 0, 0)) == -1)
	{
		slog(SPORTELLO, "sportello.pid.%d.semget.sportello sync sem.failed!", getpid());
		err_exit(strerror(errno));
	}

	//
	if ((sportelliShmSemId = semget(SPORTELLI_SHM_SEM_KEY, 0, 0)) == -1)
	{
		slog(SPORTELLO, "sportello.pid.%d.semget.services_shm_sem.failed!", getpid());
		err_exit(strerror(errno));
	}

	if ((sportelliShmId = shmget(SPORTELLI_SHARED_MEMORY_KEY, 0, 0)) == -1)
	{
		slog(SPORTELLO, "sportello.pid.%d.shmget.service_shared_memory.failed!", getpid());
		err_exit(strerror(errno));
	}

	int days = 0;
	MsgBuff msgBuff;
	SportelloAdtPtr sportelliPtr;
	while (1)
	{
		if ((operatoreSportelloSemId = semget(IPC_PRIVATE, 1, S_IWUSR | S_IRUSR)) == -1)
		{
			slog(SPORTELLO, "sportello.pid.%d.semget.ipc_private.operatoreSportello sem.failed", getpid());
			err_exit(strerror(errno));
		}
		if (init_sem_available(operatoreSportelloSemId, 0) == -1)
		{
			slog(SPORTELLO, "sportello.pid.%d.init_sem_available.operatoreSportello.failed", getpid());
			err_exit(strerror(errno));
		}
		slog(SPORTELLO, "sportello.pid.%d.operatoreSportelloSem.%d.available", getpid(), operatoreSportelloSemId);
		msgBuff.mtype = SPORTELLO_GROUP;
		msgBuff.payload.senderPid = getpid();
		strcpy(msgBuff.payload.msg, ROLE);
		if (msgsnd(dirMsgQueueId, &msgBuff, sizeof(msgBuff) - sizeof(long), 0) == -1)
		{
			slog(SPORTELLO, "sportello.pid.%d.msgsnd.failed to request role for day: %d", getpid(), days + 1);
			err_exit(strerror(errno));
		}
		slog(SPORTELLO, "sportello.pid.%d.sent request of role", getpid());
		if (msgrcv(dirMsgQueueId, &msgBuff, sizeof(msgBuff) - sizeof(long), getpid(), 0) == -1)
		{
			slog(SPORTELLO, "sportello.pid.%d.msgrcv.failed to receive role for day", getpid());
			err_exit(strerror(errno));
		}
		slog(SPORTELLO, "sportello.pid.%d.received role: %s for day: %d", getpid(), msgBuff.payload.msg, days + 1);
		// aggiornare memoria condivisa con operatoreSportelloSemId
		slog(SPORTELLO, "sportello.pid.%d.reserving sem to update sportelli shm", getpid());
		if (reserve_sem(sportelliShmSemId, 0) == -1)
		{
			slog(SPORTELLO, "sportello.pid.%d.reserve_sem.sportelli shm sem.failed!", getpid());
			err_exit(strerror(errno));
		}
		slog(SPORTELLO, "sportello.pid.%d.reserved sem to update sportelli shm", getpid());
		sportelliPtr = shmat(sportelliShmId, NULL, SHM_RND);
		if (sportelliPtr == (void *)-1)
		{
			slog(SPORTELLO, "sportello.pid.%d.shmat.failed", getpid());
			err_exit(strerror(errno));
		}
		slog(SPORTELLO, "sportello.pid.%d.updating sportelli shm...", getpid());
		for (int i = 0; i < configuration.nofWorkerSeats; i++)
		{
			if (strcmp(sportelliPtr[i].serviceName, msgBuff.payload.msg) == 0 && sportelliPtr[i].sportelloPid == getpid())
			{
				strcpy(sportelliPtr[i].serviceName, msgBuff.payload.msg);
				sportelliPtr[i].deskAvailable = 1;
				sportelliPtr[i].deskSemId = operatoreSportelloSemId;
				sportelliPtr[i].deskSemun = 0;
				break;
			}
		}
		slog(SPORTELLO, "sportello.pid.%d.updated sportelli", getpid());

		if (release_sem(sportelliShmSemId, 0) == -1)
		{
			slog(SPORTELLO, "sportello.pid.%d.relase_sem.sportelli shm sem.failed!", getpid());
			err_exit(strerror(errno));
		}
		slog(SPORTELLO, "sportello.pid.%d.released sem to update sportelli shm", getpid());

		if (shmdt(sportelliPtr) == -1)
		{
			slog(SPORTELLO, "sportello.pid.%d.shmdt.sportelli shm.failed!", getpid());
			return -1;
		}

		// notify sportelli shm has been updated
		if (release_sem(sportelloSemId, 0) == -1)
		{
			slog(SPORTELLO, "sportello.pid.%d.release_sem.sportello_sem.failed", getpid());
			err_exit(strerror(errno));
		}
		slog(SPORTELLO, "sportello.pid.%d.release_sem.%d.semun.0", getpid(), sportelloSemId);

		if (reserve_sem(sportelloSemId, 1) == -1)
		{
			slog(SPORTELLO, "sportello.pid.%d.reserve_sem.sportello_sem.failed", getpid());
			err_exit(strerror(errno));
		}

		slog(SPORTELLO, "sportello.pid.%d.reserving sem to update sportelli shm", getpid());
		if (reserve_sem(sportelliShmSemId, 0) == -1)
		{
			slog(SPORTELLO, "sportello.pid.%d.reserve_sem.sportelli shm sem.failed!", getpid());
			err_exit(strerror(errno));
		}
		slog(SPORTELLO, "sportello.pid.%d.reserved sem to update sportelli shm", getpid());
		sportelliPtr = shmat(sportelliShmId, NULL, SHM_RND);
		if (sportelliPtr == (void *)-1)
		{
			slog(SPORTELLO, "sportello.pid.%d.shmat.failed", getpid());
			err_exit(strerror(errno));
		}
		slog(SPORTELLO, "sportello.pid.%d.updating sportelli shm...", getpid());
		for (int i = 0; i < configuration.nofWorkerSeats; i++)
		{
			if (strcmp(sportelliPtr[i].serviceName, msgBuff.payload.msg) == 0 && sportelliPtr[i].sportelloPid == getpid())
			{
				strcpy(sportelliPtr[i].serviceName, msgBuff.payload.msg);
				sportelliPtr[i].deskAvailable = 1;
				sportelliPtr[i].deskSemId = 0;
				sportelliPtr[i].deskSemun = 0;
				break;
			}
		}
		slog(SPORTELLO, "sportello.pid.%d.updated sportello", getpid());
		delete_ipc_resources(operatoreSportelloSemId, "sem");
		if (release_sem(sportelliShmSemId, 0) == -1)
		{
			slog(SPORTELLO, "sportello.pid.%d.relase_sem.sportelli shm sem.failed!", getpid());
			err_exit(strerror(errno));
		}
		slog(SPORTELLO, "sportello.pid.%d.released sem to update sportelli shm", getpid());

		if (shmdt(sportelliPtr) == -1)
		{
			slog(SPORTELLO, "sportello.pid.%d.shmdt.sportelli shm.failed!", getpid());
			return -1;
		}

		slog(SPORTELLO, "sportello.pid.%d.release_sem.%d.semun.2", getpid(), sportelloSemId);
		if (reserve_sem(configurationSemId, 1) == -1)
		{
			slog(SPORTELLO, "sportello.pid.%d.failed to reserve config sem", getpid());
			err_exit(strerror(errno));
		}
		slog(SPORTELLO, "sportello.pid.%d.reserved config sem.semdid: %d - semun: %d", getpid(), configurationSemId, 1);
		if (get_timeout(configurationSemId) >= configuration.simDuration || get_explode(configurationSemId) >= configuration.explodeThreshold)
		{
			if (release_sem(configurationSemId, 1) == -1)
			{
				slog(SPORTELLO, "sportello.pid.%d.failed to release config sem", getpid());
				err_exit(strerror(errno));
			}
			slog(SPORTELLO, "sportello.pid.%d.released config sem.semdid: %d - semun: %d", getpid(), configurationSemId, 1);
			if (release_sem(sportelloSemId, 2) == -1)
			{
				slog(SPORTELLO, "sportello.pid.%d.release_sem.sportello_sem.failed", getpid());
				err_exit(strerror(errno));
			}
			break;
		}
		if (release_sem(configurationSemId, 1) == -1)
		{
			slog(SPORTELLO, "sportello.pid.%d.failed to release config sem", getpid());
			err_exit(strerror(errno));
		}
		slog(SPORTELLO, "sportello.pid.%d.released config sem.semdid: %d - semun: %d", getpid(), configurationSemId, 1);

		if (release_sem(sportelloSemId, 2) == -1)
		{
			slog(SPORTELLO, "sportello.pid.%d.release_sem.sportello_sem.failed", getpid());
			err_exit(strerror(errno));
		}
		days++;
	}
	slog(SPORTELLO, "sportello.pid.%d.delete ipc resources", getpid());
	// delete_ipc_resources(operatoreSportelloSemId, "sem");
	slog(SPORTELLO, "sportello.pid.%d.deleted ipc resources", getpid());
	exit(EXIT_SUCCESS);
}
