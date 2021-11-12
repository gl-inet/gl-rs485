#ifndef _LOG_H_
#define _LOG_H_


#include "globals.h"
#include "cfg.h"

extern int isdaemon;

#define LOGPATH "/tmp/log/"
#define LOGNAME "rs485.log"

int log_init(char *logname);

extern const char *g_master_file;
extern int         g_master_line;
extern const char *g_master_fun;
extern const char *g_master_lev;

#define EASYLOG_DATE    (1 << 0)
#define EASYLOG_TIME    (1 << 1)
#define EASYLOG_FILE    (1 << 2)
#define EASYLOG_LINE    (1 << 3)
#define EASYLOG_FUNC    (1 << 4)
#define EASYLOG_LEVEL   (1 << 5)
#define LOG_FILE_MAX_SIZE   512000000UL

int easylog_flag_add(int flag);
//int easylog_flag_rm(int flag);
int easylog_file(const char *logfile);
int easylog_write(char level, const char *fmt, ...);

#define logw(level ,fmt, arg...) \
    do{\
        g_master_file = __FILE__;\
        g_master_line = __LINE__;\
        g_master_fun  = __func__;\
        g_master_lev  = "DEBUG";\
        easylog_write(level,fmt, ##arg);\
    }while(0)
#endif

