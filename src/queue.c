
#include "queue.h"

/*
 * Queue structure initialization
 */
void
queue_init(queue_t *queue)
{
  queue->beg = NULL;
  queue->end = NULL;
  queue->len = 0;
}

/*
 * Add new element to queue
 */
conn_t *
queue_new_elem(queue_t *queue)
{
  conn_t *newconn = (conn_t *)malloc(sizeof(conn_t));
  if (!newconn)
  { /* Aborting program execution */
    logw(2, "queue_new_elem(): out of memory for new element (%s)",
           strerror(errno));
    exit(errno);
  }
  newconn->next = NULL;
  if ((newconn->prev = queue->end) != NULL)
    queue->end->next = newconn;
  else /* we add first element */
    queue->beg = newconn;
  queue->end = newconn;
  queue->len++;
  logw(2, "queue_new_elem(): length now is %d", queue->len);
  return newconn;
}

/*
 * Remove element from queue
 */
void
queue_delete_elem(queue_t *queue, conn_t *conn)
{
  if (queue->len <= 0)
  { /* queue is empty */
    logw(2, "queue_delete_elem(): queue empty!");
    return;
  }
  if (conn->prev == NULL)
  { /* deleting first element */
    if ((queue->beg = queue->beg->next) != NULL)
      queue->beg->prev = NULL;
  }
  else 
    conn->prev->next = conn->next;
  if (conn->next == NULL)
  { /* deleting last element */
    if ((queue->end = queue->end->prev) != NULL)
      queue->end->next = NULL;
  }
  else
    conn->next->prev = conn->prev;
  queue->len--;
  free((void *)conn);
#ifdef DEBUG  
  logw(5, "queue_delete_elem(): length now is %d", queue->len);
#endif
  return;
}

/*
 * Obtain pointer to next element in the QUEUE (with wrapping)
 * Parameters: CONN - pointer to current queue element
 * Return: pointer to next queue element
 */
conn_t *
queue_next_elem(queue_t *queue, conn_t *conn)
{
  return (conn->next == NULL) ? queue->beg : conn->next;
}
