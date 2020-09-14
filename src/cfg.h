
#ifndef _CFG_H
#define _CFG_H

#include "globals.h"
#include "log.h"
#include "tty.h"
#include "conn.h"


/* Global configuration storage structure */
typedef struct
{
#ifdef LOG
  /* debug level */
  char dbglvl;
  /* log file name */
  char logname[INTBUFSIZE+1];
#endif
  /* tty port name */
  char ttyport[INTBUFSIZE+1];
  /* tty speed */
  int ttyspeed;
  int ttytimeout;
  /* tty mode */
  char ttymode[INTBUFSIZE+1];
  /* trx control type (0 - ADDC, 1 - by RTS, 2 - by sysfs GPIO with 1 activating transmit, 3 - by sysfs GPIO with 0 activating transmit) */
  int trxcntl;
  /* trx control sysfs file */
  char trxcntl_file[INTBUFSIZE+1];
  /* TCP server address */
  char serveraddr[INTBUFSIZE+1];
  /* TCP/UDP server/client */
  char connmode[INTBUFSIZE+1];
  /* TCP server port number */
  int serverport;
  /* maximum number of connections */
  int maxconn;
  /* number of tries of request in case timeout (0 - no tries attempted) */
  int maxtry;
  /* staled connection timeout (in sec) */
  int conntimeout;
} cfg_t;

extern cfg_t cfg;
void cfg_init(void);
unsigned char my_hex_str_to_i_l(char *s,unsigned char len,unsigned char offset);
#endif /* _CFG_H */
