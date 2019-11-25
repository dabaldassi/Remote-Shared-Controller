#include "network.h"

static const uint8_t broadcast_ll_addr[6] = {0xff,0xff,0xff,0xff,0xff,0xff};

int create_scnp_socket(int if_index) {
  /* create socket */
  int fd = socket(AF_PACKET, SOCK_DGRAM, ETH_P_SCNP);
  if (fd == -1) {
    perror("Cannot create socket");
    exit(EXIT_FAILURE);
  }

  /* init sock_addr */
  struct sockaddr_ll sock_addr;
  sock_addr.sll_protocol = ETH_P_SCNP;
  sock_addr.sll_ifindex = if_index;

  /* bind socket */
  bind(fd, (struct sockaddr*) &sock_addr, sizeof(sock_addr));

  return fd;
}

void init_request_buffer(uint8_t * buffer, struct scnp_req * request) {
  *buffer = request->flags;
  *(buffer+1) = request->input;
}

void send_scnp_request(int scnp_socket, int if_index, const uint8_t dest_addr[6], struct scnp_req * request) {
  /* init sock_addr */
  struct sockaddr_ll sock_addr;
  sock_addr.sll_family = AF_PACKET;
  sock_addr.sll_ifindex = if_index;
  sock_addr.sll_halen = ETHER_ADDR_LEN;
  sock_addr.sll_protocol = ETH_P_SCNP;
  memcpy(sock_addr.sll_addr, dest_addr, ETHER_ADDR_LEN);

  /* init request buffer */
  uint8_t buffer[SCNP_PACKET_LEN];
  init_request_buffer(buffer, request);

  /* send request */
  sendto(scnp_socket, buffer, sizeof(buffer), 0, (struct sockaddr *) &sock_addr, sizeof(sock_addr));
}

void create_scnp_init_request(struct scnp_req *request) {
  request->flags = 0x80;
  request->input = 0x00;
}

void scnp_init(int scnp_socket, int if_index) {
  struct scnp_req request;
  create_scnp_init_request(&request);
  send_scnp_request(scnp_socket, if_index, broadcast_ll_addr, &request);
}

void create_scnp_input_request(struct scnp_req * request, uint8_t input, int pressed) {
  if (pressed) request->flags = 0x60;
  else request->flags = 0x40;
  request->input = input;
}

void scnp_input(int scnp_socket, int if_index, const uint8_t dest_addr[6], uint8_t input, int pressed) {
  struct scnp_req request;
  create_scnp_input_request(&request, input, pressed);
  send_scnp_request(scnp_socket, if_index, dest_addr, &request);
}

int recv_scnp_request(int scnp_socket, int if_index, struct scnp_req * request) {
  /* init buffer */
  uint8_t buffer[SCNP_PACKET_LEN];
  bzero(buffer, SCNP_PACKET_LEN);

  /* receive request */
  int rec = recv(scnp_socket, buffer, sizeof(buffer), 0);
  //int rec = recvfrom(scnp_socket, buffer, sizeof(buffer), 0, (struct sockaddr *) &sock_addr, sizeof(sock_addr));
  if (rec != 0) {
    printf("%d octets reçus\n", rec);
    request->flags = buffer[0];
    request->input = buffer[1];
  }

  return rec;
}

void scnp_string(struct scnp_req * request) {
  if (request->flags == 0x80) printf("initialisation request\n");
  else
    if ((request->flags >> 6) & 1) {
      printf("key request: %d", request->input);
      if ((request->flags >> 5) & 1) printf(" pressed\n");
      else printf(" released\n");
    }
}
