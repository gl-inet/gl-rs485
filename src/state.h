#ifndef _STATE_H
#define _STATE_H

#include "globals.h"
#include "conn.h"
#include "queue.h"
#include "cfg.h"
#include "log.h"

void state_conn_set(conn_t *conn, int state);

#endif /* _STATE_H */

