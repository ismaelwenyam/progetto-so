#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "logapi.h"

void log_time()
{
	struct timeval tv;
	struct tm *tm_info;
	char buffer[40];

	gettimeofday(&tv, NULL);

	tm_info = localtime(&tv.tv_sec);

	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);

	printf("%s.%03ld - ",
		   buffer,
		   tv.tv_usec / 1000);
}

void slog(int group, const char *format, ...)
{
#ifdef DEBUG
	FILE *logFile = fopen("./logs.txt", "a");
	if (logFile == NULL)
	{
		printf("couldn't open logs.txt\n");
	}
	log_time();

	va_list args;
	va_start(args, format);

	char buffer[256];
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	const char *color;
	switch (group)
	{
	case DIRETTORE:
		color = ANSI_COLOR_CYAN;
		break;
	case EROGATORE:
		color = ANSI_COLOR_GREEN;
		break;
	case SPORTELLO:
		color = ANSI_COLOR_YELLOW;
		break;
	case OPERATORE:
		color = ANSI_COLOR_BLUE;
		break;
	default:
		color = ANSI_COLOR_MAGENTA;
		break;
	}

	printf("%s%s%s\n", color, buffer, ANSI_COLOR_RESET);
	if (logFile != NULL)
	{
		fprintf(logFile, "%s\n", buffer);
		fclose(logFile);
	}
#endif
}
