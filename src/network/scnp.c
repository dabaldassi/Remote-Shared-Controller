#include <stdlib.h>
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
  int fd = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_SCNP));
  if (fd == -1) return -1;

  /* provide information in socket structure */
  sock->fd = fd;
  sock->if_index = if_index;

  return 0;
}

int scnp_close_socket(struct scnp_socket * sock)
{
  return close(sock->fd);
}

static int scnp_is_ack_needed(const struct scnp_packet * packet)
{
  return packet->type != SCNP_ACK && packet->type != SCNP_MNG && packet->type != SCNP_MOV;
}

static int map_type(uint8_t type)
{
  if (type >= 0xfe) return type - 0xfe + 3;
  return type - 1;
}

static uint8_t * alloc_buffer(uint8_t type, size_t * length)
{
  uint8_t * buffer = NULL;
  size_t packet_sizes[] = {KEY_LENGTH, MOV_LENGTH, OUT_LENGTH, MNG_LENGTH, ACK_LENGTH};
  if (type == SCNP_KEY || type == SCNP_MOV || type == SCNP_OUT || type == SCNP_MNG || type == SCNP_ACK) {
    *length = packet_sizes[map_type(type)];
    buffer = (uint8_t *) malloc(*length);
    memset(buffer, 0, *length);
  } else {
    *length = 0;
    errno = EBADMSG;
  }
  return buffer;
}

static int build_key_buffer(uint8_t * buf, const struct scnp_packet * packet)
{
  // TODO condition with sizeof scnp key and scnp packet
  struct scnp_key * p = (struct scnp_key *) packet;

  uint16_t code = htons(p->code);
  memcpy(buf, &code, sizeof(uint16_t));
  uint8_t pressed_flag = (p->pressed) << 7;
  uint8_t repeated_flag = (p->repeated) << 6;
  *(buf + sizeof(uint16_t)) = pressed_flag + repeated_flag;

  return 0;
}

static int build_mov_buffer(uint8_t * buf, const struct scnp_packet * packet)
{
  // TODO same as previous function
  struct scnp_movement * p = (struct scnp_movement *) packet;

  uint16_t code = htons(p->code);
  memcpy(buf, &code, sizeof(uint16_t));
  uint32_t value = htonl(p->value);
  memcpy(buf + sizeof(uint16_t), &value, sizeof(uint32_t));

  return 0;
}

static int build_out_buffer(uint8_t * buf, const struct scnp_packet * packet)
{
  // TODO same as previous function
  struct scnp_out * p = (struct scnp_out *) packet;

  uint8_t direction_flag = (p->direction) << 7;
  uint8_t side_flag = (p->side) << 6;
  *buf = direction_flag + side_flag;
  if (p->height > 1 || p->height < 0) {
    p->height = 0.5f;
  }
  uint16_t height = p->height * ((1 << 16) - 1);
  height = htons(height);
  memcpy(buf + sizeof(uint8_t), &height, sizeof(uint16_t));

  return 0;
}

static int build_mng_buffer(uint8_t * buf, const struct scnp_packet * packet)
{
  // TODO same as previous function
  struct scnp_management * p = (struct scnp_management *) packet;

  memcpy(buf, p->hostname, HOSTNAME_LENGTH);

  return 0;
}

static int build_ack_buffer(uint8_t * buf, const struct scnp_packet * packet)
{
  return 0;
}

static int build_buffer(uint8_t * buf, const struct scnp_packet * packet)
{
  static int (*buf_builders[])(uint8_t *, const struct scnp_packet *) = {
      build_key_buffer,
      build_mov_buffer,
      build_out_buffer,
      build_mng_buffer,
      build_ack_buffer
  };

  *buf = packet->type;
  return buf_builders[map_type(packet->type)](buf + 1, packet);
}

static int build_key_packet(struct scnp_packet * packet, const uint8_t * buf)
{
  struct scnp_key key;
  key.type = SCNP_KEY;
  memcpy(&key.code, buf, sizeof(uint16_t));
  key.code = ntohs(key.code);
  key.pressed = *(buf + sizeof(uint16_t)) >> 7;
  key.repeated = *(buf + sizeof(uint16_t)) & (1 << 6);

  memcpy(packet, &key, sizeof(struct scnp_key));

  return 0;
}

static int build_mov_packet(struct scnp_packet * packet, const uint8_t * buf)
{
  struct scnp_movement mov;
  mov.type = SCNP_MOV;
  memcpy(&mov.code, buf, sizeof(uint16_t));
  mov.code = ntohs(mov.code);
  memcpy(&mov.value, buf + sizeof(uint16_t), sizeof(uint32_t));
  mov.value = ntohl(mov.value);

  memcpy(packet, &mov, sizeof(struct scnp_movement));

  return 0;
}

static int build_out_packet(struct scnp_packet * packet, const uint8_t * buf)
{
  struct scnp_out out;
  out.type = SCNP_OUT;
  out.direction = *buf >> 7;
  out.side = *buf & (1 << 6);
  uint16_t height;
  memcpy(&height, buf + sizeof(uint8_t), sizeof(uint16_t));
  out.height = (float) ntohs(height) / ((1 << 16) - 1);

  memcpy(packet, &out, sizeof(struct scnp_out));

  return 0;
}

static int build_mng_packet(struct scnp_packet * packet, const uint8_t * buf)
{
  struct scnp_management mng;
  mng.type = SCNP_MNG;
  memcpy(&mng.hostname, buf, HOSTNAME_LENGTH);

  memcpy(packet, &mng, sizeof(struct scnp_management));

  return 0;
}

static int build_ack_packet(struct scnp_packet * packet, const uint8_t * buf)
{
  packet->type = SCNP_ACK;

  return 0;
}

static int build_packet(struct scnp_packet * packet, const uint8_t * buf)
{
  static int (*builders[])(struct scnp_packet *, const uint8_t *) = {
      build_key_packet,
      build_mov_packet,
      build_out_packet,
      build_mng_packet,
      build_ack_packet
  };

  if (*buf == 4 || *buf == 5 || map_type(*buf) > 4 || map_type(*buf) < 0) return -1;
  return builders[map_type(*buf)](packet, buf + 1);
}

static ssize_t scnp_recv_ackless(struct scnp_socket * sock, struct scnp_packet * packet, uint8_t * src_addr)
{
  /* init sock_addr */
  socklen_t addrlen = sizeof(struct sockaddr_ll);
  struct sockaddr_ll sock_addr;
  memset(&sock_addr, 0, addrlen);
  sock_addr.sll_protocol = htons(ETH_P_SCNP);
  sock_addr.sll_ifindex = sock->if_index;

  /* init buffer */
  uint8_t buf[MAX_PACKET_LENGTH];
  memset(buf, 0, MAX_PACKET_LENGTH);

  /* receive packet */
  ssize_t bytes_received = recvfrom(sock->fd, buf, sizeof(buf), 0, (struct sockaddr *) &sock_addr, &addrlen);
  if (bytes_received == -1) return -1;

  /* copy source address into parameter address */
  memcpy(src_addr, sock_addr.sll_addr, ETHER_ADDR_LEN);

  /* copy buffer into packet */
  if (build_packet(packet, buf)) return -1;

  return bytes_received;
}

ssize_t scnp_send(struct scnp_socket * sock, const struct scnp_packet * packet, const uint8_t * dest_addr)
{
  /* init sock_addr */
  struct sockaddr_ll sock_addr;
  memset(&sock_addr, 0, sizeof(struct sockaddr_ll));
  sock_addr.sll_family = AF_PACKET;
  memcpy(sock_addr.sll_addr, dest_addr, ETHER_ADDR_LEN);
  sock_addr.sll_halen = ETHER_ADDR_LEN;
  sock_addr.sll_ifindex = sock->if_index;
  sock_addr.sll_protocol = htons(ETH_P_SCNP);

  /* init buffer */
  size_t buf_length;
  uint8_t * buf = alloc_buffer(packet->type, &buf_length);
  if (buf == NULL) return 0;
  if (build_buffer(buf, packet)) return 0;
  
  /* send packet */
  ssize_t bytes_sent = 0;
  if (scnp_is_ack_needed(packet)) {

    /* create socket to receive acknowledgment */
    struct scnp_socket ack_sock;
    bool build_failed = scnp_create_socket(&ack_sock, sock->if_index);

    /* set acknowledgment timeout */
    struct timeval ack_timeout;
    ack_timeout.tv_sec = ACK_TIMEOUT_S;
    ack_timeout.tv_usec = ACK_TIMEOUT_US;
    build_failed = build_failed || setsockopt(ack_sock.fd, SOL_SOCKET, SO_RCVTIMEO, &ack_timeout, sizeof(ack_timeout));

    /* bind socket on sending interface */
    struct sockaddr_ll ack_addr;
    memset(&ack_addr, 0, sizeof(struct sockaddr_ll));
    ack_addr.sll_family = AF_PACKET;
    ack_addr.sll_protocol = htons(ETH_P_SCNP);
    ack_addr.sll_ifindex = sock->if_index;
    build_failed = build_failed || bind(ack_sock.fd, (struct sockaddr *) &ack_addr, sizeof(ack_addr));

    /* create response packet and response address */
    struct scnp_packet response;
    response.type = 0;
    memset(response.data, 0, MAX_PACKET_LENGTH - 1);
    uint8_t response_addr[ETHER_ADDR_LEN];

    /* send packet and wait for acknowledgment */
    int i = 0;
    do {
      bytes_sent = sendto(sock->fd, buf, buf_length, 0, (struct sockaddr *) &sock_addr, sizeof(sock_addr));
      ssize_t bytes_received = 0;
      while (!build_failed && bytes_sent != -1 && bytes_received != -1 && (response.type != SCNP_ACK || memcmp(response_addr, dest_addr, ETHER_ADDR_LEN) != 0)) {
        bytes_received = scnp_recv_ackless(&ack_sock, &response, response_addr);
      }
    } while (!build_failed && bytes_sent != -1 && ++i < ACK_MAX_TRIES && (response.type != SCNP_ACK || memcmp(response_addr, dest_addr, ETHER_ADDR_LEN) != 0));
    if (bytes_sent == -1) bytes_sent = 0;
    if (bytes_sent > 0 && (errno == EAGAIN || response.type != SCNP_ACK || memcmp(response_addr, dest_addr, ETHER_ADDR_LEN) != 0)) {
      errno = ETIMEDOUT;
      bytes_sent *= -1;
    }
    if (ack_sock.fd >= 0) scnp_close_socket(&ack_sock);
  }
  else {
    bytes_sent = sendto(sock->fd, buf, buf_length, 0, (struct sockaddr *) &sock_addr, sizeof(sock_addr));
    if (bytes_sent == -1) bytes_sent = 0;
  }
  free(buf);

  return bytes_sent;
}

static ssize_t scnp_send_ack(struct scnp_socket * sock, const uint8_t * dest_addr)
{
  /* init acknowledgement */
  struct scnp_ack ack;
  ack.type = SCNP_ACK;

  /* send acknowledgement */
  return scnp_send(sock, (struct scnp_packet *) &ack, dest_addr);
}

ssize_t scnp_recv(struct scnp_socket * sock, struct scnp_packet * packet, uint8_t * src_addr)
{
  /* init sock_addr */
  socklen_t addrlen = sizeof(struct sockaddr_ll);
  struct sockaddr_ll sock_addr;
  memset(&sock_addr, 0, addrlen);
  sock_addr.sll_protocol = ETH_P_SCNP;
  sock_addr.sll_ifindex = sock->if_index;

  /* init buffer */
  uint8_t buf[MAX_PACKET_LENGTH];
  memset(buf, 0, MAX_PACKET_LENGTH);

  /* receive packet */
  ssize_t bytes_received = recvfrom(sock->fd, buf, sizeof(buf), 0, (struct sockaddr *) &sock_addr, &addrlen);
  if (bytes_received == -1) return -1;

  /* copy source address into parameter address */
  memcpy(src_addr, sock_addr.sll_addr, ETHER_ADDR_LEN);

  /* copy buffer into packet */
  if (build_packet(packet, buf)) return -1;

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

static void insert_session(struct scnp_session ** head, struct scnp_session * new_session)
{
  new_session->prev_session = NULL;
  new_session->next_session = *head;
  if (*head != NULL)
    (*head)->prev_session = new_session;
  *head = new_session;
}

static void remove_session(struct scnp_session ** head, struct scnp_session * to_remove)
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

static void * session_run(void * session_data)
{
  /* get the session */
  struct scnp_session * current_session = (struct scnp_session *) session_data;

  /* init socket */
  struct scnp_socket session_sock;
  if (scnp_create_socket(&session_sock, current_session->if_index) == 0) { // TODO return error by a pipe to the main thread

    /* init data to send */
    struct scnp_management packet;
    packet.type = SCNP_MNG;
    gethostname(packet.hostname, HOSTNAME_LENGTH); // Possible error

    /* send I'm alive packet every SESSION_TIMEOUT */
    while (current_session->is_session_running) {
      scnp_send(&session_sock, (struct scnp_packet *) &packet, broadcast_ll_addr);
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
