
#include "globals.h"
#include "string.h"
#include "errno.h"
#include "cfg.h"
#include "tty.h"
#include "sock.h"
#include "conn.h"
#include "queue.h"
#ifdef LOG
#  include "log.h"
#endif
#define PACKAGE   "GL-RS485"
#define VERSION   "1.0"
extern char logfullname[];
int isdaemon = TRUE;

void usage(char *exename);

/* Server socket */
int server_sd = -1;
/* TTY related data storage variable */
ttydata_t tty;
/* Connections queue descriptor */
queue_t queue;

#ifndef HAVE_DAEMON
#include <fcntl.h>
#include <unistd.h>

int
daemon(nochdir, noclose)
  int nochdir, noclose;
{
  int fd;

  switch (fork()) {
    case -1:
      return (-1);
    case 0:
      break;
    default:
      _exit(0);
  }

  if (setsid() == -1)
    return (-1);

  if (!nochdir)
    (void)chdir("/");

  if (!noclose && (fd = open("/dev/null", O_RDWR, 0)) != -1) {
    (void)dup2(fd, STDIN_FILENO);
    (void)dup2(fd, STDOUT_FILENO);
    (void)dup2(fd, STDERR_FILENO);
    if (fd > 2)
      (void)close(fd);
  }
  return (0);
}
#endif

void
usage(char *exename)
{
  cfg_init();
  printf("%s-%s  \n\n"
   "Usage: %s [-h] [-d] "
#ifdef LOG
   "[-L logfile] [-v level]\n"
#endif
   "             [-p device]  [-s speed] [-m mode]    [-t tty timeout]\n"
   "             [-A address] [-P port]  [-C maxconn] [-T conn timeout]\n\n"
   "Options:\n"
   "  -h         : this help\n"
   "  -d         : don't daemonize\n"
#ifdef LOG
   "  -L logfile : set log file name (default %s%s, \n"
#ifdef DEBUG
   "  -v level   : set log level (0-9, default %d, 0 - errors only)\n"
#else
   "  -v level   : set log level (0-2, default %d, 0 - errors only)\n"
#endif
#endif
   "  -c cfgfile : read configuration from cfgfile\n"
   "  -p device  : set serial port device name (default %s)\n"
   "  -s speed   : set serial port speed (default %d)\n"
   "  -m mode    : set serial port mode (default %s)\n"
   "  -A address : set TCP server address to bind (default %s)\n"
   "  -P port    : set TCP server port number (default %d)\n"
   "  -M mode    : set socket connection mode\n"
   "  -S string  : send string & receive string \n"
   "  -H hex     : send hex &receive hex  \n"
   "  -t timeout : set tty timeout value in milliseconds\n"
#ifdef TRXCTL
   "  -t         : enable RTS RS-485 data direction control using RTS\n"
   "  -y         : enable RTS RS-485 data direction control using sysfs file, active transmit\n"
   "  -Y         : enable RTS RS-485 data direction control using sysfs file, active receive\n"
#endif
   "  -C maxconn : set maximum number of simultaneous TCP connections\n"
   "               (1-%d, default %d)\n"
   "  -T timeout : set connection timeout value in seconds\n"
   "               (0-%d, default %d, 0 - no timeout)"
   "\n", PACKAGE, VERSION, exename,
#ifdef LOG
      LOGPATH, LOGNAME, cfg.dbglvl,
#endif
      cfg.ttyport, cfg.ttyspeed, cfg.ttymode,
      cfg.serveraddr, cfg.serverport,
      MAX_MAXCONN, cfg.maxconn,
      MAX_CONNTIMEOUT, cfg.conntimeout);
  exit(0);
}

int
main(int argc, char *argv[])
{
  int err = 0, rc, err_line;
  char *exename;
  char ttyparity;
  char ttybuf[256] = {0};

  cfg_init();

  if ((exename = strrchr(argv[0], '/')) == NULL)
    exename = argv[0];
  else
    exename++;

  /* command line argument list parsing */
  while ((rc = getopt(argc, argv,
               "dh"
#ifdef TRXCTL
               "ty:Y:"
#endif
#ifdef LOG
               "v:L:"
#endif
               "p:s:m:A:P:C:T:S:H:c:")) != RC_ERR)
  {
    switch (rc)
    {
      case '?':
        exit(-1);
      case 'd':
        isdaemon = FALSE;
        break;
      case 't':
        strncpy(cfg.ttytimeout, optarg, INTBUFSIZE);;
        break;
#ifdef TRXCTL
      case 't':
        cfg.trxcntl = TRX_RTS;
        break;
      case 'y':
        cfg.trxcntl = TRX_SYSFS_1;
        strncpy(cfg.trxcntl_file, optarg, INTBUFSIZE);
	break;
      case 'Y':
        cfg.trxcntl = TRX_SYSFS_0;
        strncpy(cfg.trxcntl_file, optarg, INTBUFSIZE);
	break;
#endif
#ifdef LOG
      case 'v':
        cfg.dbglvl = (char)strtol(optarg, NULL, 0);
#  ifdef DEBUG
        if (cfg.dbglvl > 9)
        { /* report about invalid log level */
          printf("%s: -v: invalid loglevel value"
                 " (%d, must be 0-9)\n", exename, cfg.dbglvl);
#  else
        if (cfg.dbglvl < 0 || cfg.dbglvl > 9)
        { /* report about invalid log level */
          printf("%s: -v: invalid loglevel value"
                 " (%d, must be 0-2)\n", exename, cfg.dbglvl);
#  endif
          exit(-1);
        }
        break;
      case 'L':
        if (*optarg != '/')
        {
          if (*optarg == '-')
          {
            /* logging to file disabled */
            *cfg.logname = '\0';
          }
          else
          { /* concatenate given log file name with default path */
            strncpy(cfg.logname, LOGPATH, INTBUFSIZE);
            strncat(cfg.logname, optarg, INTBUFSIZE - strlen(cfg.logname));
          }
        }
        else strncpy(cfg.logname, optarg, INTBUFSIZE);
        break;
#endif
      case 'p':
        if (*optarg != '/')
        { /* concatenate given port name with default
             path to devices mountpoint */
          strncpy(cfg.ttyport, "/dev/", INTBUFSIZE);
          strncat(cfg.ttyport, optarg, INTBUFSIZE - strlen(cfg.ttyport));
        }
        else strncpy(cfg.ttyport, optarg, INTBUFSIZE);
        break;
      case 's':
        cfg.ttyspeed = strtoul(optarg, NULL, 0);
        break;
      case 'm':
        strncpy(cfg.ttymode, optarg, INTBUFSIZE);
        /* tty mode sanity checks */
        if (strlen(cfg.ttymode) != 3)
        {
          printf("%s: -m: invalid serial port mode ('%s')\n",
                 exename, cfg.ttymode);
          exit(-1);
        }
        if (cfg.ttymode[0] != '8')
        {
          printf("%s: -m: invalid serial port character size "
              "(%c, must be 8)\n",
              exename, cfg.ttymode[0]);
          exit(-1);
        }
        ttyparity = toupper(cfg.ttymode[1]);
        if (ttyparity != 'N' && ttyparity != 'E' && ttyparity != 'O')
        {
          printf("%s: -m: invalid serial port parity "
              "(%c, must be N, E or O)\n", exename, ttyparity);
          exit(-1);
        }
        if (cfg.ttymode[2] != '1' && cfg.ttymode[2] != '2')
        {
          printf("%s: -m: invalid serial port stop bits "
              "(%c, must be 1 or 2)\n", exename, cfg.ttymode[2]);
          exit(-1);
        }
        break;
      case 'A':
        strncpy(cfg.serveraddr, optarg, INTBUFSIZE);
        break;
      case 'P':
        cfg.serverport = strtoul(optarg, NULL, 0);
        break;
      case 'C':
        cfg.maxconn = strtoul(optarg, NULL, 0);
        if (cfg.maxconn < 1 || cfg.maxconn > MAX_MAXCONN)
        { /* report about invalid max conn number */
          printf("%s: -C: invalid maxconn value"
                 " (%d, must be 1-%d)\n", exename, cfg.maxconn, MAX_MAXCONN);
          exit(-1);
        }
        break;
      case 'T':
        cfg.conntimeout = strtoul(optarg, NULL, 0);
        if (cfg.conntimeout > MAX_CONNTIMEOUT)
        { /* report about invalid conn timeout value */
          printf("%s: -T: invalid conn timeout value"
                 " (%d, must be 1-%d)\n", exename, cfg.conntimeout, MAX_CONNTIMEOUT);
          exit(-1);
        }
        break;
      case 'S':
	strncpy(ttybuf, optarg, 256);
        tty_write_read(ttybuf,strlen(ttybuf),'S');
        exit(0);
      case 'H':
	strncpy(ttybuf, optarg, 256);
        tty_write_read(ttybuf,strlen(ttybuf),'H');
        exit(0);
      case 'M':
        strncpy(cfg.connmode, optarg, INTBUFSIZE);
        break;
      case 'h':
        usage(exename);
        break;
    }
  }

#ifdef LOG
  if (log_init(cfg.logname) != RC_OK)
  {
    printf("%s: can't open logfile '%s' (%s), exiting...\n",
           exename,
           logfullname[0] ? logfullname : "no log name was given",
           strerror(errno));
    exit(-1);
  }
  logw(2, "%s-%s started...", PACKAGE, VERSION);
#endif

  if (conn_init())
  {
#ifdef LOG
    err = errno;
    logw(2, "conn_init() failed, exiting...");
#endif
    exit(err);
  }

  /* go or not to daemon mode? */
  if (isdaemon && (rc = daemon(TRUE, FALSE)))
  {
#ifdef LOG
    logw(0, "Can't be daemonized (%s), exiting...", strerror(errno));
#endif
    exit(rc);
  }

  conn_loop();

  err = errno;
#ifdef LOG
  logw(2, "%s-%s exited...", PACKAGE, VERSION);
#endif
  return (err);
}
