
Usage:
------
```
Usage: gl-rs485 [-h] [-d]
             [-L logfile] [-v level]
             [-p port   ] [-s speed] [-m mode] [-t tty timeout]
             [-A address] [-P port ] [-T conn timeout]

Options:
  -h         : this help
  -d         : don't daemonize
  -L logfile : set log file name (default /tmp/log/rs485.log) 
  -v level   : set log level (0-5, default 0, 0 - no log)
  -p port    : set serial port device name (default /dev/ttyUSB0)
  -s speed   : set serial port speed (default 9600)
  -m mode    : set serial port mode (default 8n1)
  -B begin   : begin connection (socket/mqtt default socket)
  -A address : set TCP/UDP server address to bind (default 192.168.8.1)
  -P port    : set TCP/UDP server port number (default 502)
  -M mode    : set socket connection mode
  -S string  : send string & receive string 
  -H hex     : send hex &receive hex  
  -t timeout : set tty timeout value in milliseconds
  -T timeout : set socket timeout value in seconds
               (0-86400, default 60, 0 - no timeout

```
------ 
```             
tty mode :[7-8][n/e/o][1/2]    n: no parity e:even parity o: odd parity
socket mode: tcps tcpc udp     TCP server/client 
```
