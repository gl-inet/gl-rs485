
#ifndef _STATE_H
#define _STATE_H

#include "globals.h"
#include "conn.h"
#include "queue.h"
#include "cfg.h"
#include "log.h"

/* prototypes */
conn_t *state_conn_search(queue_t *queue, conn_t *conn, int state);
void state_conn_set(conn_t *conn, int state);

#endif /* _STATE_H */
