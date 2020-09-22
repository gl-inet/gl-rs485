
#ifndef _GLOBALS_H
#define _GLOBALS_H


#define LOG 1
#define DEBUG 1
#define PACKAGE   "GL-RS485"
#define VERSION   "1.0"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <fcntl.h>
#include <ctype.h>
#ifdef HAVE_LIBUTIL
#  include <libutil.h>
#endif

/*
 * Useful min/max macroses
 */
#define MAX(a, b) ( (a > b) ? a : b )
#define MIN(a, b) ( (a < b) ? a : b )

/*
 * Boolean constants
 */
#define FALSE 0
#define TRUE  !FALSE

/*
 * Constants
 */
#define RC_OK       0
#define RC_ERR     -1
#define RC_BREAK   -2
#define RC_TIMEOUT -3
#define RC_AOPEN   -4
#define RC_ACLOSE  -5

/* Internal string buffers size */
#if defined(PATH_MAX)
#  define INTBUFSIZE PATH_MAX
#else
#  define INTBUFSIZE 63
#endif

#endif
