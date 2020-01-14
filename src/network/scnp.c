#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>

#include "scnp.h"

#define ACK_TIMEOUT_S 0
#define ACK_TIMEOUT_US 1000 // 5 us should be enough
#define ACK_MAX_TRIES 3

#define SESSION_TIMEOUT 1

static const uint8_t broadcast_ll_addr[] = {0xff,0xff,0xff,0xff,0xff,0xff};

int scnp_create_socket(struct scnp_socket * sock, int if_index)
{
  /* create socket */
  int fd = socket(AF_PACKET, SOCK_DGRAM, ETH_P_SCNP);
  if (fd == -1) return -1;

  /* provide information in socket structure */
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
  /* write packet type */
  buffer[0] = packet->type;

  uint16_t net_order_short;
  uint32_t net_order_long;
  switch (buffer[0]) {

    /* SCNP key */
    case EV_KEY:
      memcpy(&net_order_short, packet->data, sizeof(uint16_t));
      net_order_short = htons(net_order_short);
      memcpy(buffer+1, &net_order_short, sizeof(uint16_t));
      buffer[3] = packet->data[2];
      break;

    /* SCNP movement */
    case EV_REL:
    case EV_ABS:
      memcpy(&net_order_short, packet->data, sizeof(uint16_t));
      net_order_short = htons(net_order_short);
      memcpy(buffer+1, &net_order_short, sizeof(uint16_t));
      memcpy(&net_order_long, packet->data+2, sizeof(uint32_t));
      net_order_long = htonl(net_order_long);
      memcpy(buffer+3, &net_order_long, sizeof(uint32_t));
      break;

    /* SCNP out of screen */
    case SCNP_OUT:
      buffer[1] = packet->data[0];
      break;

    /* SCNP management */
    case SCNP_MNGT:
      buffer[1] = packet->data[0];
      break;

    /* SCNP acknowledgement */
    case SCNP_ACK:
      break;

    /* unknown type */
    default:
      memcpy(buffer+1, packet->data, sizeof(packet->data));
  }
}

void scnp_buffer_to_packet(const uint8_t * buffer, struct scnp_packet * packet)
{
  /* write packet type */
  packet->type = buffer[0];

  uint16_t host_order_short;
  uint32_t host_order_long;
  switch (buffer[0]) {

    /* SCNP key */
    case EV_KEY:
      memcpy(&host_order_short, buffer+1, sizeof(uint16_t));
      host_order_short = ntohs(host_order_short);
      memcpy(packet->data, &host_order_short, sizeof(uint16_t));
      packet->data[2] = buffer[3];
      break;

    /* SCNP movement */
    case EV_REL:
    case EV_ABS:
      memcpy(&host_order_short, buffer+1, sizeof(uint16_t));
      host_order_short = ntohs(host_order_short);
      memcpy(packet->data, &host_order_short, sizeof(uint16_t));
      memcpy(&host_order_long, buffer+3, sizeof(uint32_t));
      host_order_long = ntohl(host_order_long);
      memcpy(packet->data+2, &host_order_long, sizeof(uint32_t));
      break;

    /* SCNP out of screen */
    case SCNP_OUT:
      packet->data[0] = buffer[1];
      break;

    /* SCNP management */
    case SCNP_MNGT:
      packet->data[0] = buffer[1];
      break;

    /* SCNP acknowledgement */
    case SCNP_ACK:
      memset(packet->data, 0, MAX_SCNP_PACKET_LENGTH - 1);
      break;

    /* unknown type */
    default:
      memcpy(packet->data, buffer+1, MAX_SCNP_PACKET_LENGTH - 1);
  }
}

int scnp_is_ack_needed(struct scnp_packet * packet)
{
  return packet->type != SCNP_ACK && packet->type != SCNP_MNGT && packet->type != EV_REL && packet->type != EV_ABS;
}

int scnp_recv_ackless(struct scnp_socket * sock, struct scnp_packet * packet, uint8_t * src_addr)
{
  /* init sock_addr */
  struct sockaddr_ll sock_addr;
  sock_addr.sll_protocol = ETH_P_SCNP;
  sock_addr.sll_ifindex = sock->if_index;
  socklen_t sock_len = sizeof(sock_addr);

  /* init buffer */
  uint8_t buffer[MAX_SCNP_PACKET_LENGTH];
  memset(buffer, 0, MAX_SCNP_PACKET_LENGTH);

  /* receive packet */
  int bytes_received = recvfrom(sock->packet_socket, buffer, sizeof(buffer), 0, (struct sockaddr *) &sock_addr, &sock_len);
  if (bytes_received == -1) return -1;

  /* copy source address into parameter address */
  memcpy(src_addr, sock_addr.sll_addr, ETHER_ADDR_LEN);

  /* copy buffer into packet */
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

  /* init buffer */
  uint8_t * buffer = (uint8_t *) malloc(packet_length);
  if (buffer == NULL) {
    if (errno != ENOMEM) errno = ENODATA;
    return -1;
  }

  /* copy packet into buffer */
  scnp_packet_to_buffer(buffer, packet);

  /* send packet */
  int bytes_sent = 0;
  if (scnp_is_ack_needed(packet)) {
    /* create socket to receive aknowledgment */
    struct scnp_socket ack_sock;
    if (scnp_create_socket(&ack_sock, sock->if_index) == -1) bytes_sent = -1;

    /* set acknowledgment timeout */
    struct timeval ack_timeout;
    ack_timeout.tv_sec = ACK_TIMEOUT_S;
    ack_timeout.tv_usec = ACK_TIMEOUT_US;
    if (setsockopt(ack_sock.packet_socket, SOL_SOCKET, SO_RCVTIMEO, &ack_timeout, sizeof(ack_timeout)) == -1) bytes_sent = -1;

    /* send packet and wait for acknowledgment */
    struct scnp_packet ack;
    ack.type = 0;
    memset(ack.data, 0, MAX_SCNP_PACKET_LENGTH - 1);
    uint8_t ack_src[ETHER_ADDR_LEN];
    int i = 0;
    while (bytes_sent != -1 && i++ < ACK_MAX_TRIES && (ack.type != SCNP_ACK || memcmp(ack_src, dest_addr, ETHER_ADDR_LEN) != 0)) {
      bytes_sent = sendto(sock->packet_socket, buffer, packet_length, 0, (struct sockaddr *) &sock_addr, sizeof(sock_addr));
      int bytes_received = 0;
      while (bytes_sent != -1 && bytes_received != -1 && (ack.type != SCNP_ACK || memcmp(ack_src, dest_addr, ETHER_ADDR_LEN) != 0)) {
        bytes_received = scnp_recv_ackless(&ack_sock, &ack, ack_src);
      }
    }
    if (ack.type != SCNP_ACK || memcmp(ack_src, dest_addr, ETHER_ADDR_LEN) != 0) errno = ETIMEDOUT; // Set errno when no acknowledgement has been received
    if (scnp_close_socket(&ack_sock) == -1 || ack.type != SCNP_ACK || memcmp(ack_src, dest_addr, ETHER_ADDR_LEN) != 0) bytes_sent = -1;
  }
  else {
    bytes_sent = sendto(sock->packet_socket, buffer, packet_length, 0, (struct sockaddr *) &sock_addr, sizeof(sock_addr));
  }
  free(buffer);

  return bytes_sent;
}

int scnp_send_ack(struct scnp_socket * sock, const uint8_t * dest_addr)
{
  /* init acknowledgement */
  struct scnp_ack ack;
  ack.type = SCNP_ACK;

  /* send acknowledgement */
  return scnp_send(sock, dest_addr, (struct scnp_packet *) &ack, sizeof(ack));
}

int scnp_recv(struct scnp_socket * sock, struct scnp_packet * packet)
{
  /* init sock_addr */
  struct sockaddr_ll sock_addr;
  sock_addr.sll_protocol = ETH_P_SCNP;
  sock_addr.sll_ifindex = sock->if_index;
  socklen_t sock_len = sizeof(sock_addr);

  /* init buffer */
  uint8_t buffer[MAX_SCNP_PACKET_LENGTH];
  memset(buffer, 0, MAX_SCNP_PACKET_LENGTH);

  /* receive packet */
  int bytes_received = recvfrom(sock->packet_socket, buffer, sizeof(buffer), 0, (struct sockaddr *) &sock_addr, &sock_len);
  if (bytes_received == -1) return -1;

  /* copy buffer into packet */
  scnp_buffer_to_packet(buffer, packet);

  /* send ack */
  if (scnp_is_ack_needed(packet)) {
    scnp_send_ack(sock, sock_addr.sll_addr);
  }

  return bytes_received;
}

int scnp_recv_from(struct scnp_socket * sock, struct scnp_packet * packet, uint8_t * src_addr)
{
  /* init sock_addr */
  struct sockaddr_ll sock_addr;
  sock_addr.sll_protocol = ETH_P_SCNP;
  sock_addr.sll_ifindex = sock->if_index;
  socklen_t sock_len = sizeof(sock_addr);

  /* init buffer */
  uint8_t buffer[MAX_SCNP_PACKET_LENGTH];
  memset(buffer, 0, MAX_SCNP_PACKET_LENGTH);

  /* receive packet */
  int bytes_received = recvfrom(sock->packet_socket, buffer, sizeof(buffer), 0, (struct sockaddr *) &sock_addr, &sock_len);
  if (bytes_received == -1) return -1;

  /* copy source address into parameter address */
  memcpy(src_addr, sock_addr.sll_addr, ETHER_ADDR_LEN);

  /* copy buffer into packet */
  scnp_buffer_to_packet(buffer, packet);

  /* send ack */
  if (scnp_is_ack_needed(packet)) {
    scnp_send_ack(sock, sock_addr.sll_addr);
  }

  return bytes_received;
}

struct scnp_session                     // Doubly linked list containing SCNP session threads
{
  int if_index;                         // Index of the interface used to send SCNP packets
  pthread_t session_thread;             // Pthread used to send SCNP management packets
  int is_session_running;               // Boolean which tells the thread it has to be stopped
  struct scnp_session * next_session;   // Pointer to the next session in the list
  struct scnp_session * prev_session;   // Pointer to the previous session in the list
};

void insert_session(struct scnp_session ** head, struct scnp_session * new_session)
{
  new_session->prev_session = NULL;
  new_session->next_session = *head;
  if (*head != NULL)
    (*head)->prev_session = new_session;
  *head = new_session;
}

void remove_session(struct scnp_session ** head, struct scnp_session * to_remove)
{
  if (to_remove->next_session != NULL)
    to_remove->next_session->prev_session = to_remove->prev_session;
  if (to_remove->prev_session != NULL)
    to_remove->prev_session->next_session = to_remove->next_session;
  else
    *head = to_remove->next_session;
}

struct scnp_session * get_session(struct scnp_session * head, int if_index)
{
  struct scnp_session * current_session = head;

  while (current_session != NULL && current_session->if_index != if_index) {
    current_session = current_session->next_session;
  }

  return current_session;
}

struct scnp_session * session = NULL;  // Head of the session list

void * session_run(void * session_data)
{
  /* get the session */
  struct scnp_session * current_session = (struct scnp_session *) session_data;

  /* init socket */
  struct scnp_socket session_sock;
  if (scnp_create_socket(&session_sock, current_session->if_index) == 0) { // TODO return error by a pipe to the main thread

    /* init data to send */
    struct scnp_management packet;
    packet.type = SCNP_MNGT;
    packet.flags = 0x80;
    size_t packet_length = sizeof(packet);

    /* send I'm alive packet every SESSION_TIMEOUT */
    while (current_session->is_session_running) {
      scnp_send(&session_sock, broadcast_ll_addr, (struct scnp_packet *) &packet, packet_length);
      sleep(SESSION_TIMEOUT);
    }

    /* close socket */
    scnp_close_socket(&session_sock);
  }

  return NULL;
}

int scnp_start_session(int if_index)
{
  /* init the new session */
  struct scnp_session * new_session;

  /* allocate new session if it does not already exist */
  if (get_session(session, if_index) != NULL) return -1;
  new_session = (struct scnp_session *) malloc(sizeof(struct scnp_session));

  /* fill session structure */
  new_session->if_index = if_index;
  new_session->is_session_running = 1;

  /* add the new session in the list */
  insert_session(&session, new_session);

  /* run session thread */
  if (pthread_create(&(new_session->session_thread), NULL, session_run, (void *) new_session) == -1) return -1;

  return 0;
}

int scnp_stop_session(int if_index)
{
  /* get the session to stop */
  struct scnp_session * running_session = get_session(session, if_index);

  if (running_session == NULL) return -1;

  /* stop the session */
  running_session->is_session_running = 0;
  pthread_join(running_session->session_thread, NULL);

  /* remove the session from the list */
  remove_session(&session, running_session);

  /* free the session */
  free(running_session);

  return 0;
}
