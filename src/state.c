
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

/*
 * Set tty device to STATE
 */
void
state_tty_set(ttydata_t *mod, int state)
{
  switch (state)
  {
    case TTY_PAUSE:
      mod->trynum = 0;
//      mod->timer = (unsigned long)cfg.rqstpause * 1000l;
#ifdef DEBUG
      logw(5, "tty: state now is TTY_PAUSE");
#endif
      break;
    case TTY_READY:
      mod->trynum = 0;
//      mod->timer = 0l;
#ifdef DEBUG
      logw(5, "tty: state now is TTY_READY");
#endif
      break;
    case TTY_RQST:
      mod->ptrbuf = 0;
//      mod->timer = 0l;
      mod->trynum = mod->trynum ? mod->trynum - 1 : (unsigned)cfg.maxtry;
#ifdef DEBUG
      logw(5, "tty: state now is TTY_RQST");
#endif
      break;
    case TTY_RESP:
      mod->ptrbuf = 0;
      mod->rxoffset = 0;
      /* XXX need real recv length? */
      mod->rxlen = TTY_BUFSIZE;
//      mod->timer = cfg.respwait * 1000l + DV(mod->txlen, mod->bpc, mod->speed);
#ifdef DEBUG
      logw(5, "tty: state now is TTY_RESP");
#endif
      break;
    case TTY_PROC:
#ifdef DEBUG
      logw(5, "tty: state now is TTY_PROC");
#endif
      break;
    default:
      /* unknown state, exiting */
#ifdef DEBUG
      logw(5, "tty_set_state() - invalid state (%d)", state);
#endif
      exit (-1);
  }
  mod->state = state;
}
