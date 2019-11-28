#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>

#include "network.h"

#define MIN_SCNP_PACKET_LENGTH 1
#define MAX_SCNP_PACKET_LENGTH 7

static const uint8_t broadcast_ll_addr[6] = {0xff,0xff,0xff,0xff,0xff,0xff};

void create_scnp_socket(struct scnp_socket * sock, int if_index)
{
  /* create socket */
  int fd = socket(AF_PACKET, SOCK_DGRAM, ETH_P_SCNP);
  if (fd == -1) {
    perror("Cannot create socket");
    exit(EXIT_FAILURE);
  }

  sock->packet_socket = fd;
  sock->if_index = if_index;
}

void close_scnp_socket(struct scnp_socket * sock)
{
  close(sock->packet_socket);
}

void scnp_request_to_buffer(uint8_t * buffer, struct scnp_req * request)
{
  buffer[0] = request->type;
  uint16_t net_order_short = htons(request->code);
  memcpy(buffer+1, &net_order_short, sizeof(uint16_t));
  uint32_t net_order_long;
  switch (buffer[0]) {
    case EV_KEY:
      buffer [3] = request->key_flags;
      break;
    case EV_REL:
    case EV_ABS:
      net_order_long = htonl(request->value);
      memcpy(buffer+3, &net_order_long, sizeof(uint32_t));
      break;
    default:
      perror("Unknown SCNP type");
  }
}

void buffer_to_scnp_request(uint8_t * buffer, struct scnp_req * request)
{
  request->type = buffer[0];
  memcpy(&request->code, buffer+1, sizeof(uint16_t));
  request->code = ntohs(request->code);
  switch (buffer[0]) {
    case EV_KEY:
      request->key_flags = buffer[3];
      break;
    case EV_REL:
    case EV_ABS:
      memcpy(&request->value, buffer+3, sizeof(uint32_t));
      request->value = ntohl(request->value);
      break;
    default:
      perror("Unknown SCNP type");
  }
}

void send_scnp_request(struct scnp_socket * sock, const uint8_t * dest_addr, struct scnp_req *request)
{
  /* init sock_addr */
  struct sockaddr_ll sock_addr;
  sock_addr.sll_family = AF_PACKET;
  sock_addr.sll_ifindex = sock->if_index;
  sock_addr.sll_halen = ETHER_ADDR_LEN;
  sock_addr.sll_protocol = ETH_P_SCNP;
  memcpy(sock_addr.sll_addr, dest_addr, ETHER_ADDR_LEN);

  /* init request buffer */
  size_t buffer_size = sizeof(request->type) + sizeof(request->code);
  switch (request->type) {
    case EV_KEY:
      buffer_size += sizeof(request->key_flags);
      break;
    case EV_REL:
    case EV_ABS:
      buffer_size += sizeof(request->value);
      break;
    default:
      perror("Unknown SCNP type");
  }
  uint8_t * buffer = (uint8_t *) malloc(buffer_size);
  scnp_request_to_buffer(buffer, request);

  /* send request */
  sendto(sock->packet_socket, buffer, sizeof(buffer), 0, (struct sockaddr *) &sock_addr, sizeof(sock_addr));
  free(buffer);
}

void recv_scnp_request(struct scnp_socket * sock, struct scnp_req * request)
{
  /* init sock_addr */
  struct sockaddr_ll sock_addr;
  sock_addr.sll_protocol = ETH_P_SCNP;
  sock_addr.sll_ifindex = sock->if_index;

  /* init buffer */
  uint8_t buffer[MAX_SCNP_PACKET_LENGTH];
  bzero(buffer, MAX_SCNP_PACKET_LENGTH);

  /* receive request */
  socklen_t sock_len = sizeof(sock_addr);
  int rec = recvfrom(sock->packet_socket, buffer, sizeof(buffer), 0, (struct sockaddr *) &sock_addr, &sock_len);
  if (rec < MIN_SCNP_PACKET_LENGTH) {
    perror("Cannot create SCNP request: not enough data received.");
  }
  else {
    buffer_to_scnp_request(buffer, request);
  }
}

void create_scnp_request(struct scnp_req * request, uint8_t type, uint16_t code, uint8_t key_flags, uint32_t value)
{
  request->type = type;
  request->code = code;
  request->key_flags = key_flags;
  request->value = value;
}

void scnp_send_key(struct scnp_socket * sock, const uint8_t * dest_addr, uint16_t code, int pressed)
{
  struct scnp_req request;
  uint8_t key_flags = (pressed != 0) << 7;
  create_scnp_request(&request, EV_KEY, code, key_flags, 0);
  send_scnp_request(sock, dest_addr, &request);
}
