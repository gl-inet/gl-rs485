
#ifndef _TTY_H
#define _TTY_H

#include "globals.h"
#include "cfg.h"

/*
 * Delay value calculation macros.
 * c - number of characters
 * b - bits per character
 * s - bits per second
 */
#define	DV(c, b, s) (c * b * 1000000l / s)

/*
 * Default tty port parameters
 */
#if defined (__CYGWIN__)
#  define DEFAULT_PORT "/dev/COM1"
#elif defined (__linux__)
#  define DEFAULT_PORT "/dev/ttyS0"
#else
#  define DEFAULT_PORT "/dev/cuaa0"
#endif

#define DEFAULT_SPEED 9600
#define DEFAULT_BSPEED B9600

#define DEFAULT_MODE "8N1"

#define DEFAULT_BITS_PER_CHAR 10

/*
 * Maximum tty buffer size
 */
#define TTY_BUFSIZE 256

/*
 * TRX control types
 */
#ifdef  TRXCTL
#define TRX_ADDC    0
#define TRX_RTS     1
#define TRX_SYSFS_1 2
#define TRX_SYSFS_0 3
#endif

/*
 * TTY device FSM states
 */
#define TTY_PAUSE 0
#define TTY_READY 1
#define TTY_RQST  2
#define TTY_RESP  3
#define TTY_PROC  4

/*
 * TTY related data storage structure
 */
typedef struct
{
  int fd;                       /* tty file descriptor */
  int speed;                    /* serial port speed */
  char *port;                   /* serial port device name */
  int bpc;                      /* bits per character */
#ifdef TRXCTL
  int trxcntl;                  /* trx control type (enum - see values in config.h) */
#endif
  struct termios tios;          /* working termios structure */
  struct termios savedtios;     /* saved termios structure */
  int state;                    /* current state */
  unsigned int trynum;             /* try counter */
//  unsigned long timer;          /* time tracking variable */
  unsigned int txlen;           /* tx data length */
  unsigned int rxlen;           /* rx data length */
  unsigned char ptrbuf;         /* ptr in the buffer */
  unsigned char rxoffset;       /* ptr in the buffer */
  unsigned char txbuf[TTY_BUFSIZE]; /* transmitting buffer */
  unsigned char rxbuf[TTY_BUFSIZE]; /* receiving buffer */
} ttydata_t;

/* prototypes */
void tty_sighup(void);
void tty_init(ttydata_t *mod);
int tty_open(ttydata_t *mod);
int tty_set_attr(ttydata_t *mod);
speed_t tty_transpeed(int speed);
int tty_cooked(ttydata_t *mod);
int tty_close(ttydata_t *mod);
void tty_set_rts(int fd);
void tty_clr_rts(int fd);
void tty_delay(int usec);

#endif /* _TTY_H */
