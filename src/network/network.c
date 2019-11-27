#include "network.h"

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

static inline void memcpy_inv(uint8_t * dstpp, const void * srcpp, size_t len)
{
  uint64_t * srcp = (uint64_t *) srcpp;
  for (size_t i = 0; i < len; ++i) {
    *dstpp++ = *srcp >> (8 * (len - i - 1)) & 0xff;
  }
}

void scnp_request_to_buffer(uint8_t buffer[SCNP_PACKET_LEN], struct scnp_req * request)
{
  buffer[0] = request->type;
  memcpy_inv(buffer+1, &request->value, sizeof(uint32_t));
  memcpy_inv(buffer+5, &request->code, sizeof(uint16_t));
}

void buffer_to_scnp_request(uint8_t buffer[SCNP_PACKET_LEN], struct scnp_req * request)
{
  request->type = buffer[0];
  request->value = buffer[1] << 24 | buffer[2] << 16 | buffer[3] << 8 | buffer[4];
  request->code = buffer[5] << 8 | buffer[6];
}

void send_scnp_request(struct scnp_socket * sock, const uint8_t dest_addr[ETHER_ADDR_LEN], struct scnp_req *request)
{
  /* init sock_addr */
  struct sockaddr_ll sock_addr;
  sock_addr.sll_family = AF_PACKET;
  sock_addr.sll_ifindex = sock->if_index;
  sock_addr.sll_halen = ETHER_ADDR_LEN;
  sock_addr.sll_protocol = ETH_P_SCNP;
  memcpy(sock_addr.sll_addr, dest_addr, ETHER_ADDR_LEN);

  /* init request buffer */
  uint8_t buffer[SCNP_PACKET_LEN];
  scnp_request_to_buffer(buffer, request);

  /* send request */
  sendto(sock->packet_socket, buffer, sizeof(buffer), 0, (struct sockaddr *) &sock_addr, sizeof(sock_addr));
}

void recv_scnp_request(struct scnp_socket * sock, struct scnp_req * request)
{
  /* init sock_addr */
  struct sockaddr_ll sock_addr;
  sock_addr.sll_protocol = ETH_P_SCNP;
  sock_addr.sll_ifindex = sock->if_index;

  /* init buffer */
  uint8_t buffer[SCNP_PACKET_LEN];
  bzero(buffer, SCNP_PACKET_LEN);

  /* receive request */
  socklen_t sock_len = sizeof(sock_addr);
  int rec = recvfrom(sock->packet_socket, buffer, sizeof(buffer), 0, (struct sockaddr *) &sock_addr, &sock_len);
  if (rec != SCNP_PACKET_LEN) {
    perror("Cannot receive enough data");
  }
  else {
    buffer_to_scnp_request(buffer, request);
  }
}

void create_scnp_request(struct scnp_req * request, uint8_t type, uint32_t value, uint16_t code)
{
  request->type = type;
  request->value = value;
  request->code = code;
}

void scnp_send_key(struct scnp_socket * sock, const uint8_t dest_addr[ETHER_ADDR_LEN], uint8_t type, uint32_t value, uint16_t code)
{
  struct scnp_req request;
  create_scnp_request(&request, type, value, code);
  send_scnp_request(sock, dest_addr, &request);
}
