#ifndef QUEUE_H
#define QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __gnu_linux__
#include <net/ethernet.h>
#include <pthread.h>
#include <semaphore.h>
#else
#define ETHER_ADDR_LEN 6
#include <pthread.h>
#include <semaphore.h>
#endif

#include "scnp.h"


/**
 * @struct queue_elt
 * @brief Element of an struct scnp_queue.
 *
 * Structure used to store a SCNP packet and an ethernet address (source or
 * destination) in a queue.
 *
 * @var packet A SCNP packet that needs to be stored in a queue.
 * @var addr An address associated with the SCNP packet in the structure.
 * @var prev Pointer to the previous element in the queue.
 * @var next Pointer to the next element in the queue.
 */

typedef struct queued_packet
{
  struct scnp_packet *packet;
  uint8_t addr[ETHER_ADDR_LEN];
  struct queued_packet *prev;
  struct queued_packet *next;
} queue_elt;

/**
 * @struct struct scnp_queue
 * @brief Queue structure that stores queue_elt.
 *
 * Structure of queue used to store SCNP packet and an ethernet address.
 *
 * @var head Pointer to the first element in the queue. It is the element that
 * is going to be pulled.
 * @var tail Pointer to the last element in the queue. It is the last element
 * that has been pushed.
 */

struct scnp_queue
{
  pthread_mutex_t __mutex;
  sem_t __sem;
  queue_elt *head;
  queue_elt *tail;
};

/**
 * @fn struct scnp_queue * init_queue(void)
 * @brief Constructor of struct scnp_queue.
 *
 * Function that allocate memory for a pointer to a struct scnp_queue
 * and initialize its fields.
 *
 * @return On success returns an initialized pointer to a struct scnp_queue.
 * On error, returns NULL and errno is set appropriately
 * @section Errors
 * ENOMEM Out of memory.
 */

struct scnp_queue *init_queue(void);

/**
 * @fn int free_queue(struct scnp_queue * queue)
 * @brief Destructor of struct scnp_queue.
 *
 * Function that frees memory allocated to a struct scnp_queue
 * and all queue_elt it contains. Free a struct scnp_queue while modifying it
 * will result to an undefined behavior.
 *
 * @param queue Pointer to the struct scnp_queue to destroy.
 */

void free_queue(struct scnp_queue *queue);

/**
 * @fn int push(struct scnp_queue * queue, const struct scnp_packet * packet, const uint8_t * addr)
 * @brief Add an element at the end of a struct scnp_queue.
 *
 * Function that adds a queue_elt (filled with packet and address in parameters)
 * in a struct scnp_queue. If the queue is full, the element will not be added.
 *
 * @param queue Pointer to the struct scnp_queue where the new element will
 * be inserted.
 * @param packet SCNP data of the new element.
 * @param addr Ethernet address of the new element.
 * @return On success, returns 0.
 * On error, returns -1 and errno is set appropriately.
 * @section Errors
 * EINVAL Invalid queue pointer. Maybe the queue was not initialized or was freed.
 * ENOMEM Out of memory.
 * EXFULL The queue is full. Pull an element before push a new one.
 */

int push(struct scnp_queue *queue, const struct scnp_packet *packet, const uint8_t *addr);

/**
 * @fn int pull(struct scnp_queue * queue, struct scnp_packet * packet, uint8_t * addr, long long int tout_nsec);
 * @brief Remove the first element of a struct scnp_queue.
 *
 * Function that removes the first element of a struct scnp_queue and
 * copy its data into the packet and the address in parameters.
 * If the queue is empty, this function will block the execution
 * for tout_nsec nanoseconds. If tout_nsec is negative, the execution will be
 * blocked until an element is inserted.
 *
 * @param queue Pointer to the struct scnp_queue where the element will be
 * removed.
 * @param packet SCNP data of the removed element.
 * @param addr Ethernet address of the element removed.
 * @param tout_nsec Number of nanoseconds the function should block until
 * the queue is no more empty. If negative, the function will block until
 * an element is inserted.
 * @return On success, returns 0, packet and addr are filled with the data in
 * the removed element. On error, returns -1 and errno is set appropriately.
 * @section Errors
 * EINVAL Invalid queue pointer. Maybe the queue was not initialized or was freed.
 * ETIMEDOUT The call timed out before a new element was pushed.
 */

int pull(struct scnp_queue *queue, struct scnp_packet *packet, uint8_t *addr, long long int tout_nsec);

#ifdef __cplusplus
}
#endif

#endif /* QUEUE_H */
