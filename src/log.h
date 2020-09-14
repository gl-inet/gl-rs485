
#ifndef _LOG_H
#define _LOG_H

#include "globals.h"

#define LOG 1
#define DEBUG 1

extern int isdaemon;

#define LOGPATH "/tmp/log/"
#define LOGNAME "rs485.log"

#ifdef LOG
int log_init(char *logname);
int log_app(char *logname, char *string);
void logw(int level, char *fmt, ...);
#else
    #define logw(fmt, ...) {}
#endif

#endif /* _LOG_H */
