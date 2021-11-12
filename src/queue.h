#ifndef _QUEUE_H
#define _QUEUE_H

#include "globals.h"
#include "conn.h"
#include "log.h"

/*
 * Queue parameters structure
 */
typedef struct {
    conn_t *beg; /* address of first queue element */
    conn_t *end; /* address of last queue element */
    int len;     /* number of elements in the queue */
} queue_t;

/* prototypes */
void queue_init(queue_t *queue);
conn_t *queue_new_elem(queue_t *queue);
void queue_delete_elem(queue_t *queue, conn_t *conn);
conn_t *queue_next_elem(queue_t *queue, conn_t *conn);

#endif /* _QUEUE_H */

