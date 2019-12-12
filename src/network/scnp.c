#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>

#include "scnp.h"

#define ACK_TIMEOUT_S 0
#define ACK_TIMEOUT_US 1000
#define ACK_MAX_TRIES 3

static const uint8_t broadcast_ll_addr[] = {0xff,0xff,0xff,0xff,0xff,0xff};

int scnp_create_socket(struct scnp_socket * sock, int if_index)
{
  /* create socket */
  int fd = socket(AF_PACKET, SOCK_DGRAM, ETH_P_SCNP);
  if (fd == -1) return -1;

  sock->packet_socket = fd;
  sock->if_index = if_index;
  return 0;
}

int scnp_close_socket(struct scnp_socket * sock)
{
  return close(sock->packet_socket);
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
    case SCNP_ACK:
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
    case SCNP_ACK:
      memset(packet->data, 0, MAX_SCNP_PACKET_LENGTH - 1);
      break;
    default:
      memcpy(packet->data, buffer+1, MAX_SCNP_PACKET_LENGTH - 1);
  }
}

int scnp_recv_ackless(struct scnp_socket * sock, struct scnp_packet * packet, uint8_t * src_addr)
{
  /* init sock_addr */
  struct sockaddr_ll sock_addr;
  sock_addr.sll_protocol = ETH_P_SCNP;
  sock_addr.sll_ifindex = sock->if_index;

  /* init buffer */
  uint8_t buffer[MAX_SCNP_PACKET_LENGTH];
  memset(buffer, 0, MAX_SCNP_PACKET_LENGTH);

  /* receive packet */
  socklen_t sock_len = sizeof(sock_addr);
  int bytes_received = recvfrom(sock->packet_socket, buffer, sizeof(buffer), 0, (struct sockaddr *) &sock_addr, &sock_len);
  if (bytes_received == -1) return -1;
  memcpy(src_addr, sock_addr.sll_addr, ETHER_ADDR_LEN);
  scnp_buffer_to_packet(buffer, packet);

  return bytes_received;
}

int scnp_send(struct scnp_socket * sock, const uint8_t * dest_addr, struct scnp_packet * packet, size_t packet_length)
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
  int bytes_sent = -1;
  if (packet->type != SCNP_ACK && packet->type != EV_REL && packet->type != EV_ABS) {
    /* create socket to receive aknowledgment */
    struct scnp_socket ack_sock;
    if (scnp_create_socket(&ack_sock, sock->if_index) == -1) return -1;

    /* set acknowledgment timeout */
    struct timeval ack_timeout;
    ack_timeout.tv_sec = ACK_TIMEOUT_S;
    ack_timeout.tv_usec = ACK_TIMEOUT_US;
    if (setsockopt(ack_sock.packet_socket, SOL_SOCKET, SO_RCVTIMEO, &ack_timeout, sizeof(ack_timeout)) == -1) return -1;

    /* send packet and wait for acknowledgment */
    struct scnp_packet ack;
    ack.type = 0;
    memset(ack.data, 0, MAX_SCNP_PACKET_LENGTH - 1);
    uint8_t ack_src[ETHER_ADDR_LEN];
    int i = 0;
    while (i++ < ACK_MAX_TRIES && (ack.type != SCNP_ACK || memcmp(ack_src, dest_addr, ETHER_ADDR_LEN) != 0)) {
      bytes_sent = sendto(sock->packet_socket, buffer, packet_length, 0, (struct sockaddr *) &sock_addr, sizeof(sock_addr));
      int bytes_received = 0;
      while (bytes_received != -1 && (ack.type != SCNP_ACK || memcmp(ack_src, dest_addr, ETHER_ADDR_LEN) != 0)) {
        bytes_received = scnp_recv_ackless(&ack_sock, &ack, ack_src);
      }
    }
    if (scnp_close_socket(&ack_sock) == -1 || ack.type != SCNP_ACK || memcmp(ack_src, dest_addr, ETHER_ADDR_LEN) != 0) bytes_sent = -1;
  }
  else {
    bytes_sent = sendto(sock->packet_socket, buffer, packet_length, 0, (struct sockaddr *) &sock_addr, sizeof(sock_addr));
  }
  free(buffer);

  if (bytes_sent == -1) return -1;
  return bytes_sent;
}

int scnp_recv(struct scnp_socket * sock, struct scnp_packet * packet)
{
  /* init sock_addr */
  struct sockaddr_ll sock_addr;
  sock_addr.sll_protocol = ETH_P_SCNP;
  sock_addr.sll_ifindex = sock->if_index;

  /* init buffer */
  uint8_t buffer[MAX_SCNP_PACKET_LENGTH];
  memset(buffer, 0, MAX_SCNP_PACKET_LENGTH);

  /* receive packet */
  socklen_t sock_len = sizeof(sock_addr);
  int bytes_received = recvfrom(sock->packet_socket, buffer, sizeof(buffer), 0, (struct sockaddr *) &sock_addr, &sock_len);
  if (bytes_received == -1) return -1;
  scnp_buffer_to_packet(buffer, packet);

  /* send ack */
  if (packet->type != SCNP_ACK && packet->type != EV_REL && packet->type != EV_ABS) {
    struct scnp_ack ack;
    ack.type = SCNP_ACK;
    scnp_send(sock, sock_addr.sll_addr, (struct scnp_packet *) &ack, sizeof(ack));
  }

  return bytes_received;
}
