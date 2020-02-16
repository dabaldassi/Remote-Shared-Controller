#include <stdlib.h>
#include <string.h>

#include <scnp_socket.h>
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>

#include "queue.h"
#include "interface.h"
#include "scnp.h"

#define ID_MAX_INCR 0x100000
#define ACK_TIMEOUT_NS 1000000000
#define ACK_MAX_TRIES 3
#define SESSION_TIMEOUT 1


/* information used by all threads */
static struct
{
  struct scnp_socket  socket;
  struct scnp_queue * rqueue;
  struct scnp_queue * aqueue;
  struct scnp_queue * squeue;
  pthread_t rthread;
  bool is_rthread_running;
  pthread_t sthread;
  bool is_sthread_running;
  bool stop_mthread;
} thread_info = {
    .rqueue = NULL,
    .aqueue = NULL,
    .squeue = NULL,
    .is_rthread_running = false,
    .is_sthread_running = false,
    .stop_mthread = true
};

uint32_t current_id; // current identifier for packet that need acknowledgement

/* parameters of the sending and receiving threads */
typedef struct
{
  int if_index;
  sem_t thread_cnt; // used to resume scnp_start after the thread is launched
} param_t;

static int stop_and_fail(void)
{
  scnp_stop();
  return -1;
}

static void * recv_packets(void * arg);
static void * send_packets(void * arg);
static void * manage(void * arg);

int scnp_start(unsigned int if_index)
{
  /* do not start if it is already started */
  if (
      scnp_socket_opened(&thread_info.socket) ||
      thread_info.is_rthread_running ||
      thread_info.is_sthread_running
      ) {
    errno = EALREADY;
    return -1;
  }

  /* verify interface existence */
  if (!interface_exists(if_index)) {
    if (errno == 0) errno = ENODEV;
    return -1;
  }

  /* open a socket to send and receive SCNP data */

  if (scnp_socket_open(&thread_info.socket, if_index)) {
      return stop_and_fail();
  }

  /* initialize the current identifier */
  srandom(time(NULL));
  current_id = random();

  /* initialize threads parameters */
  param_t param;
  param.if_index = (int) if_index;
  sem_init(&param.thread_cnt, 0, 0);

  /* create the receiving thread */
  if (pthread_create(&thread_info.rthread, NULL, recv_packets, (void *) &param)) {
    sem_destroy(&param.thread_cnt);
    return stop_and_fail();
  }
  /* wait for the end of the receiving thread initialization */
  sem_wait(&param.thread_cnt);

  /* create the sending thread */
  if (pthread_create(&thread_info.sthread, NULL, send_packets, (void *) &param)) {
    sem_destroy(&param.thread_cnt);
    return stop_and_fail();
  }
  /* wait for the end of the sending thread initialization */
  sem_wait(&param.thread_cnt);
  sem_destroy(&param.thread_cnt);

  return 0;
}

void scnp_stop(void)
{
  /* stop sending thread */
  if (thread_info.is_sthread_running) {
    pthread_cancel(thread_info.sthread);
  }
  pthread_join(thread_info.sthread, NULL);

  /* stop receiving thread */
  if (thread_info.is_rthread_running) {
    pthread_cancel(thread_info.rthread);
  }
  pthread_join(thread_info.rthread, NULL);

  /* close the socket */
  if (scnp_socket_opened(&thread_info.socket)) scnp_socket_close(&thread_info.socket);
}

static int is_ack_needed(const struct scnp_packet * packet);
static bool is_ack_correct(const struct scnp_packet * sent, const uint8_t * dest_addr, const struct scnp_packet * received, const uint8_t * src_addr);
static uint32_t get_id_from_packet(const struct scnp_packet * packet);

int scnp_send(struct scnp_packet * packet, const uint8_t * dest_addr)
{
  /* raise error when the sending thread is not running */
  if (!thread_info.is_sthread_running) {
    errno = ESRCH;
    return -1;
  }

  if (is_ack_needed(packet)) {
    /* initialize acknowledgement packet and address */
    struct scnp_packet ack;
    ack.type = 0;
    uint8_t ack_addr[ETHER_ADDR_LEN];
    memset(ack_addr, 0, ETHER_ADDR_LEN);

    int tries = 0;
    do {
      if (push(thread_info.squeue, packet, dest_addr)) return -1;
      ++tries;
      pull(thread_info.aqueue, &ack, ack_addr, ACK_TIMEOUT_NS);
    } while (tries < ACK_MAX_TRIES && !is_ack_correct(packet, dest_addr, &ack, ack_addr));
    if (!is_ack_correct(packet, dest_addr, &ack, ack_addr)) {
      errno = ETIMEDOUT;
      return -1;
    }
  }
  else {
    if (push(thread_info.squeue, packet, dest_addr)) return -1;
  }

  return 0;
}

int scnp_recv(struct scnp_packet * packet, uint8_t * src_addr)
{
  /* raise error when the receiving thread is not running */
  if (!thread_info.is_rthread_running)  {
    errno = ESRCH;
    return -1;
  }

  if (pull(thread_info.rqueue, packet, src_addr, -1)) return -1;

  if (is_ack_needed(packet)) {
    /* initialize acknowledgement packet */
    struct scnp_ack ack;
    ack.type = SCNP_ACK;
    ack.id = get_id_from_packet(packet);
    /* send acknowledgement */
    scnp_send((struct scnp_packet *) &ack, src_addr);
  }

  return 0;
}

static int map_type(uint8_t type)
{
  if (type >= 0xfe) return type - 0xfe + 3;
  return type - 1;
}

static int build_key_packet(struct scnp_packet * packet, const uint8_t * buf)
{
  struct scnp_key key;
  /* type */
  key.type = SCNP_KEY;
  /* id */
  memcpy(&key.id, buf, sizeof(uint32_t));
  key.id = ntohl(key.id);
  /* code */
  memcpy(&key.code, buf + sizeof(uint32_t), sizeof(uint16_t));
  key.code = ntohs(key.code);
  /* pressed */
  key.pressed = *(buf + sizeof(uint32_t) + sizeof(uint16_t)) >> 7u;
  /* repeated */
  key.repeated = *(buf + sizeof(uint32_t) + sizeof(uint16_t)) & (1u << 6u);

  memcpy(packet, &key, sizeof(struct scnp_key));

  return 0;
}

static int build_mov_packet(struct scnp_packet * packet, const uint8_t * buf)
{
  struct scnp_movement mov;
  /* type */
  mov.type = SCNP_MOV;
  /* code */
  memcpy(&mov.code, buf, sizeof(uint16_t));
  mov.code = ntohs(mov.code);
  /* value */
  memcpy(&mov.value, buf + sizeof(uint16_t), sizeof(uint32_t));
  mov.value = ntohl(mov.value);

  memcpy(packet, &mov, sizeof(struct scnp_movement));

  return 0;
}

static int build_out_packet(struct scnp_packet * packet, const uint8_t * buf)
{
  struct scnp_out out;
  /* type */
  out.type = SCNP_OUT;
  /* id */
  memcpy(&out.id, buf, sizeof(uint32_t));
  out.id = ntohl(out.id);
  /* direction */
  out.direction = *(buf + sizeof(uint32_t)) >> 7u;
  /* side */
  out.side = *(buf + sizeof(uint32_t)) & (1u << 6u);
  /* height */
  uint16_t height;
  memcpy(&height, buf + sizeof(uint32_t) + sizeof(uint8_t), sizeof(uint16_t));
  out.height = (float) ntohs(height) / ((1u << 16u) - 1u);

  memcpy(packet, &out, sizeof(struct scnp_out));

  return 0;
}

static int build_mng_packet(struct scnp_packet * packet, const uint8_t * buf)
{
  struct scnp_management mng;
  /* type */
  mng.type = SCNP_MNG;
  /* hostname */
  memcpy(&mng.hostname, buf, HOSTNAME_LENGTH);

  memcpy(packet, &mng, sizeof(struct scnp_management));

  return 0;
}

static int build_ack_packet(struct scnp_packet * packet, const uint8_t * buf)
{
  struct scnp_ack ack;
  /* type */
  ack.type = SCNP_ACK;
  /* id */
  memcpy(&ack.id, buf, sizeof(uint32_t));
  ack.id = ntohl(ack.id);

  memcpy(packet, &ack, sizeof(struct scnp_ack));

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

static uint8_t * alloc_buffer(uint8_t type, size_t * length)
{
  uint8_t * buffer = NULL;
  size_t packet_sizes[] = { KEY_LENGTH, MOV_LENGTH, OUT_LENGTH, MNG_LENGTH, ACK_LENGTH };
  if (type == SCNP_KEY || type == SCNP_MOV || type == SCNP_OUT || type == SCNP_MNG || type == SCNP_ACK) {
    *length = packet_sizes[map_type(type)];
    buffer = (uint8_t *) malloc(*length);
    if (buffer == NULL) return NULL;
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

  /* id */
  uint32_t id = htonl(current_id += random() % ID_MAX_INCR);
  memcpy(buf, &id, sizeof(uint32_t));
  /* code */
  uint16_t code = htons(p->code);
  memcpy(buf + sizeof(uint32_t), &code, sizeof(uint16_t));
  /* flags */
  uint8_t pressed_flag = (p->pressed) << 7u;
  uint8_t repeated_flag = (p->repeated) << 6u;
  *(buf + sizeof(uint32_t) + sizeof(uint16_t)) = pressed_flag + repeated_flag;

  return 0;
}

static int build_mov_buffer(uint8_t * buf, const struct scnp_packet * packet)
{
  // TODO same as previous function
  struct scnp_movement * p = (struct scnp_movement *) packet;

  /* code */
  uint16_t code = htons(p->code);
  memcpy(buf, &code, sizeof(uint16_t));
  /* value */
  uint32_t value = htonl(p->value);
  memcpy(buf + sizeof(uint16_t), &value, sizeof(uint32_t));

  return 0;
}

static int build_out_buffer(uint8_t * buf, const struct scnp_packet * packet)
{
  // TODO same as previous function
  struct scnp_out * p = (struct scnp_out *) packet;

  /* id */
  uint32_t id = htonl(current_id += random() % ID_MAX_INCR);
  memcpy(buf, &id, sizeof(uint32_t));
  /* flags */
  uint8_t direction_flag = (p->direction) << 7u;
  uint8_t side_flag = (p->side) << 6u;
  *(buf + sizeof(uint32_t)) = direction_flag + side_flag;
  /* height */
  if (p->height > 1 || p->height < 0) p->height = 0.5f;
  uint16_t height = p->height * ((1u << 16u) - 1u);
  height = htons(height);
  memcpy(buf + sizeof(uint32_t) + sizeof(uint8_t), &height, sizeof(uint16_t));

  return 0;
}

static int build_mng_buffer(uint8_t * buf, const struct scnp_packet * packet)
{
  // TODO same as previous function
  struct scnp_management * p = (struct scnp_management *) packet;

  /* hostname */
  memcpy(buf, p->hostname, HOSTNAME_LENGTH);

  return 0;
}

static int build_ack_buffer(uint8_t * buf, const struct scnp_packet * packet)
{
  // TODO same as previous function
  struct scnp_ack * p = (struct scnp_ack *) packet;

  /* id */
  uint32_t id = htonl(p->id);
  memcpy(buf, &id, sizeof(uint32_t));

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

  /* type */
  *buf = packet->type;

  return buf_builders[map_type(packet->type)](buf + 1, packet);
}

static void rcleanup(void * garbage)
{
  /* free all allocated memory */
  if (garbage != NULL) free(garbage);
  free_queue(thread_info.rqueue);
  thread_info.rqueue = NULL;
  free_queue(thread_info.aqueue);
  thread_info.aqueue = NULL;

  thread_info.is_rthread_running = false;
}

static void * recv_packets(void * arg)
{
  thread_info.is_rthread_running = true;
  bool stop = false;

  /* initialize parameters */
  param_t * param = (param_t *) arg;
  int if_index = param->if_index;

  /* allocate memory for the buffer */
  size_t packetlen = sizeof(struct scnp_packet);
  uint8_t * buf = (uint8_t *) malloc(packetlen);
  if (buf == NULL) stop = true;

  /* initialize receive and ack queue */
  thread_info.rqueue = init_queue();
  thread_info.aqueue = init_queue();
  if (thread_info.rqueue == NULL || thread_info.aqueue == NULL) {
    stop = true;
  }

  /* push cleanup routine */
  pthread_cleanup_push(rcleanup, buf)

  /* resume scnp_start */
  sem_post(&param->thread_cnt);

  /* initialize the packet and the socket address */
  struct scnp_packet packet;
  uint8_t            addr[ETHER_ADDR_LEN];


  while (!stop) {
    /* reset packet and buffer */
    memset(&packet, 0, sizeof(struct scnp_packet));
    memset(buf, 0, MAX_PACKET_LENGTH);
    /* receive scnp data */
    if (scnp_socket_recvfrom(&thread_info.socket, buf, packetlen, 0, addr) == -1) {
      stop = true;
    }
    /* build the packet from buffer data */
    if (build_packet(&packet, buf) == 0) {
      /* push the packet in the corresponding queue */
      if (packet.type == SCNP_ACK) {
        push(thread_info.aqueue, &packet, addr);
      }
      else {
        push(thread_info.rqueue, &packet, addr);
      }
    }
  }

  /* execute rcleanup */
  pthread_cleanup_pop(1);

  return NULL;
}

struct waste_t
{
  uint8_t * buf;
  pthread_t mthread;
};

static void scleanup(void * garbage)
{
  /* free all allocated memory */
  struct waste_t * waste = (struct waste_t *) garbage;
  free(waste->buf);
  thread_info.stop_mthread = true;
  pthread_join(waste->mthread, NULL);
  free_queue(thread_info.squeue);
  thread_info.squeue = NULL;

  thread_info.is_sthread_running = false;
}

static void * send_packets(void * arg)
{
  thread_info.is_sthread_running = true;
  bool stop = false;

  /* initialize parameters */
  param_t * param = (param_t *) arg;
  int if_index = param->if_index;

  /* declare buffer */
  struct waste_t waste;
  waste.buf = NULL;

  /* initialize sending queue */
  thread_info.squeue = init_queue();
  if (thread_info.squeue == NULL) stop = true;

  /* create management thread */
  thread_info.stop_mthread = false;
  if (pthread_create(&waste.mthread, NULL, manage, NULL)) {
    stop = true;
  }

  /* push cleanup routine */
  pthread_cleanup_push(scleanup, &waste)

  /* resume scnp_start */
  sem_post(&param->thread_cnt);

  /* initialize the packet and the socket address */
  struct scnp_packet packet;
  uint8_t            addr[ETHER_ADDR_LEN];
  
  /* initialize the queue timeout */
  int64_t timeout = -1;

  while(!stop) {
    /* pull packet */
    if (pull(thread_info.squeue, &packet, addr, timeout)) {
      stop = true;
    }
    /* initialize buffer */
    size_t buf_length;
    waste.buf = alloc_buffer(packet.type, &buf_length);
    if (waste.buf == NULL && errno != EBADMSG) stop = true;
    /* build packet */
    if (waste.buf != NULL && build_buffer(waste.buf, &packet) == 0) {
      /* send packet */
      if (scnp_socket_sendto(&thread_info.socket, waste.buf, buf_length, 0,addr) <= 0) {
        stop = true;
      }
    }
    /* reset buffer */
    free(waste.buf);
    waste.buf = NULL;
  }

  /* exxecute scleanup */
  pthread_cleanup_pop(1);

  return NULL;
}

static void * manage(void * arg)
{
  arg = (void *) arg;

  /* initialize ethernet broadcast address */
  uint8_t broadcast[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

  /* initialize management packet */
  struct scnp_management mng;
  mng.type = SCNP_MNG;
  memset(mng.hostname, 0, HOSTNAME_LENGTH);
  if (gethostname(mng.hostname, HOSTNAME_LENGTH)) {
    mng.hostname[HOSTNAME_LENGTH - 1] = 0;
    errno = 0;
  }

  while (!thread_info.stop_mthread) {
    /* send management packet */
    if (scnp_send((struct scnp_packet *) &mng, broadcast) && (errno == ENOMEM || errno == ESRCH)) {
      return NULL;
    }
    /* wait */
    sleep(SESSION_TIMEOUT);
  }

  return NULL;
}

static int is_ack_needed(const struct scnp_packet * packet)
{
  return packet->type == SCNP_KEY  || packet->type == SCNP_OUT;
}

static uint32_t get_id_from_packet(const struct scnp_packet * packet)
{
  switch (packet->type) {
    case SCNP_KEY:
    {
      struct scnp_key * key = (struct scnp_key *) packet;
      return key->id;
    }
    case SCNP_OUT:
    {
      struct scnp_out * out = (struct scnp_out *) packet;
      return out->id;
    }
    default:
      errno = EBADMSG;
  }

  return 0;
}

static bool is_ack_correct(const struct scnp_packet * sent, const uint8_t * dest_addr, const struct scnp_packet * received, const uint8_t * src_addr)
{
  return (received->type == SCNP_ACK && get_id_from_packet(sent) == get_id_from_packet(received) && memcmp(src_addr, dest_addr, ETHER_ADDR_LEN) == 0);
}
