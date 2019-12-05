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
#define SCNP_ACK 254

  struct scnp_socket
  {
    int packet_socket;
    int if_index;
  };

  struct scnp_packet
  {
    uint8_t type;
    uint8_t data[MAX_SCNP_PACKET_LENGTH - 1];
  } PACKED;

  struct scnp_key
  {
    uint8_t type;
    uint16_t code;
    uint8_t flags;
  } PACKED;

  struct scnp_movement
  {
    uint8_t type;
    uint16_t code;
    uint32_t value;
  } PACKED;

  struct scnp_management
  {
    uint8_t type;
    uint8_t flags;
  } PACKED;

  struct scnp_ack
  {
    uint8_t type;
  } PACKED;

  int scnp_create_socket(struct scnp_socket * sock, int if_index);

  int scnp_close_socket(struct scnp_socket * sock);

  void scnp_packet_to_buffer(uint8_t * buffer, const struct scnp_packet * packet);

  void scnp_buffer_to_packet(const uint8_t * buffer, struct scnp_packet * packet);

  int scnp_send(struct scnp_socket * sock, const uint8_t * dest_addr, struct scnp_packet * packet, size_t packet_length);

  int scnp_recv(struct scnp_socket * sock, struct scnp_packet * packet);

#ifdef __cplusplus
}
#endif

#endif /* SCNP_H */
