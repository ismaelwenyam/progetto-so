#include <stdio.h>
#include <stdlib.h>

#include "ticket.h"



SportelloPtr mkSportello(pid_t pid, bool serviceAvailable, int deskSemId, int deskSemun, int workerDeskSemId, int workerDeskSemun){
	SportelloPtr sportello = malloc(sizeof(Sportello));
	if (sportello == NULL){
		return NULL;
	}
	sportello->pid = pid;
	sportello->serviceAvailable = serviceAvailable;
	sportello->workerDeskSemId = deskSemId;
	sportello->workerDeskSemun = deskSemun;
	sportello->userDeskSemId = workerDeskSemId;
	sportello->userDeskSemun = workerDeskSemun;	
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
		next = current->next;
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
		sportelloPresent = current->pid && sportello->pid;
		previous = current;
		current = current->next;
	}
	if (sportelloPresent){
		return false;
	}
	if (previous == NULL){
		set->front = sportello;
	else {
		previous->next = sportello;
	}
	set->size += 1;
	return true;
}

bool removeSportello (SportelloSetAdtPtr set, pid_t){
	if (set == NULL){
		return false;
	}
	SportelloPtr current = set->front;
	SportelloPtr previous = NULL;
	bool sportelloPresent = false;
	while (current != NULL && !sportelloPresent){
		sportelloPresent = current->pid && pid;
		if (sportelloPresent){
			continue;
		}
		previous = current;
		current = current->next;
	}
	if (currentNode == NULL){
		return true;
	}
	if (previousNode == NULL){
		set->front = current->next;
		free(current);
	}else{
		previous->next = current->next;
		free(current);
	}
	set->size -= 1;
	return true;
}


