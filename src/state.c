
#include "state.h"

/*
 * Search for first client connection in state STATE
 * Parameters: CONN - ptr to queue element
 *             (if NULL - search from queue begin),
 *             QUEUE - ptr to the queue;
 * Return:     pointer to queue element
 *             or NULL if none found
 */
conn_t *
state_conn_search(queue_t *queue, conn_t *conn, int state)
{
  int len = queue->len;

  /* check for zero queue length */
  if (!queue->len) return NULL;

  if (conn == NULL)
    conn = queue->beg;
  else
    conn = queue_next_elem(queue, conn);

  while (len--)
  {
    if (conn->state == state)
      return conn;
    conn = queue_next_elem(queue, conn);
  }

  return NULL; /* none found */
}

/*
 * Set connection CONN to STATE
 */
void
state_conn_set(conn_t *conn, int state)
{
  conn->state = state;
  /* reset timeout value */
  conn->timeout = cfg.conntimeout;
}

