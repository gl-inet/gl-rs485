
Usage:
------
```
 gl-rs485 [-h] [-d] [-L logfile] [-v level]
             [-p device]  [-s speed] [-m mode]    [-t tty timeout]
             [-A address] [-P port]  [-C maxconn] [-T conn timeout]

Options:
  -h         : this help
  -d         : don't daemonize
  -L logfile : set log file name (default /tmp/log/rs485.log, 
  -v level   : set log level (0-9, default 2, 0 - errors only)
  -c cfgfile : read configuration from cfgfile
  -p device  : set serial port device name (default /dev/ttyS0)
  -s speed   : set serial port speed (default 9600)
  -m mode    : set serial port mode (default 8n1)
  -A address : set TCP server address to bind (default 192.168.8.1)
  -P port    : set TCP server port number (default 502)
  -M mode    : set socket connection mode
  -S string  : send string & receive string 
  -H hex     : send hex &receive hex  
  -t timeout : set tty timeout value in milliseconds
  -C maxconn : set maximum number of simultaneous TCP connections
               (1-32, default 32)
  -T timeout : set connection timeout value in seconds
               (0-1000, default 60, 0 - no timeout)
```
------ 
```             
tty mode :[7-8][n/e/o][1/2]    n: no parity e:even parity o: odd parity
socket mode: tcps tcpc udps udpc   TCP/UDP server/client 
```