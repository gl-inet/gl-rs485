#include "sock.h"
#include "cfg.h"

/*
 * Bring i/o descriptor SD to BLKMODE (if non-zero - socket is nonblocking)
 *
 * Return: RC_ERR if there some errors
 */
int
sock_set_blkmode(int sd, int blkmode)
{
    int flags;

    flags = fcntl(sd, F_GETFL);
    if (flags == -1) return -1;

    flags = blkmode ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
    flags = fcntl(sd, F_SETFL, flags);

    return flags;
}

/*
 * Create new socket in BLKMODE mode (if non-zero - socket is nonblocking)
 *
 * Return: socket descriptor, otherwise RC_ERR if there some errors
 */
int
sock_create(int blkmode, sa_family_t sa_family)
{
    int sock=0;
    if(strncmp(cfg.connmode,"udp",3)) {
        sock = socket(sa_family, SOCK_STREAM, IPPROTO_TCP);
    } else {
        sock = socket(sa_family, SOCK_DGRAM, IPPROTO_UDP);
    }
    if (sock == -1) {
        logw(2, "sock_create(): unable to create socket (%s)",
             strerror(errno));
        return RC_ERR;
    }

    /* set socket to desired blocking mode */
    if(strncmp(cfg.connmode,"tcpc",4)) {
        if (sock_set_blkmode(sock, blkmode) == -1) {
            logw(2, "sock_create(): unable to set "
                 "server socket to nonblocking (%s)",
                 strerror(errno));
            return RC_ERR;
        }
    } else {
        if (sock_set_blkmode(sock, 0) == -1) {
            logw(2, "sock_create(): unable to set "
                 "server socket to nonblocking (%s)",
                 strerror(errno));
            return RC_ERR;
        }

    }


    return sock;
}

/*
 * Create new server socket with SERVER_PORT, SERVER_IP -
 * port and address to bind the socket,
 * BLKMODE - blocking mode (if non-zero - socket is nonblocking)
 *
 * Return: socket descriptor, otherwise RC_ERR if there some errors
 */
int
sock_create_server(char *server_ip, unsigned short server_port, int blkmode)
{
    struct sockaddr_storage server_sockaddr;
    int sock_opt = 1;
    int server_s;

    memset(&server_sockaddr, 0, sizeof(server_sockaddr));

    /* parse address to bind socket */
    if (server_ip != NULL) {
        /* try first to parse server_ip as IPv6 address, if it fail, try to parse as IPv4 */
        if (inet_pton(AF_INET6, server_ip,
                      &(*((struct sockaddr_in6 *)&server_sockaddr)).sin6_addr) != 0)
            server_sockaddr.ss_family = AF_INET6;
        else if (inet_pton(AF_INET, server_ip,
                           &(*((struct sockaddr_in *)&server_sockaddr)).sin_addr) != 0)
            server_sockaddr.ss_family = AF_INET;
        else {
            logw(2, "sock_create_server():"
                 " can't parse address: %s",
                 server_ip);
            return RC_ERR;
        }
    } else {
        server_sockaddr.ss_family = AF_INET; // TODO AF_INET6/in6addr_any?
        (*((struct sockaddr_in *)&server_sockaddr)).sin_addr.s_addr = htonl(INADDR_ANY);
    }

    /* parse IP port */
    switch (server_sockaddr.ss_family) {
    case AF_INET:
        (*((struct sockaddr_in *)&server_sockaddr)).sin_port = htons(server_port);
        break;
    case AF_INET6:
        (*((struct sockaddr_in6 *)&server_sockaddr)).sin6_port = htons(server_port);
        break;
    }

    /* create socket in desired blocking mode */
    server_s = sock_create(blkmode, server_sockaddr.ss_family);
    if (server_s < 0) return server_s;

    /* set to close socket on exec() */
    if (fcntl(server_s, F_SETFD, 1) == -1) {
        logw(2, "sock_create_server():"
             " can't set close-on-exec on socket (%s)",
             strerror(errno));
        return RC_ERR;
    }
    /* set reuse socket address */
    if (setsockopt(server_s, SOL_SOCKET,
                   SO_REUSEADDR, (void *)&sock_opt,
                   sizeof(sock_opt)) == -1) {
        logw(2, "sock_create_server():"
             " can't set socket to SO_REUSEADDR (%s)",
             strerror(errno));
        return RC_ERR;
    }
    /* adjust socket rx and tx buffer sizes */
    sock_opt = SOCKBUFSIZE;
    if ((setsockopt(server_s, SOL_SOCKET,
                    SO_SNDBUF, (void *)&sock_opt,
                    sizeof(sock_opt)) == -1) ||
        (setsockopt(server_s, SOL_SOCKET,
                    SO_RCVBUF, (void *)&sock_opt,
                    sizeof(sock_opt)) == -1)) {
        logw(2, "sock_create_server():"
             " can't set socket TRX buffers sizes (%s)",
             strerror(errno));
        return RC_ERR;
    }

    /* bind socket to given address and port */
    if((!strncmp(cfg.connmode,"udp",3))||(!(strncmp(cfg.connmode,"tcps",4)))) {

        if (bind(server_s, (struct sockaddr *)&server_sockaddr,
                 sa_len((struct sockaddr *)&server_sockaddr)) == -1) {
            logw(2, "sock_create_server():"
                 " unable to bind() socket (%s)",
                 strerror(errno));
            return RC_ERR;
        }
    } else if(!strncmp(cfg.connmode,"tcpc",4)) {

        if (connect(server_s, (struct sockaddr *)&server_sockaddr,
                    sa_len((struct sockaddr *)&server_sockaddr)) == -1) {
            logw(2, "sock_create_server():"
                 " unable to connect socket (%s)",
                 strerror(errno));
            return RC_ERR;
        }
    }

    if (sock_set_blkmode(server_s, 1) == -1) {
        logw(2, "unblk unable to set "
             " (%s)",
             strerror(errno));
        return RC_ERR;
    }


    if(!strncmp(cfg.connmode,"tcps",4)) {
        /* let's listen */
        if (listen(server_s, BACKLOG) == -1) {
            logw(3, "sock_create_server():"
                 " unable to listen() on socket (%s)",
                 strerror(errno));
            exit(errno);
        }

    }
    return server_s;
}

/*
 * Accept connection from SERVER_SD - server socket descriptor
 * and create socket in BLKMODE blocking mode
 * (if non-zero - socket is nonblocking)
 *
 * Return: socket descriptor, otherwise RC_ERR if there some errors;
 *         RMT_ADDR - ptr to connection info structure
 */
int
sock_accept(int server_sd, struct sockaddr *rmt_addr, socklen_t rmt_len, int blkmode)
{
    int sd, sock_opt = SOCKBUFSIZE;

    if(!strncmp(cfg.connmode,"tcps",4)) {
        sd = accept(server_sd, rmt_addr, &rmt_len);
    } else {
        sd = server_sd ;
    }
    if (sd == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            /* some errors caused */
            logw(4, "sock_accept(): error in accept() (%s)", strerror(errno));
        return RC_ERR;
    }
    /* tune socket */
    if (sock_set_blkmode(sd, blkmode) == RC_ERR) {
        logw(3, "sock_accept(): can't set socket blocking mode (%s)",
             strerror(errno));
        close(sd);
        return RC_ERR;
    }
    /* adjust socket rx and tx buffer sizes */
    if ((setsockopt(sd, SOL_SOCKET,
                    SO_SNDBUF, (void *)&sock_opt,
                    sizeof(sock_opt)) == -1) ||
        (setsockopt(sd, SOL_SOCKET,
                    SO_RCVBUF, (void *)&sock_opt,
                    sizeof(sock_opt)) == -1)) {
        logw(3, "sock_accept():"
             " can't set socket TRX buffer sizes (%s)",
             strerror(errno));
        return RC_ERR;
    }
    return sd;
}

/*
 * Return reference to socket address structure according to its family (AF_INET/AF_INET6)
 */
void *
sock_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

