#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>

#include "network.h"

static const uint8_t broadcast_ll_addr[6] = {0xff,0xff,0xff,0xff,0xff,0xff};

void scnp_create_socket(struct scnp_socket * sock, int if_index)
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

void scnp_close_socket(struct scnp_socket * sock)
{
  if (close(sock->packet_socket) != 0) {
    perror("Cannot close socket");
    exit(EXIT_FAILURE);
  }
}

void scnp_packet_to_buffer(uint8_t * buffer, const struct scnp_packet * packet)
{
  buffer[0] = packet->type;
  uint16_t net_order_short;
  uint32_t net_order_long;
  switch (buffer[0]) {
    case EV_KEY:
      memcpy(&net_order_short, packet->data, sizeof(uint16_t));
      net_order_short = htons(net_order_short);
      memcpy(buffer+1, &net_order_short, sizeof(uint16_t));
      buffer[3] = packet->data[2];
      break;
    case EV_REL:
    case EV_ABS:
      memcpy(&net_order_short, packet->data, sizeof(uint16_t));
      net_order_short = htons(net_order_short);
      memcpy(buffer+1, &net_order_short, sizeof(uint16_t));
      memcpy(&net_order_long, packet->data+2, sizeof(uint32_t));
      net_order_long = htonl(net_order_long);
      memcpy(buffer+3, &net_order_long, sizeof(uint32_t));
      break;
    default:
      memcpy(buffer+1, packet->data, sizeof(packet->data));
  }
}

void scnp_buffer_to_packet(const uint8_t * buffer, struct scnp_packet * packet)
{
  packet->type = buffer[0];
  uint16_t host_order_short;
  uint32_t host_order_long;
  switch (buffer[0]) {
    case EV_KEY:
      memcpy(&host_order_short, buffer+1, sizeof(uint16_t));
      host_order_short = ntohs(host_order_short);
      memcpy(packet->data, &host_order_short, sizeof(uint16_t));
      packet->data[2] = buffer[3];
      break;
    case EV_REL:
    case EV_ABS:
      memcpy(&host_order_short, buffer+1, sizeof(uint16_t));
      host_order_short = ntohs(host_order_short);
      memcpy(packet->data, &host_order_short, sizeof(uint16_t));
      memcpy(&host_order_long, buffer+3, sizeof(uint32_t));
      host_order_long = ntohl(host_order_long);
      memcpy(packet->data+2, &host_order_long, sizeof(uint32_t));
      break;
    default:
      memcpy(packet->data, buffer+1, MAX_SCNP_PACKET_LENGTH - 1);
  }
}

void scnp_send(struct scnp_socket * sock, const uint8_t * dest_addr, struct scnp_packet * packet, size_t packet_length)
{
  /* init sock_addr */
  struct sockaddr_ll sock_addr;
  sock_addr.sll_family = AF_PACKET;
  sock_addr.sll_ifindex = sock->if_index;
  sock_addr.sll_halen = ETHER_ADDR_LEN;
  sock_addr.sll_protocol = ETH_P_SCNP;
  memcpy(sock_addr.sll_addr, dest_addr, ETHER_ADDR_LEN);

  /* init packet buffer */
  uint8_t * buffer = (uint8_t *) malloc(packet_length);
  scnp_packet_to_buffer(buffer, packet);

  /* send packet */
  sendto(sock->packet_socket, buffer, packet_length, 0, (struct sockaddr *) &sock_addr, sizeof(sock_addr));
  free(buffer);
}

void scnp_recv(struct scnp_socket * sock, struct scnp_packet * packet)
{
  /* init sock_addr */
  struct sockaddr_ll sock_addr;
  sock_addr.sll_protocol = ETH_P_SCNP;
  sock_addr.sll_ifindex = sock->if_index;

  /* init buffer */
  uint8_t buffer[MAX_SCNP_PACKET_LENGTH];
  bzero(buffer, MAX_SCNP_PACKET_LENGTH);

  /* receive packet */
  socklen_t sock_len = sizeof(sock_addr);
  if  (recvfrom(sock->packet_socket, buffer, sizeof(buffer), 0, (struct sockaddr *) &sock_addr, &sock_len) == -1) {
    perror("Cannot receive scnp packet");
  }
  else {
    scnp_buffer_to_packet(buffer, packet);
  }
}
