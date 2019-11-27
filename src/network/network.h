#ifndef NETWORK_H
#define NETWORK_H

#ifdef __cplusplus
extern "C" {
#endif
  
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>

#define ETH_P_SCNP 0x8888
#define SCNP_PACKET_LEN 7

  struct scnp_socket
  {
    int packet_socket;
    int if_index;
  };

  struct scnp_req
  {
    uint8_t type;
    uint32_t value;
    uint16_t code;
  };

  void create_scnp_socket(struct scnp_socket * sock, int if_index);

  void scnp_request_to_buffer(uint8_t buffer[SCNP_PACKET_LEN], struct scnp_req * request);

  void buffer_to_scnp_request(uint8_t buffer[SCNP_PACKET_LEN], struct scnp_req * request);

  void send_scnp_request(struct scnp_socket * sock, const uint8_t dest_addr[ETHER_ADDR_LEN], struct scnp_req *request);

  void recv_scnp_request(struct scnp_socket * sock, struct scnp_req * request);

  void create_scnp_request(struct scnp_req * request, uint8_t type, uint32_t value, uint16_t code);

  void scnp_send_key(struct scnp_socket * sock, const uint8_t dest_addr[ETHER_ADDR_LEN], uint8_t type, uint32_t value, uint16_t code);

#ifdef __cplusplus
}
#endif
  
#endif /* NETWORK_H */
