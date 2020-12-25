#ifndef _CONN_H
#define _CONN_H

#include "globals.h"
#include "cfg.h"
#include "sock.h"
#include "log.h"

/*
 * Default values
 */
#define DEFAULT_SERVERADDR "192.168.8.1"
#define DEFAULT_SERVERPORT 502
#define DEFAULT_MAXCONN 32
#define DEFAULT_CONNTIMEOUT 60

/* Max simultaneous TCP connections to server */
#ifndef MAX_MAXCONN
#  define MAX_MAXCONN 32
#endif

/* Max connection timeout, in secs */
#ifndef MAX_CONNTIMEOUT
#  define MAX_CONNTIMEOUT 86400
#endif


#define BUFSIZE 262     /* size (in bytes) of MODBUS data */

/*
 * Client connection FSM states
 */
#define CONN_HEADER    0  /* reading frame header */
#define CONN_RQST_FUNC 1  /* reading request function code */
#define CONN_RQST_NVAL 2  /* reading request number of values (registers/coils) */
#define CONN_RQST_TAIL 3  /* reading request tail */
#define CONN_TTY       4  /* writing request to TTY */
#define CONN_RESP      5  /* reading response from TTY */

/*
 * Client connection related data storage structure
 */
typedef struct conn_t
{
  struct conn_t *prev;  /* linked list previous connection */
  struct conn_t *next;  /* linked list next connection */
  int sd;               /* socket descriptor */
  int state;            /* current state */
  int timeout;          /* timeout value, secs */
  char remote_addr[INET6_ADDRSTRLEN]; /* remote client address */
  int ctr;              /* counter of data in the buffer */
  int read_len;         /* length of modbus frame to read */
  unsigned char buf[BUFSIZE];    /* data buffer */
} conn_t;

/* prototypes */
int conn_init(void);
void conn_loop(void);
void conn_open(void);
conn_t *conn_close(conn_t *conn);
ssize_t tty_write_read(char *buf, size_t nbytes,char type);
ssize_t tty_write_file(char * file);

#endif /* _CONN_H */

