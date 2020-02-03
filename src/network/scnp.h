#ifndef SCNP_H
#define SCNP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>


#define ETH_P_SCNP 0x8888

#define SCNP_KEY 0x01
#define SCNP_MOV 0x02
#define SCNP_OUT 0x03
#define SCNP_MNG 0xfe
#define SCNP_ACK 0xff

#define KEY_LENGTH 4
#define MOV_LENGTH 7
#define OUT_LENGTH 4
#define MNG_LENGTH 65
#define ACK_LENGTH 1

#define MAX_PACKET_LENGTH 127
#define HOSTNAME_LENGTH 64
#define OUT_EGRESS true
#define OUT_INGRESS false
#define OUT_RIGHT true
#define OUT_LEFT false


struct scnp_packet
{
  uint8_t type;                          // Type of the SCNP packet
  uint8_t data[MAX_PACKET_LENGTH - 1];   // Data in the SCNP packet
};

struct scnp_key
{
  uint8_t type;   // SCNP_KEY
  uint16_t code;  // Code associated to the key event
  bool pressed;   // true if the key is pressed, false otherwise (even if repeated is true)
  bool repeated;  // true if the key is repeated, false otherwise
};

struct scnp_movement
{
  uint8_t type;     // SCNP_MOV
  uint16_t code;    // Code associated to the movement event
  int32_t value;    // Value of the movement
};

struct scnp_out
{
  uint8_t type;         // SCNP_OUT
  bool direction;       // true if the mouse is going from the host to the client, false otherwise
  bool side;            // true if the mouse is located at the right border of the screen, false if it is on the left
  float height;         // Height of the cursor relative to the size of the screen
};

struct scnp_management
{
  uint8_t type;                     // SCNP_MNG
  char hostname[HOSTNAME_LENGTH];   // Hostname label of the device linked with the source interface
};

struct scnp_ack
{
  uint8_t type;   // SCNP_ACK
};

/**
 * @fn int scnp_start(int if_index)
 * @brief Start a SCNP session.
 *
 * Function that creates and runs threads required to send and receive
 * SCNP packets. Also it begins to send periodic management packets.
 * You cannot start multiple SCNP session in a same process.
 *
 * @param if_index Index of the network interface used to send
 * and receive SCNP packets.
 * @return On success, returns 0.
 * On error, returns -1 and errno is set appropriately.
 * @section Errors
 * EACCES Permission denied. User must be root.
 * EAGAIN Insufficient resources to create threads.
 * EALREADY SCNP session already started. Stop the older one before with
 * scnp_stop() or create a new process.
 * EMFILE The per-process limit on the number of open file descriptors has
 * been reached.
 * ENETDOWN Interface is not up.
 * ENFILE The system-wide limit on the total number of open files has been
 * reached.
 * ENOBUFS or ENOMEM Out of memory.
 * ENODEV Unknown interface index.
 * ENXIO Invalid interface index.
 * EPERM Permission denied, user is not the superuser.
 */

int scnp_start(unsigned int if_index);

/**
 * @fn int scnp_stop(void)
 * @brief Stop a SCNP session.
 *
 * Function that stop all threads used to perform SCNP session. It is no more
 * possible to send and receive SCNP packets after a call to this function.
 * If no SCNP session is running when this function is called, no errors will
 * be raised. This function cannot fail.
 */

void scnp_stop(void);

/**
 * @fn int scnp_send(struct scnp_packet * packet, const uint8_t * dest_addr)
 * @brief Send a SCNP packet.
 *
 * @param packet SCNP packet to be sent.
 * @param dest_addr Destination ethernet address.
 * @return On success, returns 0.
 * On error, returns -1 and errno is set appropriately.
 * @section Errors
 * ENOMEM Out of memory.
 * ESRCH No SCNP session running.
 * ETIMEDOUT Packet sent and an acknowledgement was required but none was
 * received.
 * EXFULL Packet queue is full.
 */

int scnp_send(struct scnp_packet * packet, const uint8_t * dest_addr);

/**
 * @fn int scnp_recv(struct scnp_packet * packet, uint8_t * src_addr)
 * @brief Receive a SCNP packet and provide the source address of
 * the message.
 *
 * @param packet SCNP packet that will be fill with received SCNP data.
 * @param src_addr Source address of the received SCNP packet.
 * @return On success, returns 0.
 * On error, returns -1 and errno is set appropriately.
 * @section Errors
 * ESRCH No SCNP session is running.
 */

int scnp_recv(struct scnp_packet * packet, uint8_t * src_addr);

#ifdef __cplusplus
}
#endif

#endif /* SCNP_H */
