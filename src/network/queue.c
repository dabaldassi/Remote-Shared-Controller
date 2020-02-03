#include <stdlib.h>
#include <errno.h>
#include <values.h>
#include <string.h>
#include <time.h>


#include "queue.h"


struct scnp_queue * init_queue(void)
{
  struct scnp_queue * q = malloc(sizeof(struct scnp_queue));

  if (q != NULL) {
    /* initialize queue fields */
    pthread_mutex_init(&q->__mutex, NULL);
    sem_init(&q->__sem, 0, 0);
    q->head = NULL;
    q->tail = NULL;
  }

  return q;
}

void free_queue(struct scnp_queue * queue)
{
  if (queue != NULL) {
    /* remove and free elements in the queue */
    queue_elt * e = queue->head;
    while (e != NULL) {
      queue_elt * x = e;
      e = e->next;
      free(x->packet);
      free(x);
    }

    /* destroy and free the queue */
    pthread_mutex_destroy(&queue->__mutex);
    sem_destroy(&queue->__sem);
    free(queue);
  }
}

int push(struct scnp_queue * queue, const struct scnp_packet * packet, const uint8_t * addr)
{
  if (queue == NULL) {
    errno = EINVAL;
    return -1;
  }

  /* verify the size of the queue */
  int sval;
  sem_getvalue(&queue->__sem, &sval);
  if (sval >= SEM_VALUE_MAX) {
    errno = EXFULL;
    return -1;
  }

  queue_elt * e = malloc(sizeof(queue_elt));
  if (e == NULL) return -1;

  /* initialize a new element fields */
  e->packet = malloc(sizeof(struct scnp_packet));
  if (e->packet == NULL) return -1;
  memcpy(e->packet, packet, sizeof(struct scnp_packet));
  memcpy(e->addr, addr, ETHER_ADDR_LEN);
  e->next = NULL;

  pthread_mutex_lock(&queue->__mutex);

  /* insert the new element */
  e->prev = queue->tail;
  if (queue->tail != NULL) queue->tail->next = e;
  else queue->head = e;
  queue->tail = e;

  sem_post(&queue->__sem);

  pthread_mutex_unlock(&queue->__mutex);

  return 0;
}

int pull(struct scnp_queue * queue, struct scnp_packet * packet, uint8_t * addr, long long int tout_nsec)
{
  if (queue == NULL) {
    errno = EINVAL;
    return -1;
  }

  /* verify the size of the queue */
  if (tout_nsec < 0) {
    if (sem_wait(&queue->__sem)) return -1;
  }
  else {
    struct timespec timeout;
    timeout.tv_sec = time(NULL) + tout_nsec / 1000000000;
    timeout.tv_nsec = tout_nsec % 1000000000;
    if (sem_timedwait(&queue->__sem, &timeout)) return -1;
  }

  pthread_mutex_lock(&queue->__mutex);

  /* copy data of the first element */
  memcpy(packet, queue->head->packet, sizeof(struct scnp_packet));
  memcpy(addr, queue->head->addr, ETHER_ADDR_LEN);

  /* remove the first element */
  free(queue->head->packet);
  if (queue->head == queue->tail) {
    free(queue->head);
    queue->head = NULL;
    queue->tail = NULL;
  }
  else {
    queue->head = queue->head->next;
    free(queue->head->prev);
    queue->head->prev = NULL;
  }

  pthread_mutex_unlock(&queue->__mutex);

  return 0;
}
