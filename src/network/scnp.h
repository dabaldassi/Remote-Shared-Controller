#ifndef SCNP_H
#define SCNP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <scnp_socket.h>

/* Types */
#define SCNP_KEY 0x01
#define SCNP_MOV 0x02
#define SCNP_OUT 0x03
#define SCNP_MNG 0xfe
#define SCNP_ACK 0xff

/* Type lengths */
#define KEY_LENGTH 8
#define MOV_LENGTH 8
#define OUT_LENGTH 8
#define MNG_LENGTH 65
#define ACK_LENGTH 5

/* Maximum length of SCNP packet */
#define MAX_PACKET_LENGTH 127

/* Hostname length in SCNP management */
#define HOSTNAME_LENGTH 64

/* SCNP movement flag */
#define MOV_REL true
#define MOV_ABS false

/* SCNP out flags */
#define OUT_EGRESS true
#define OUT_INGRESS false
#define OUT_RIGHT true
#define OUT_LEFT false


/**
 * @struct struct scnp_packet
 * @brief SCNP packet generic structure.
 *
 * Structure of a SCNP packet used for parameters of every SCNP methods.
 *
 * @var type Type of the SCNP packet.
 * @var data Data contained by the SCNP packet.
 */

struct scnp_packet
{
  uint8_t type;
  uint8_t data[MAX_PACKET_LENGTH - 1];
};

/**
 * @struct struct scnp_key
 * @brief SCNP key structure.
 *
 * Structure of a SCNP packet used to send a key event.
 *
 * @var type Type of the SCNP packet. Must be SCNP_KEY.
 * @var id Identifier of the SCNP packet. The corresponding acknowledgement
 * need the same identifier. The library set this field automatically,
 * it may be set to zero.
 * @var code Code associated with the key event.
 * @var pressed Equals to true if the key is pressed, false otherwise.
 * @var repeated Equals to true if the key is repeated, false otherwise.
 */

struct scnp_key
{
  uint8_t type;
  uint32_t id;
  uint16_t code;
  bool pressed;
  bool repeated;
};

/**
 * @struct struct scnp_movement
 * @brief SCNP movement structure.
 *
 * Structure of a SCNP packet used to send a movement event.
 *
 * @var type Type of the SCNP packet. Must be SCNP_MOV.
 * @var code Code associated with the movement event.
 * @var value Value of the movement.
 */

struct scnp_movement
{
  uint8_t type;
  bool move_type;
  uint16_t code;
  int32_t value;
};

/**
 * @struct struct scnp_out
 * @brief SCNP out structure.
 *
 * Structure of a SCNP packet used to indicate that the cursor has exited
 * or entered a screen.
 *
 * @var type Type of the SCNP packet. Must be SCNP_OUT.
 * @var id Identifier of the SCNP packet. The corresponding acknowledgement
 * need the same identifier. The library set this field automatically,
 * it may be set to zero.
 * @var direction Direction of screen change. Equals to true (OUT_EGRESS)
 * if the cursor is exiting a screen, false (OUT_INGRESS) if the mouse is
 * entering a screen.
 * @var side Side of the screen where the cursor is exiting or entering.
 * Equals to true (OUT_RIGHT) for the right side, false (OUT_LEFT) for the left
 * side.
 * @var height Height of the cursor relative to the size of the screen. Must be
 * between zero and one.
 */

struct scnp_out
{
  uint8_t type;
  uint32_t id;
  bool direction;
  bool side;
  float height;
};

/**
 * @struct struct scnp_management
 * @brief SCNP management structure.
 *
 * Structure of a SCNP packet used to indicate the existence of the device.
 *
 * @var type Type of the SCNP packet. Must be SCNP_MNG.
 * @var hostname Hostname of the device. Its length must not exceed
 * 63 characters. The character following the last one must be '\0'.
 */

struct scnp_management
{
  uint8_t type;
  char hostname[HOSTNAME_LENGTH];   // Hostname label of the device linked with the source interface
};

/**
 * @struct scnp_ack
 * @brief SCNP acknowledgement structure.
 *
 * Structure of a SCNP packet used to confirm the reception of another
 * SCNP packet.
 *
 * @var type Type of the SCNP packet. Must be SCNP_ACK.
 * @var id Identifier of the SCNP packet. Must equals to the identifier of
 * the received packet that needs acknowledgement.
 */

struct scnp_ack
{
  uint8_t type;   // SCNP_ACK
  uint32_t id;    //identifier
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
 * @param key Key used to encrypt input data that will be transferes through
 * the network. The key need at least two characters.
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

int scnp_start(unsigned int if_index, const char * key);

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

void scnp_set_key(const char * key);

#ifdef __cplusplus
}
#endif

#endif /* SCNP_H */
