#ifndef _SOCKUTILS_H
#define _SOCKUTILS_H

#include "globals.h"
#include "log.h"

#define BACKLOG 5

/* Socket buffers size */
#define SOCKBUFSIZE 512

#define sa_len(sa_ptr) ((sa_ptr)->sa_family == AF_INET \
    ? sizeof (struct sockaddr_in) : sizeof (struct sockaddr_in6))

int sock_set_blkmode(int sd, int blkmode);
int sock_create(int blkmode, sa_family_t sa_family);
int sock_create_server(char *server_ip, unsigned short server_port, int blkmode);
int sock_accept(int server_sd, struct sockaddr *rmt_addr, socklen_t rmt_len, int blkmode);
void *sock_addr(struct sockaddr *sa);

#endif

