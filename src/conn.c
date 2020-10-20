
#include "conn.h"
#include "cfg.h"
#include "uart.h"
#include "queue.h"
#include "state.h"
#include <sys/file.h>

/* global variables */
extern int server_sd;
extern queue_t queue;
extern ttydata_t tty;
extern cfg_t cfg;

conn_t *actconn; /* last active connection */
int max_sd; /* major descriptor in the select() sets */

ssize_t conn_read(int d, void *buf, size_t nbytes);
ssize_t conn_write(int d, void *buf, size_t nbytes, int istty);

#define FD_MSET(d, s) do { FD_SET(d, s); max_sd = MAX(d, max_sd); } while (0);

int tty_open(ttydata_t *mod)
{
	cfg_init();
	mod->fd = uartOpen(cfg.ttyport,cfg.ttyspeed,0,cfg.ttytimeout);
	if(mod->fd < 0){
		return RC_ERR;
	}

	return 0;
}
int tty_reopen()
{
  logw(0, "tty re-opening...");
  MyuartClose(tty.fd);
  if (tty_open(&tty) != RC_OK)
  {
    logw(3, "tty_reopen():"
           " can't open tty device %s (%s)",
           cfg.ttyport, strerror(errno));
    return RC_ERR;
  }
  logw(0, "tty re-opened.");
  return RC_OK;
}

void tty_reinit()
{
  logw(0, "trying to re-open tty...");
  tty_reopen();
}

/*
 * Connections startup initialization
 * Parameters: none
 * Return: RC_OK in case of success, RC_ERR otherwise
 */
int
conn_init(void)
{
  /* tty device initialization */
  if (tty_open(&tty) != RC_OK)
  {
    logw(4, "conn_init():"
           " can't open tty device %s (%s)",
           cfg.ttyport, strerror(errno));
    return RC_ERR;
  }

  /* create server socket */
  if ((server_sd = sock_create_server(cfg.serveraddr, cfg.serverport, TRUE)) < 0)
  {
    logw(4, "conn_init():"
           " can't create listen socket (%s)",
           (errno != 0) ? strerror(errno) : "failed");
    return RC_ERR;
  }

  /* connections queue initialization */
  queue_init(&queue);

  return RC_OK;
}

/*
 * Open new client connection
 * Parameters: none
 * Return: none
 */
void
conn_open(void)
{
  int sd;
  conn_t *newconn;
  struct sockaddr_storage rmt_addr;
  char ipstr[INET6_ADDRSTRLEN];

 if(!strncmp(cfg.connmode,"tcps",4)){
  if ((sd = sock_accept(server_sd, (struct sockaddr *)&rmt_addr,
                        sizeof(rmt_addr), TRUE)) == RC_ERR)
  { /* error in conn_accept() */
    logw(4, "conn_open(): error in accept() (%s)", strerror(errno));
    return;
  }
 }
 else{
   sd = server_sd;
 }
 inet_ntop(rmt_addr.ss_family, sock_addr((struct sockaddr *)&rmt_addr),
           ipstr, sizeof(ipstr));
  logw(2, "conn_open(): accepting connection from %s", ipstr);
  /* compare descriptor of connection with FD_SETSIZE */
  if (sd >= FD_SETSIZE)
  {
    logw(2, "conn_open(): FD_SETSIZE limit reached,"
           " connection from %s will be dropped", ipstr);
    close(sd);
    return;
  }
  /* check current number of connections */
  if (queue.len == cfg.maxconn)
  {
    logw(2, "conn_open(): number of connections limit reached,"
           " connection from %s will be dropped", ipstr);
    close(sd);
    return;
  }
  /* enqueue connection */
  newconn = queue_new_elem(&queue);
  newconn->sd = sd;
  memcpy((void *) &newconn->remote_addr, &ipstr, sizeof(ipstr));
  state_conn_set(newconn, CONN_HEADER);
}

/*
 * Close client connection
 * Parameters: CONN - ptr to connection to close
 * Return: pointer to next queue element
 */
conn_t *
conn_close(conn_t *conn)
{
  conn_t *nextconn;
  logw(3, "conn_close(): closing connection from %s", conn->remote_addr);
  /* close socket */
  close(conn->sd);
  /* get pointer to next element */
  nextconn = queue_next_elem(&queue, conn);
  /* dequeue connection */
  queue_delete_elem(&queue, conn);
  if (actconn == conn) actconn = nextconn;
  return nextconn;
}


/*
 * Read() wrapper. Read nomore BYTES from descriptor D in buffer BUF
 * Return: number of successfully read bytes,
 *         RC_ERR in case of error.
 */
ssize_t
tty_read(int d, void *buf, size_t nbytes)
{
	int ret = 8;
	int count = 0;

	while(ret==8){
		ret = MyuartRxExpires(d,200,&buf[0+count],cfg.ttytimeout);
		count +=ret;
	}
	return count;
}

int len_ud = 0 ;
struct sockaddr_in caddr;

ssize_t
conn_read(int d, void *buf, size_t nbytes)
{
  int rc = 0 ;
  len_ud = sizeof(struct sockaddr);
  do
  { /* trying read from descriptor while breaked by signals */
  if(strncmp(cfg.connmode,"udp",3)){
    rc = read(d, buf, nbytes);
  }
 else{
    rc = recvfrom(d, buf, nbytes, 0, (struct sockaddr*)&caddr, &len_ud);
    logw(0, "udp: addr:%s   %d",inet_ntoa(caddr.sin_addr),d);

  }
 } while (rc == -1 && errno == EINTR);
  return (rc < 0) ? RC_ERR : rc;
}

#include <stdio.h>
#include <stdlib.h>


/*
 * Write() wrapper. Write no more than BYTES to descriptor D from buffer BUF
 * Return: number of successfully written bytes,
 *         RC_ERR in case of error.
 */
ssize_t
tty_write(int d, void *buf, size_t nbytes)
{

	int time_out = 0; 
	int rc = 0;
        if(d < 0 )
        {
                return -1;
        }

        while( flock(d,LOCK_EX|LOCK_NB) != 0 ){//get file lock
                if( ++time_out > 30 ){//time out
                        MyuartClose(d);
                        return -2;
                }
                usleep(1000);
        }
        MyflushIoBuffer(d);
       	rc = MyuartTxNonBlocking(d,nbytes,buf);

	return (rc < 0) ? RC_ERR : rc+nbytes;
}

ssize_t
tty_write_read(char * buf, size_t nbytes,char type)
{
	int fd = 0;
        int ret = 8;
        int count = 0;
	unsigned write_buff[512]={0};
	fd = uartOpen(cfg.ttyport,cfg.ttyspeed,0,cfg.ttytimeout);

	if(type=='H'){
		if(nbytes%2){
			printf("date len err\n");
			return -1;
		}
		gl_str2acsll(buf,nbytes,write_buff);
		tty_write(fd,write_buff,nbytes/2);
	}
	else{
		tty_write(fd,buf,nbytes);
	}

	unsigned  char rec_buff[512] = {0};
        while(ret==8){
                ret = MyuartRxExpires(fd,200,&rec_buff[0+count],cfg.ttytimeout);
                count +=ret;
        }
	usleep(1000);
	MyuartClose(fd);

	if(type=='H'){
		char alldata[1024] = {0};
		gl_hex2str(rec_buff,count,alldata);
		printf("rec:%s\n",alldata);
	}
	else{
		printf("rec:%s\n",rec_buff);
	}
        return count;
}

ssize_t tty_write_file(char * file)
{
	char completion =0;
	int up_cont=0;
        int fd = 0;
        FILE* fp = 0;
        int read_count = 0;
	int file_count =0;
        unsigned char tx_buf[512]={0};

        fd = uartOpen(cfg.ttyport,cfg.ttyspeed,0,cfg.ttytimeout);
	fp=fopen(file,"rb");

	if(NULL==fp){
		printf("file open err,please check file & try again\n");
		return -1;
	}

	fseek(fp,0,SEEK_END);

	file_count = ftell(fp);

	printf("file size : %d\n",file_count);
	file_count =((file_count/512)+1)*512;

	fseek(fp,0L,SEEK_SET);
	while(1){
            memset(tx_buf,'\0',sizeof(tx_buf));
            read_count = fread(tx_buf,1, sizeof(tx_buf),fp);
		printf("read : %d\n",read_count);

            if(512==read_count){
                MyuartTx(fd,512,tx_buf);
	        usleep(1000);
                up_cont++;
                if(up_cont==(file_count/5120)){
                    up_cont = 0;
                    completion++;
                    if(completion<=9)
                        printf("completion : %d%%\n",completion*10);
                }
	    }
            else if(512>read_count){
                MyuartTx(fd,read_count,tx_buf);
		printf("completion : 100%%\n");
		break;
	    }
	}
	sleep(1);
	MyuartClose(fd);
	fclose(fp);

	char *date_rc =  getShellCommandReturnDynamic("date '+%Y-%m-%d %H:%M:%S'");
	printf("MCU update completed:%s\n",date_rc);

	return 0;

}

ssize_t
conn_write(int d, void *buf, size_t nbytes, int istty)
{
  int rc;
  fd_set fs;
#ifdef TRXCTL
  if (istty && cfg.trxcntl != TRX_ADDC)
  {
    tty_set_rts(d);
    tty_delay(35000000l/cfg.ttyspeed);
  }
#endif

  FD_ZERO(&fs);
  FD_SET(d, &fs);
  do
  {
  if(strncmp(cfg.connmode,"udp",3)){
    rc = write(d, buf, nbytes);
  }
  else{
    d = server_sd;
    rc =sendto(d, buf,nbytes, 0, (struct sockaddr*)&caddr, len_ud);
    logw(0, "udp: addr1:%s   %d",inet_ntoa(caddr.sin_addr),d);
  }
  } while (rc == -1 && errno == EINTR);

#ifdef TRXCTL
  if (istty && cfg.trxcntl != TRX_ADDC )
  {
    tty_delay(DV(nbytes, tty.bpc, cfg.ttyspeed));
    tty_clr_rts(d);
  }
#endif

  return (rc < 0) ? RC_ERR : rc;

}

#if 0
/*
 * Select() wrapper with signal checking.
 * Return: number number of ready descriptors,
 *         RC_ERR in case of error.
 */
int
conn_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
            struct timeval *timeout)
{
  int rc;
  do
  { /* trying to select() while breaked by signals */
    rc = select(nfds, readfds, writefds, exceptfds, timeout);
  } while (rc == -1 && errno == EINTR);
  return (rc < 0) ? RC_ERR : rc;
}
#endif

/*
 * Connections serving loop
 * Parameters: none
 * Return: none
 */
void
conn_loop(void)
{
  int rc, max_sd, len, min_timeout;
  unsigned int i;
  fd_set sdsetrd, sdsetwr;
  struct timeval ts, tts, t_out;
  unsigned long tval, tout_sec, tout = 0ul;
  conn_t *curconn = NULL;
  char t[1025], v[5];

  while (TRUE)
  {
    /* update FD_SETs */
    FD_ZERO(&sdsetrd);
    max_sd = server_sd;
    FD_MSET(server_sd, &sdsetrd);
    FD_ZERO(&sdsetwr);

    /* update FD_SETs by TCP connections */
    len = queue.len;
    curconn = queue.beg;
    min_timeout = cfg.conntimeout;
    while (len--)
    {
      switch (curconn->state)
      {
        case CONN_HEADER:
        case CONN_RQST_FUNC:
        case CONN_RQST_NVAL:
        case CONN_RQST_TAIL:
          FD_MSET(curconn->sd, &sdsetrd);
          break;
        case CONN_RESP:
          FD_MSET(curconn->sd, &sdsetwr);
          break;
      }
      min_timeout = MIN(min_timeout, curconn->timeout);
      curconn = queue_next_elem(&queue, curconn);
    }

      t_out.tv_usec = 0ul;
      if (cfg.conntimeout)
        t_out.tv_sec = min_timeout; /* minor timeout value */
      else
        t_out.tv_sec = 10ul; /* XXX default timeout value */

    (void)gettimeofday(&ts, NULL); /* make timestamp */

    logw(2, "conn_loop(): select(): max_sd = %d, t_out = %06lu:%06lu ",
           max_sd, t_out.tv_sec, t_out.tv_usec);
    rc = select(max_sd + 1, &sdsetrd, &sdsetwr, NULL, &t_out);
    logw(2, "conn_loop(): select() returns %d ", rc);
    if (rc < 0)
    { /* some error caused while select() */
      if (errno == EINTR) continue; /* process signals */
      /* unrecoverable error in select(), exiting */
      logw(4, "conn_loop(): error in select() (%s)", strerror(errno));
/*      break; */
    }

    /* calculating elapsed time */
    (void)gettimeofday(&tts, NULL);
    tval = 1000000ul * (tts.tv_sec - ts.tv_sec) +
                       (tts.tv_usec - ts.tv_usec);

    if (cfg.conntimeout)
    { /* expire staled connections */
      tout += tval;
      tout_sec = tout / 1000000ul;
      if (tout_sec)
      { /* at least one second elapsed, check for staled connections */
        len = queue.len;
        curconn = queue.beg;
        while (len--)
        {
          curconn->timeout -= tout_sec;
          if (curconn->timeout <= 0)
          { /* timeout expired */
            if (curconn->state == CONN_TTY)
            { /* deadlock in CONN_TTY state, make attempt to reinitialize serial port */
              logw(3, "conn[%s]: state CONN_TTY deadlock.", curconn->remote_addr);
              tty_reinit();
            }
            /* purge connection */
            logw(3, "conn[%s]: timeout, closing connection", curconn->remote_addr);
            curconn = conn_close(curconn);
            continue;
          }
          curconn = queue_next_elem(&queue, curconn);
        }
        tout = tout % 1000000ul;
      }
    }
    /* checking for pending connections */
    if (FD_ISSET(server_sd, &sdsetrd)) conn_open();

    if (rc == 0){
      continue;	/* timeout caused, we will do select() again */
    }
    len = queue.len;
    curconn = queue.beg;
    while (len--)
    {
      switch (curconn->state)
      {
	case CONN_HEADER:
        case CONN_RQST_FUNC:
        case CONN_RQST_NVAL:
        case CONN_RQST_TAIL:
		if (FD_ISSET(curconn->sd, &sdsetrd)){
			rc = conn_read(curconn->sd,curconn->buf + curconn->ctr,curconn->read_len - curconn->ctr);
			logw(2, "conn read count (%d,%d)",rc,curconn->read_len);
			if (rc < 0){ // error - drop this connection and go to next queue element 
				curconn = conn_close(curconn);
				break;
			}
			else{
				FD_MSET(tty.fd, &sdsetwr);
				tty.txlen = rc;
				memcpy((void *)(tty.txbuf),(void *)(curconn->buf), tty.txlen);

				t[0] = '\0';
				int i;
				for (i = 0; i < rc; i++) {
				  sprintf(v, "[%2.2x]", curconn->buf[i]);
				  strncat(t, v, 1024-strlen(t));
				}
				logw(2, "conn[%s]: request: %s", curconn->remote_addr, t);

				remove_blank1(tty.txbuf,tty.txlen);
				tty.txlen = strlen(tty.txbuf);
				if(tty.txlen%2){
					strncpy(curconn->buf,"data format error",17);
					tty.rxlen = 17;
					logw(0,"data format error \n");
					state_conn_set(curconn, CONN_RESP);
					FD_MSET(curconn->sd, &sdsetwr);
					goto socketsend;
				}

                		gl_str2acsll(tty.txbuf,tty.txlen,tty.writebuf);

				//tty write
				if (FD_ISSET(tty.fd, &sdsetwr)){

					tcflush(tty.fd, TCIOFLUSH);
					rc = tty_write(tty.fd, tty.writebuf ,tty.txlen/2);

					if (rc < 0){
						logw(3,"tty write error %d\n",rc);
						strncpy(curconn->buf,"tty write error",15);
						tty.rxlen = 15;
						state_conn_set(curconn, CONN_RESP);
						FD_MSET(curconn->sd, &sdsetwr);
						goto socketsend;
					}
					else{
						FD_MSET(tty.fd, &sdsetrd);
						logw(2, "tty: write %d bytes", rc);
						//tty read
						if (FD_ISSET(tty.fd, &sdsetrd)){
							rc = tty_read(tty.fd,tty.rxbuf,tty.rxlen);
							tty.rxlen = rc;
							logw(2, "tty:read bytes %d",tty.rxlen);
							if (rc <= 0){ 
								logw(0,"no data %d\n",rc);
								strncpy(curconn->buf,"no data",7);
								tty.rxlen = 7;
								state_conn_set(curconn, CONN_RESP);
							}
							else{
								gl_hex2str(tty.rxbuf,tty.rxlen,curconn->buf);
								tty.rxlen *=2 ;
								state_conn_set(curconn, CONN_RESP);
							}
							logw(2, "tty: read len %d \n",tty.rxlen);
					    }
					}
				}//end tty write and read
			}
		}
		break;
	case CONN_RESP:
socketsend:
		if (FD_ISSET(curconn->sd, &sdsetwr)){
			rc = conn_write(curconn->sd,curconn->buf ,tty.rxlen, 0);
			logw(2,"socket write len %d \n",rc);
			if (rc <= 0){
				// error - drop this connection and go to next queue element 
				curconn = conn_close(curconn);
				break;
			}
			state_conn_set(curconn, CONN_HEADER);
		}
		break;
      } // switch (curconn->state) 
      curconn = queue_next_elem(&queue, curconn);
    } // while (len--)
  } /* while (TRUE) */
  /* XXX some cleanup must be here */
}


