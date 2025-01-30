#include <stdio.h>
#include <stdlib.h>
#include "simerr.h"

void err_exit(char *err)
{
	fprintf(stderr, "error occured: %s (%d)\n", err == 0 ? strerror(errno) : err, errno);
	exit(EXIT_FAILURE);
}
