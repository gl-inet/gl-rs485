#ifndef _CFG_H
#define _CFG_H

#include "globals.h"
#include "log.h"
#include "tty.h"
#include "conn.h"

/* Global configuration storage structure */
typedef struct {
    /* debug level */
    char dbglvl;
    /* tty speed */
    int ttyspeed;
    int ttytimeout;
    /* trx control type (0 - ADDC, 1 - by RTS, 2 - by sysfs GPIO with 1 activating transmit, 3 - by sysfs GPIO with 0 activating transmit) */
    int trxcntl;
    int conntimeout;
    /* TCP server port number */
    int serverport;
    /* maximum number of connections */
    int maxconn;
    /* number of tries of request in case timeout (0 - no tries attempted) */
    int maxtry;
    /* staled connection timeout (in sec) */
    int mqttqos;
    int mqttautoconn;
    int mqttautoconnmaxtime;
    int mqttautoconninteval;
    int mqtttimeout;
    int mqttinterval;
    char ttytype[8];
    /* tty mode */
    char ttymode[8];
    /* TCP/UDP server/client */
    char connmode[8];
    /* tty port name */
    char ttyport[128];
    /* log file name */
    char logname[INTBUFSIZE + 1];
    /* trx control sysfs file */
    char trxcntl_file[INTBUFSIZE + 1];
    /* TCP server address */
    char serveraddr[INTBUFSIZE + 1];

    char mqttaddr[INTBUFSIZE + 1];
    char mqttusername[INTBUFSIZE + 1];
    char mqttpassword[INTBUFSIZE + 1];
    char mqttsubscribe[INTBUFSIZE + 1];
    char mqttpublish[INTBUFSIZE + 1];
    char mqttclientid[INTBUFSIZE + 1];

} cfg_t;

extern cfg_t cfg;
void cfg_init(void);
unsigned char my_hex_str_to_i_l(char *s, unsigned char len, unsigned char offset);
#endif /* _CFG_H */

