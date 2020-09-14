
#ifndef _STATE_H
#define _STATE_H

#include "globals.h"
#include "conn.h"
#include "tty.h"
#include "queue.h"
#include "cfg.h"
#ifdef LOG
#  include "log.h"
#endif

/* prototypes */
conn_t *state_conn_search(queue_t *queue, conn_t *conn, int state);
void state_conn_set(conn_t *conn, int state);
void state_tty_set(ttydata_t *mod, int state);

#endif /* _STATE_H */
