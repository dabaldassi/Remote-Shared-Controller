#ifndef SCNP_H
#define SCNP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>
#include <linux/input-event-codes.h>


#define PACKED __attribute__((packed))

#define ETH_P_SCNP 0x8888
#define MAX_SCNP_PACKET_LENGTH 7
#define SCNP_OUT 4
#define SCNP_ACK 254
#define SCNP_MNGT 255

  struct scnp_socket
  {
    int packet_socket;  // Socket used to send or receive SCNP packets
    int if_index;       // Index of the interface used
  };

  struct scnp_packet
  {
    uint8_t type;                               // Type of the SCNP packet
    uint8_t data[MAX_SCNP_PACKET_LENGTH - 1];   // Data in the SCNP packet
  } PACKED;

  struct scnp_key
  {
    uint8_t type;   // Type of the SCNP packet = EV_KEY
    uint16_t code;  // Code associated to the key event
    uint8_t flags;  // Flags of SCNP key packet
  } PACKED;

  struct scnp_movement
  {
    uint8_t type;     // Type of the SCNP packet = EV_REL || EV_ABS
    uint16_t code;    // Code associated to the movement event
    uint32_t value;   // Value of the movement
  } PACKED;

  struct scnp_out
  {
    uint8_t  type;    // Type of the SCNO packet = SCNP_OUT
    uint8_t flags;    // Flags of SCNP out of screen packet
  };

  struct scnp_management
  {
    uint8_t type;   // Type of the SCNP packet = SCNP_MNGT
    uint8_t flags;  // Flags of SCNP management packet
  } PACKED;

  struct scnp_ack
  {
    uint8_t type;   // Type of the SCNP packet = SCNP_ACK
  } PACKED;

  /**
   * @brief Create a new SCNP socket to send or receive SCNP packet.
   * @param sock New SCNP socket.
   * @param if_index Integer that identify the interface which will be used by the new socket.
   * @return On success, returns 0. On error, returns -1.
   */

  int scnp_create_socket(struct scnp_socket * sock, int if_index);

  /**
   * @brief Close a SCNP socket.
   * @param sock SCNP socket to close.
   * @return On success, returns 0. On error, returns -1.
   */

  int scnp_close_socket(struct scnp_socket * sock);

  /**
   * @brief Transform a SCNP packet to a buffer used to send SCNP data.
   * Also change bytes order to match network order.
   * @param buffer Buffer that will contain SCNP data.
   * @param packet SCNP packet that will fill the buffer.
   */

  void scnp_packet_to_buffer(uint8_t * buffer, const struct scnp_packet * packet);

  /**
   * @brief Transform a buffer which contain SCNP data to a SCNP packet.
   * Also change bytes order to match system order.
   * @param buffer Buffer that will fill the SCNP packet.
   * @param packet SCNP packet that will contain the SCNP data of the buffer.
   */

  void scnp_buffer_to_packet(const uint8_t * buffer, struct scnp_packet * packet);

  /**
   * @brief Send a SCNP packet.
   * @param sock SCNP socket used to send the SCNP packet.
   * @param dest_addr Destination address.
   * @param packet SCNP packet to be sent.
   * @param packet_length Size of the SCNP packet.
   * @return On success, returns the number of bytes sent. On error, returns -1.
   */

  int scnp_send(struct scnp_socket * sock, const uint8_t * dest_addr, struct scnp_packet * packet, size_t packet_length);

  /**
   * @deprecated Use scnp_recv_from"()" instead.
   * @brief Receive a SCNP packet.
   * @param sock SCNP socket used to receive the SCNP packet.
   * @param packet SCNP packet that will be fill with SCNP data.
   * @return On success, returns the number of bytes received. On error, returns -1.
   */

  int scnp_recv(struct scnp_socket * sock, struct scnp_packet * packet);

  /**
   * @brief Receive a SCNP packet and provide the source address of the message.
   * @param sock SCNP socket used to receive the SCNP packet.
   * @param packet  SCNP packet that will be fill with SCNP data.
   * @param src_addr Source address of the SCNP packet.
   * @return On success, returns the number of bytes received. On error, returns -1.
   */

  int scnp_recv_from(struct scnp_socket * sock, struct scnp_packet * packet, uint8_t * src_addr);

  /**
   * @brief Start a new SCNP session.
   * Start sending SCNP management packets every second to advertise the broadcast domain about the existence of an SCNP
   * session.
   * @param if_index Integer that identify the interface on which the session will be started.
   * @return On success, returns 0. On error, returns -1.
   */

  int scnp_start_session(int if_index);

  /**
   * @brief Stop a running SCNP session.
   * Stop sending SCNP management packets every second.
   * @param if_index Integer that identify the interface on which the session is running.
   * @return On success, returns 0. On error, returns -1.
   */

  int scnp_stop_session(int if_index);

#ifdef __cplusplus
}
#endif

#endif /* SCNP_H */
