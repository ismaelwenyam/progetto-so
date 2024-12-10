#include <stdio.h>
#include <unistd.h>



#include "simerr.h"
#include "ticket.h"
#include "semapi.h"
#include "config_sim.h"
#include "sim_stats.h"


int main (int argc, char **argv){
	ConfigurationAdt configuration = get_config();
	printf("operatore.%d.configuration.nof_pause: %d\n", getpid(), configuration.nofPause);
	// compete to find desk
	
	








}
