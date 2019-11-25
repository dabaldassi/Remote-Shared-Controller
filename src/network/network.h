#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>

#define ETH_P_SCNP 0x8888
#define SCNP_PACKET_LEN 2

const uint8_t broadcast_ll_addr[6];

struct scnp_req {
  uint8_t input;
  uint8_t flags;
};

int create_scnp_socket(int if_index);

void init_request_buffer(uint8_t * buffer, struct scnp_req * request);

void send_scnp_request(int scnp_socket, int if_index, const uint8_t dest_addr[6], struct scnp_req * request);

int recv_scnp_request(int scnp_socket, int if_index, struct scnp_req * request);

void scnp_string(struct scnp_req * request);

void create_scnp_init_request(struct scnp_req * request);

void scnp_init(int scnp_socket, int if_index);

void create_scnp_input_request(struct scnp_req * request, uint8_t input, int pressed);

void scnp_input(int scnp_socket, int if_index, const uint8_t dest_addr[6], uint8_t input, int pressed);

#endif /* NETWORK_H */
