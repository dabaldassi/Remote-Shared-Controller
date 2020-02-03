#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>

#include "queue.h"
#include "interface.h"
#include "scnp.h"

#define ACK_TIMEOUT_NS 1000000000
#define ACK_MAX_TRIES 3
#define SESSION_TIMEOUT 1


static int is_ack_needed(const struct scnp_packet * packet)
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
  uint8_t pressed_flag = (p->pressed) << 7u;
  uint8_t repeated_flag = (p->repeated) << 6u;
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

  uint8_t direction_flag = (p->direction) << 7u;
  uint8_t side_flag = (p->side) << 6u;
  *buf = direction_flag + side_flag;
  if (p->height > 1 || p->height < 0) {
    p->height = 0.5f;
  }
  uint16_t height = p->height * ((1u << 16u) - 1u);
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
  key.pressed = *(buf + sizeof(uint16_t)) >> 7u;
  key.repeated = *(buf + sizeof(uint16_t)) & (1u << 6u);

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
  out.direction = *buf >> 7u;
  out.side = *buf & (1u << 6u);
  uint16_t height;
  memcpy(&height, buf + sizeof(uint8_t), sizeof(uint16_t));
  out.height = (float) ntohs(height) / ((1u << 16u) - 1u);

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

static struct
{
  int socket;
  uint64_t iface;
  struct scnp_queue * rqueue;
  struct scnp_queue * aqueue;
  struct scnp_queue * squeue;
  pthread_t rthread;
  bool is_rthread_running;
  pthread_t sthread;
  bool is_sthread_running;
  pthread_t mthread;
  bool is_mthread_running;
} thread_info = {
    .socket = -1,
    .iface = 0,
    .rqueue = NULL,
    .aqueue = NULL,
    .squeue = NULL,
    .is_rthread_running = false,
    .is_sthread_running = false,
    .is_mthread_running = false
};

void scnp_stop(void)
{
  thread_info.iface = 0;

  if (thread_info.is_mthread_running) {
    pthread_join(thread_info.mthread, NULL);
  }
  if (thread_info.is_rthread_running) {
    pthread_cancel(thread_info.rthread);
    pthread_join(thread_info.rthread, NULL);
  }
  if (thread_info.is_sthread_running) {
    pthread_cancel(thread_info.sthread);
    pthread_join(thread_info.sthread, NULL);
  }

  free_queue(thread_info.rqueue);
  free_queue(thread_info.aqueue);
  free_queue(thread_info.squeue);

  if (thread_info.socket >= 0) close(thread_info.socket);
  thread_info.socket = -1;
}

static void rcleanup(void * garbage)
{
  free(garbage);
  thread_info.is_rthread_running = false;
}

static void * recv_packets(void * arg)
{
  thread_info.is_rthread_running = true;
  uint64_t  if_index = (uint64_t) arg;

  /* allocate memory for the buffer */
  size_t packetlen = sizeof(struct scnp_packet);
  uint8_t * buf = (uint8_t *) malloc(packetlen);
  if (buf == NULL) {
    thread_info.is_rthread_running = false;
    scnp_stop();
    return NULL;
  }
  pthread_cleanup_push(rcleanup, buf)

  /* initialize the packet and the socket address */
  struct scnp_packet packet;
  socklen_t addrlen = sizeof(struct sockaddr_ll);
  struct sockaddr_ll addr;
  memset(&addr, 0, addrlen);
  addr.sll_family = AF_PACKET;
  addr.sll_protocol = htons(ETH_P_SCNP);
  addr.sll_ifindex = if_index;

  while (if_index == thread_info.iface) {
    memset(&packet, 0, sizeof(struct scnp_packet));
    memset(buf, 0, MAX_PACKET_LENGTH);
    if (recvfrom(thread_info.socket, buf, packetlen, 0, (struct sockaddr *) &addr, &addrlen) == -1) {
      thread_info.is_rthread_running = false;
      scnp_stop();
    }
    if (build_packet(&packet, buf) == 0) {
      if (packet.type == SCNP_ACK) push(thread_info.aqueue, &packet, addr.sll_addr);
      else push(thread_info.rqueue, &packet, addr.sll_addr);
    }
  }

  pthread_cleanup_pop(1);

  return NULL;
}

static void scleanup(void * garbage)
{
  free(garbage);
  thread_info.is_sthread_running = false;
}

static void * send_packets(void * arg)
{
  thread_info.is_sthread_running = true;
  uint64_t if_index = (uint64_t) arg;

  /* initialize the packet and the socket address */
  struct scnp_packet packet;
  socklen_t addrlen = sizeof(struct sockaddr_ll);
  struct sockaddr_ll addr;
  memset(&addr, 0, addrlen);
  addr.sll_family = AF_PACKET;
  addr.sll_protocol = htons(ETH_P_SCNP);
  addr.sll_ifindex = if_index;
  addr.sll_halen = ETHER_ADDR_LEN;

  /* initialize the queue timeout */
  int64_t timeout = -1;

  while(if_index == thread_info.iface) {
    pull(thread_info.squeue, &packet, addr.sll_addr, timeout);
    size_t buf_length;
    uint8_t * buf = alloc_buffer(packet.type, &buf_length);
    pthread_cleanup_push(scleanup, buf)
    if (buf == NULL && errno != EBADMSG) {
      thread_info.is_sthread_running = false;
      scnp_stop();
    }
    if (buf != NULL && build_buffer(buf, &packet) == 0)
      sendto(thread_info.socket, buf, buf_length, 0, (struct sockaddr *) &addr, addrlen);
    pthread_cleanup_pop(0);
    free(buf);
  }

  thread_info.is_sthread_running = false;

  return NULL;
}

static void * run_session(void * arg)
{
  thread_info.is_mthread_running = true;
  uint64_t if_index = (uint64_t) arg;

  /* initialize management packet */
  uint8_t broadcast[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  struct scnp_management mng;
  mng.type = SCNP_MNG;
  memset(mng.hostname, 0, HOSTNAME_LENGTH);
  if (gethostname(mng.hostname, HOSTNAME_LENGTH)) {
    mng.hostname[HOSTNAME_LENGTH - 1] = 0;
    errno = 0;
  }

  while (if_index == thread_info.iface) {
    if (scnp_send((struct scnp_packet *) &mng, broadcast) && (errno == ENOMEM || errno == ESRCH)) {
      thread_info.is_mthread_running = false;
      scnp_stop();
      return NULL;
    }
    sleep(SESSION_TIMEOUT);
  }

  thread_info.is_mthread_running = false;

  return NULL;
}

static int stop_and_fail(void)
{
  scnp_stop();
  return -1;
}

int scnp_start(unsigned int if_index)
{
  /* do not start if it is already started */
  if (
    thread_info.iface > 0 ||
    thread_info.socket >= 0 ||
    thread_info.is_rthread_running ||
    thread_info.is_sthread_running ||
    thread_info.is_mthread_running ||
    thread_info.rqueue != NULL ||
    thread_info.aqueue != NULL ||
    thread_info.squeue != NULL
  ) {
    errno = EALREADY;
    return -1;
  }

  if (!interface_exists(if_index)) {
    if (errno == 0) errno = ENODEV;
    return -1;
  }

  /* create a socket to send and receive SCNP data */
  thread_info.socket = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_SCNP));
  if (thread_info.socket < 0) return -1;

  /* bind the socket to the interface */
  struct sockaddr_ll addr;
  socklen_t addrlen = sizeof(struct sockaddr_ll);
  memset(&addr, 0, addrlen);
  addr.sll_family = AF_PACKET;
  addr.sll_protocol = htons(ETH_P_SCNP);
  addr.sll_ifindex = (int) if_index;
  if (bind(thread_info.socket, (struct sockaddr *) &addr, addrlen)) {
    return stop_and_fail();
  }
  thread_info.iface = if_index;

  /* initialize the receiving queue and the sending queue */
  thread_info.rqueue = init_queue();
  thread_info.aqueue = init_queue();
  thread_info.squeue = init_queue();
  if (thread_info.rqueue == NULL || thread_info.aqueue == NULL || thread_info.squeue == NULL) {
    return stop_and_fail();
  }

  /* create the receiving thread and the sending thread */
  if (pthread_create(&thread_info.rthread, NULL, recv_packets, (void *) thread_info.iface)) {
    return stop_and_fail();
  }
  if (pthread_create(&thread_info.sthread, NULL, send_packets, (void *) thread_info.iface)) {
    return stop_and_fail();
  }

  /* create the management thread */
  if (pthread_create(&thread_info.mthread, NULL, run_session, (void *) thread_info.iface)) {
    return stop_and_fail();
  }

  return 0;
}

int scnp_send(struct scnp_packet * packet, const uint8_t * dest_addr)
{
  if (!thread_info.is_sthread_running) {
    errno = ESRCH;
    return -1;
  }

  if (is_ack_needed(packet)) {
    struct scnp_packet ack;
    ack.type = 0;
    uint8_t ack_addr[ETHER_ADDR_LEN];
    memset(ack_addr, 0, ETHER_ADDR_LEN);
    int tries = 0;
    do {
      if (push(thread_info.squeue, packet, dest_addr)) return -1;
      ++tries;
      pull(thread_info.aqueue, &ack, ack_addr, ACK_TIMEOUT_NS);
    } while (tries < ACK_MAX_TRIES && (ack.type != SCNP_ACK || memcmp(ack_addr, dest_addr, ETHER_ADDR_LEN) != 0));
    if (ack.type != SCNP_ACK || memcmp(ack_addr, dest_addr, ETHER_ADDR_LEN) != 0) return -1;
  }
  else {
    if (push(thread_info.squeue, packet, dest_addr)) return -1;
  }

  return 0;
}

int scnp_recv(struct scnp_packet * packet, uint8_t * src_addr)
{
  if (!thread_info.is_rthread_running)  {
    errno = ESRCH;
    return -1;
  }

  if (pull(thread_info.rqueue, packet, src_addr, -1)) return -1;

  if (is_ack_needed(packet)) {
    struct scnp_ack ack;
    ack.type = SCNP_ACK;
    scnp_send((struct scnp_packet *) &ack, src_addr);
  }

  return 0;
}
