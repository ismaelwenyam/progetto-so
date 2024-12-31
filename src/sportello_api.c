#include <stdio.h>
#include <stdlib.h>

#include "ticket.h"



SportelloPtr mkSportello(pid_t pid, bool deskAvailable, int deskSemId, int deskSemun, int workerDeskSemId, int workerDeskSemun){
	SportelloPtr sportello = malloc(sizeof(Sportello));
	if (sportello == NULL){
		return NULL;
	}
	sportello->operatorPid = pid;
	sportello->deskAvailable = deskAvailable;
	sportello->deskSemId = deskSemId;
	sportello->deskSemun = deskSemun;
	sportello->workerDeskSemId = workerDeskSemId;
	sportello->workerDeskSemun = workerDeskSemun;	
	sportello->nextSportello = NULL;
	return sportello;
}
bool dsSportello(SportelloPtr sportelloPtr){
	if (sportelloPtr == NULL){
		return false;
	}
	free(sportelloPtr);
	sportelloPtr = NULL;
	return true;
}

SportelloSetAdtPtr mkSportelloSet () {
	SportelloSetAdtPtr sportelloSet = malloc(sizeof(SportelloSetAdt));
	if (sportelloSet == NULL){
		return NULL;
	}
	sportelloSet->size = 0;
	sportelloSet->front = NULL;
	return sportelloSet;
}

bool dsSportelloSetAdt (SportelloSetAdtPtr *sportelloSet){
	if (sportelloSet == NULL){
		return false;
	}
	SportelloPtr current = (*sportelloSet)->front;
	SportelloPtr next = NULL;
	while (current != NULL){
		next = current->nextSportello;
		dsSportello(current);
		current = next;
	}
	free(*sportelloSet);
	*sportelloSet = NULL;
	return true;

}
bool addSportello (SportelloSetAdtPtr set, SportelloPtr sportello){
	if (set == NULL || sportello == NULL){
		return false;
	}
	SportelloPtr current = set->front;
	SportelloPtr previous = NULL;
	bool sportelloPresent = false;
	while (current != NULL && !sportelloPresent){
		sportelloPresent = current->operatorPid == sportello->operatorPid;
		previous = current;
		current = current->nextSportello;
	}
	if (sportelloPresent){
		return false;
	}
	if (previous == NULL){
		set->front = sportello;
	}else {
		previous->nextSportello = sportello;
	}
	set->size += 1;
	return true;
}

bool removeSportello (SportelloSetAdtPtr set, pid_t pid){
	if (set == NULL){
		return false;
	}
	SportelloPtr current = set->front;
	SportelloPtr previous = NULL;
	bool sportelloPresent = false;
	while (current != NULL && !sportelloPresent){
		sportelloPresent = current->operatorPid == pid;
		if (sportelloPresent){
			continue;
		}
		previous = current;
		current = current->nextSportello;
	}
	if (current == NULL){
		return true;
	}
	if (previous == NULL){
		set->front = current->nextSportello;
		free(current);
	}else{
		previous->nextSportello = current->nextSportello;
		free(current);
	}
	set->size -= 1;
	return true;
}


