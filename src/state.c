#include "state.h"

/*
 * Set connection CONN to STATE
 */
void state_conn_set(conn_t *conn, int state)
{
    conn->state = state;
    /* reset timeout value */
    conn->timeout = cfg.conntimeout;
}

