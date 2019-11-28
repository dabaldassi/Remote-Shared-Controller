#ifndef NETWORK_H
#define NETWORK_H

#ifdef __cplusplus
extern "C" {
#endif
  
#include <stdint.h>
#include <linux/input-event-codes.h>

#define ETH_P_SCNP 0x8888

  struct scnp_socket
  {
    int packet_socket;
    int if_index;
  };

  struct scnp_req
  {
    uint8_t type;
    uint16_t code;
    uint8_t key_flags;
    uint32_t value;
  };

  void create_scnp_socket(struct scnp_socket * sock, int if_index);

  void close_scnp_socket(struct scnp_socket * sock);

  void scnp_request_to_buffer(uint8_t * buffer, struct scnp_req * request);

  void buffer_to_scnp_request(uint8_t * buffer, struct scnp_req * request);

  void send_scnp_request(struct scnp_socket * sock, const uint8_t * dest_addr, struct scnp_req *request);

  void recv_scnp_request(struct scnp_socket * sock, struct scnp_req * request);

  void create_scnp_request(struct scnp_req * request, uint8_t type, uint16_t code, uint8_t key_flags, uint32_t value);

  void scnp_send_key(struct scnp_socket * sock, const uint8_t * dest_addr, uint16_t code, int pressed);

#ifdef __cplusplus
}
#endif
  
#endif /* NETWORK_H */
