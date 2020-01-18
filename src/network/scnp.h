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
#define OUT_LENGTH 2
#define MNG_LENGTH 65
#define ACK_LENGTH 1

#define MAX_PACKET_LENGTH 127
#define HOSTNAME_LENGTH 64

  struct scnp_socket
  {
    int fd;             // Socket used to send or receive SCNP packets
    int if_index;       // Index of the interface used
  };

  struct scnp_packet
  {
    uint8_t type;                          // Type of the SCNP packet
    uint8_t data[MAX_PACKET_LENGTH - 1];   // Data in the SCNP packet
  };

  struct scnp_key
  {
    uint8_t type;   // SCNP_KEY
    uint16_t code;  // Code associated to the key event
    bool pressed;   // true if the key is pressed, false otherwise
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
    bool direction;       // true if the mouse is located at the right border of the screen, false if it is on the left
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
   * @brief Send a SCNP packet.
   * @param sock SCNP socket used to send the SCNP packet.
   * @param dest_addr Destination address.
   * @param packet SCNP packet to be sent.
   * @return On success, returns the number of bytes sent. The return value is negative if the packet sent led to an
   * acknowledgement in response and it was not received. The return value is positive otherwise. On error, returns 0.
   */

  ssize_t scnp_send(struct scnp_socket * sock, const struct scnp_packet * packet, const uint8_t * dest_addr);

  /**
   * @brief Receive a SCNP packet and provide the source address of the message.
   * @param sock SCNP socket used to receive the SCNP packet.
   * @param packet  SCNP packet that will be fill with SCNP data.
   * @param src_addr Source address of the SCNP packet.
   * @return On success, returns the number of bytes received. On error, returns -1.
   */

  ssize_t scnp_recv(struct scnp_socket * sock, struct scnp_packet * packet, uint8_t * src_addr);

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
