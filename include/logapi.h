#ifndef LOGAPI_H
#define LOGAPI_H

#include <stdarg.h>

#define ANSI_COLOR_CYAN "\x1b[36m"    /* direttore logs */
#define ANSI_COLOR_GREEN "\x1b[32m"   /* erogatore logs */
#define ANSI_COLOR_YELLOW "\x1b[33m"  /* sportello logs */
#define ANSI_COLOR_BLUE "\x1b[34m"    /* operatore logs */
#define ANSI_COLOR_MAGENTA "\x1b[35m" /* utente logs */
#define ANSI_COLOR_RESET "\x1b[0m"    /* reset logs color */

#define DIRETTORE 'd'
#define EROGATORE 'e'
#define SPORTELLO 's'
#define OPERATORE 'o'
#define UTENTE 'u'

void log_time();
void slog(int, const char *, ...);

#endif
