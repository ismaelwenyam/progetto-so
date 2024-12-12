#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "logapi.h"

void log_time() {
    struct timeval tv;
    struct tm* tm_info;
    char buffer[40];

    gettimeofday(&tv, NULL);

    tm_info = localtime(&tv.tv_sec);

    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
    
    printf("%s.%03ld - ", 
           buffer, 
           tv.tv_usec / 1000);
}

void slog(int group, const char *format, ...) {
	log_time();
	
	va_list args;
    	va_start(args, format);
    
    	char buffer[256];
    	vsprintf(buffer, format, args);
    	va_end(args);

	if (group == DIRETTORE)
		printf(ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET "\n", buffer);	
	else if (group == EROGATORE)
		printf(ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET "\n", buffer);
	else if (group == SPORTELLO)
		printf(ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET "\n", buffer);
	else if (group == OPERATORE)
		printf(ANSI_COLOR_BLUE "%s" ANSI_COLOR_RESET "\n", buffer);
	else
		printf(ANSI_COLOR_MAGENTA "%s" ANSI_COLOR_RESET "\n", buffer);
}
