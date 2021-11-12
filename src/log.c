#include "log.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <sys/time.h>
#include <errno.h>
#include <gl/guci2.h>

char g_logfile[128] = {0};
char g_logfile1[128] = {0};
int  g_master_fd = -1;
char  cur_logfile = 0;
FILE *g_master_fp = NULL;
int  g_master_flag = 0;
const char *g_master_file = NULL;
int g_master_line = -1;
const char *g_master_fun = NULL;
const char *g_master_lev = NULL;

char logfullname[INTBUFSIZE + 1];

int log_init(char *logname)
{
    int maxlen = INTBUFSIZE;

    /* checking log file name */
    if (*logname == '/')
        strncpy(logfullname, logname, maxlen);
    else {
        if (!*logname) {
            /* logfile isn't needed */
            *logfullname = '\0';
            return RC_OK;
        }
        /* append default log path */
        strncpy(logfullname, LOGPATH, maxlen);
        maxlen -= strlen(logfullname);
        strncat(logfullname, logname, maxlen);
    }
    easylog_file(logfullname);
    easylog_flag_add(EASYLOG_LEVEL | EASYLOG_DATE | EASYLOG_TIME | EASYLOG_FILE | EASYLOG_LINE | EASYLOG_FUNC);
    return RC_OK;
}


int get_my_name(char *file_name, int len)
{
    if (NULL == file_name || len < 0) {
        return -1;
    }
    int ret = readlink("/proc/self/exe", file_name, len);
    if (ret < 0) {
        return -1;
    }
    file_name[ret] = '\0';
    strcat(file_name, ".log");

    return 0;
}

int easylog_file(const char *logfile)
{
    if (NULL == logfile || strlen(logfile) == 0) {
        return get_my_name(g_logfile, sizeof(g_logfile));
    }
    snprintf(g_logfile, sizeof(g_logfile), "%s", logfile);
    snprintf(g_logfile1, sizeof(g_logfile), "%stmp", logfile);
    return 0;
}

int easylog_open_file()
{
    if (strlen(g_logfile) == 0) {
        easylog_file(NULL);
    }
    if (strlen(g_logfile) == 0) {
        return -1;
    }
    g_master_fp = fopen(g_logfile, "at");
    if (NULL == g_master_fp) {
        return -1;
    }
    long length = 0;
    fseek(g_master_fp, 0, SEEK_END);
    length = ftell(g_master_fp);

    if (length > LOG_FILE_MAX_SIZE) {
        fclose(g_master_fp);
        int c = 0;
        FILE *f1 = fopen(g_logfile, "r");
        FILE *f2 = fopen(g_logfile1, "w");
        while ((c = fgetc(f1)) != EOF)
            fputc(c, f2);
        fclose(f1);
        fclose(f2);

        remove(g_logfile);
        g_master_fp = fopen(g_logfile, "at");
        if (NULL == g_master_fp) {
            return -1;
        }
    }
    g_master_fd = fileno(g_master_fp);
    return 0;
}

int file_stat_ok()
{
    if (NULL == g_master_fp) {
        return 0;
    }
    struct stat buf;
    errno = 0;
    int r = fstat(fileno(g_master_fp), &buf);
    if (0 == r) {
        return 1;
    }

    return 0;
}

int easylog_write_log(const char *fmt, va_list arg_list)
{
    if (0 == file_stat_ok()) {
        easylog_open_file();
    }
    if (0 == file_stat_ok()) {
        return -1;
    }
    struct timeval start;
    gettimeofday(&start, NULL);
    struct tm t;
    bzero(&t, sizeof(t));
    localtime_r(&start.tv_sec, &t);

    if (!isdaemon) g_master_fp = stderr;
    if (g_master_flag & EASYLOG_DATE) {
        fprintf(g_master_fp, "[%04d-%02d-%02d] ", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
    }
    if (g_master_flag & EASYLOG_TIME) {
        fprintf(g_master_fp, "[%02d:%02d:%02d.%03ld] ", t.tm_hour, t.tm_min, t.tm_sec, start.tv_usec / 1000);
    }
    if (g_master_flag & EASYLOG_FILE && NULL != g_master_file) {
        fprintf(g_master_fp, "[%s] ", g_master_file);
    }
    if (g_master_flag & EASYLOG_LINE && g_master_line > 0) {
        fprintf(g_master_fp, "[%d] ", g_master_line);
    }
    if (g_master_flag & EASYLOG_FUNC && NULL != g_master_fun) {
        fprintf(g_master_fp, "[%s()] ", g_master_fun);
    }
    if (g_master_flag) {
        fprintf(g_master_fp, "%s", "-- ");
    }
    if (g_master_flag & EASYLOG_LEVEL && NULL != g_master_lev) {
        if (!strcmp(g_master_lev, "DEBUG"))
            fprintf(g_master_fp, "\033[37m");
        else if (!strcmp(g_master_lev, "INFO"))
            fprintf(g_master_fp, "\033[32m");
        else if (!strcmp(g_master_lev, "WARNING"))
            fprintf(g_master_fp, "\033[33m");
        else if (!strcmp(g_master_lev, "ERROR"))
            fprintf(g_master_fp, "\033[31m");
        else if (!strcmp(g_master_lev, "CRITICAL"))
            fprintf(g_master_fp, "\033[31;45m");
        fprintf(g_master_fp, "[%s] ", g_master_lev);
        fprintf(g_master_fp, "\033[0m");
    }
    vfprintf(g_master_fp, fmt, arg_list);
    fprintf(g_master_fp, "\n");
    fflush(g_master_fp);

    return 0;
}

int easylog_write(char level, const char *fmt, ...)
{

    char cfg_log[8] = {0};
    struct uci_context *ctx = guci2_init();
    guci2_get(ctx, "rs485.rs485.log", cfg_log);
    cfg.dbglvl = strToNumber(cfg_log);

    if (0 == cfg.dbglvl) return;
    if (level < cfg.dbglvl) return;
    if (level == 1) {
        g_master_lev  = "DEBUG";
    } else if (level == 2) {
        g_master_lev  = "INFO";
    } else if (level == 3) {
        g_master_lev  = "WARNING";
    } else if (level == 4) {
        g_master_lev = "ERROR";
    } else if (level == 5) {
        g_master_lev = "CRITICAL";
    }

    va_list argptr;
    int ret;

    va_start(argptr, fmt);
    ret = easylog_write_log(fmt, argptr);
    va_end(argptr);

    return ret;
}

int easylog_flag_add(int flag)
{
    g_master_flag |= flag;

    return 0;
}

//int easylog_flag_rm(int flag) {
//    g_master_flag &= ~flag;

//   return 0;
//}

