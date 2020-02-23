#include "catch/catch.hpp"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <unistd.h>
#include <thread>
#include <netinet/in.h>

#include "queue.h"
#include "scnp.h"

#define LOOP_INDEX 1

TEST_CASE("queue") {
  struct scnp_queue * q = init_queue();
  REQUIRE(q != NULL);
  REQUIRE(q->head == NULL);
  REQUIRE(q->tail == NULL);
  free_queue(q);
}

TEST_CASE("push") {
  struct scnp_queue * q = init_queue();
  REQUIRE(q != NULL);
  struct scnp_packet p{};
  memset(&p, 0, sizeof(struct scnp_packet));
  uint8_t a[6] = {0, 1, 2, 3, 4, 5};
  REQUIRE(push(q, &p, a) == 0);
  REQUIRE(q->head != NULL);
  REQUIRE(q->tail != NULL);
  REQUIRE(q->head == q->tail);
  REQUIRE(memcmp(q->tail->packet, &p, sizeof(struct scnp_packet)) == 0);
  REQUIRE(memcmp(q->tail->addr, a, 6) == 0);
  REQUIRE(q->tail->prev == NULL);
  REQUIRE(q->tail->next == NULL);
  free_queue(q);

  q = init_queue();
  REQUIRE(q != NULL);
  struct scnp_packet p2{};
  memset(&p2, 1, sizeof(struct scnp_packet));
  uint8_t b[6] = {9, 8, 7, 6, 5, 4};
  REQUIRE(push(q, &p2, b) == 0);
  REQUIRE(push(q, &p, a) == 0);
  REQUIRE(q->head != NULL);
  REQUIRE(q->tail != NULL);
  REQUIRE(q->head != q->tail);
  REQUIRE(memcmp(q->tail->packet, &p, sizeof(struct scnp_packet)) == 0);
  REQUIRE(memcmp(q->tail->addr, a, 6) == 0);
  REQUIRE(q->tail->prev != NULL);
  REQUIRE(q->tail->prev == q->head);
  REQUIRE(q->tail->next == NULL);
  REQUIRE(memcmp(q->head->packet, &p2, sizeof(struct scnp_packet)) == 0);
  REQUIRE(memcmp(q->head->addr, b, 6) == 0);
  REQUIRE(q->head->next != NULL);
  REQUIRE(q->head->next == q->tail);
  REQUIRE(q->head->prev == NULL);
  free_queue(q);

  q = init_queue();
  REQUIRE(q != NULL);
  struct scnp_packet p3{};
  memset(&p3, 2, sizeof(struct scnp_packet));
  uint8_t c[6] = {0, 0, 0, 0, 0, 0};
  REQUIRE(push(q, &p2, b) == 0);
  REQUIRE(push(q, &p3, c) == 0);
  REQUIRE(push(q, &p, a) == 0);
  REQUIRE(q->head != NULL);
  REQUIRE(q->tail != NULL);
  REQUIRE(q->head != q->tail);
  REQUIRE(memcmp(q->tail->packet, &p, sizeof(struct scnp_packet)) == 0);
  REQUIRE(memcmp(q->tail->addr, a, 6) == 0);
  REQUIRE(q->tail->prev != NULL);
  REQUIRE(q->tail->next == NULL);
  REQUIRE(memcmp(q->tail->prev->packet, &p3, sizeof(struct scnp_packet)) == 0);
  REQUIRE(memcmp(q->tail->prev->addr, c, 6) == 0);
  REQUIRE(q->tail->prev->prev != NULL);
  REQUIRE(q->tail->prev->prev == q->head);
  REQUIRE(q->tail->prev->next != NULL);
  REQUIRE(q->tail->prev->next == q->tail);
  REQUIRE(memcmp(q->head->packet, &p2, sizeof(struct scnp_packet)) == 0);
  REQUIRE(memcmp(q->head->addr, b, 6) == 0);
  REQUIRE(q->head->next != NULL);
  REQUIRE(q->head->next == q->tail->prev);
  REQUIRE(q->head->prev == NULL);
  free_queue(q);
}

TEST_CASE("pull") {
  struct scnp_queue * q = init_queue();
  REQUIRE(q != NULL);
  struct scnp_packet p{};
  memset(&p, 0, sizeof(struct scnp_packet));
  uint8_t a[6] = {0, 1, 2, 3, 4, 5};
  struct scnp_packet p2{};
  memset(&p2, 1, sizeof(struct scnp_packet));
  uint8_t b[6] = {9, 8, 7, 6, 5, 4};
  struct scnp_packet p3{};
  memset(&p3, 2, sizeof(struct scnp_packet));
  uint8_t c[6] = {0, 0, 0, 0, 0, 0};
  struct scnp_packet pulled{};
  memset(&pulled, 0, sizeof(struct scnp_packet));
  uint8_t z[6] = {255, 255, 255, 255, 255, 255};
  REQUIRE(push(q, &p, a) == 0);
  REQUIRE(pull(q, &pulled, z, -1) == 0);
  REQUIRE(memcmp(&pulled, &p, sizeof(struct scnp_packet)) == 0);
  REQUIRE(memcmp(z, a, 6) == 0);
  REQUIRE(q->head == NULL);
  REQUIRE(q->tail == NULL);
  free_queue(q);

  q = init_queue();
  REQUIRE(q != NULL);
  REQUIRE(push(q, &p2, b) == 0);
  REQUIRE(push(q, &p, a) ==0);
  REQUIRE(pull(q, &pulled, z, -1) == 0);
  REQUIRE(memcmp(&pulled, &p2, sizeof(struct scnp_packet)) == 0);
  REQUIRE(memcmp(z, b, 6) == 0);
  REQUIRE(q->head != NULL);
  REQUIRE(q->tail != NULL);
  REQUIRE(q->head == q->tail);
  REQUIRE(memcmp(q->head->packet, &p, sizeof(struct scnp_packet)) == 0);
  REQUIRE(memcmp(q->head->addr, a, 6) == 0);
  REQUIRE(q->head->prev == NULL);
  REQUIRE(q->head->next == NULL);
  free_queue(q);

  q = init_queue();
  REQUIRE(q != NULL);
  REQUIRE(push(q, &p3, c) == 0);
  REQUIRE(push(q, &p, a) == 0);
  REQUIRE(push(q, &p2, b) == 0);
  REQUIRE(pull(q, &pulled, z, -1) == 0);
  REQUIRE(memcmp(&pulled, &p3, sizeof(struct scnp_packet)) == 0);
  REQUIRE(memcmp(z, c, 6) == 0);
  REQUIRE(q->head != NULL);
  REQUIRE(q->tail != NULL);
  REQUIRE(memcmp(q->head->packet, &p, sizeof(struct scnp_packet)) == 0);
  REQUIRE(memcmp(q->head->addr, a, 6) == 0);
  REQUIRE(memcmp(q->tail->packet, &p2, sizeof(struct scnp_packet)) == 0);
  REQUIRE(memcmp(q->tail->addr, b, 6) == 0);
  REQUIRE(q->head->next == q->tail);
  REQUIRE(q->head->prev == NULL);
  REQUIRE(q->tail->next == NULL);
  REQUIRE(q->tail->prev == q->head);
  free_queue(q);
}

TEST_CASE("scnp_session") {
  REQUIRE(scnp_start(42, nullptr) == -1);
  REQUIRE(scnp_start(LOOP_INDEX, nullptr) == 0);
  REQUIRE(scnp_start(LOOP_INDEX, nullptr) == -1);
  sleep(2);
  scnp_stop();
  REQUIRE(scnp_start(LOOP_INDEX, nullptr) == 0);
  scnp_stop();
}

TEST_CASE("scnp_packets") {
  REQUIRE(scnp_start(LOOP_INDEX, nullptr) == 0);
  int fd = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_SCNP));
  REQUIRE(fd > 0);
  uint8_t loopaddr[] = { 0, 0, 0, 0, 0, 0 };
  socklen_t addrlen = sizeof(struct sockaddr_ll);
  struct sockaddr_ll addr{};
  memset(&addr, 0, addrlen);
  addr.sll_family = AF_PACKET;
  addr.sll_protocol = htons(ETH_P_SCNP);
  addr.sll_ifindex = LOOP_INDEX;
  memcpy(addr.sll_addr, loopaddr, ETHER_ADDR_LEN);
  addr.sll_halen = ETHER_ADDR_LEN;

  int b;
  uint8_t addr_r[] = { 1, 1, 1, 1, 1, 1 };
  auto * packet = static_cast<scnp_packet *>(malloc(sizeof(struct scnp_packet)));
  REQUIRE(packet != NULL);

  /* scnp_management */
  memset(packet, 0, sizeof(struct scnp_packet));
  REQUIRE(scnp_recv(packet, addr_r) == 0);

  auto * mng = reinterpret_cast<scnp_management *>(packet);
  REQUIRE(mng->type == SCNP_MNG);
  char hostname[HOSTNAME_LENGTH];
  memset(hostname, 0, HOSTNAME_LENGTH);
  gethostname(hostname, HOSTNAME_LENGTH);
  hostname[HOSTNAME_LENGTH - 1] = 0;
  REQUIRE(memcmp(mng->hostname, hostname, HOSTNAME_LENGTH) == 0);

  /* scnp_out */
  uint8_t out_buf[] = { SCNP_OUT, 0x12, 0x34, 0x56, 0x78, 0x80, 0x99, 0x99 };

  b = sendto(fd, out_buf, OUT_LENGTH, 0, (struct sockaddr *) &addr, addrlen);
  REQUIRE(b == OUT_LENGTH);

  memset(packet, 0, sizeof(struct scnp_packet));
  memset(addr_r, 1, ETHER_ADDR_LEN);
  while (packet->type != SCNP_OUT) {
    REQUIRE(scnp_recv(packet, addr_r) == 0);
  }

  REQUIRE(memcmp(addr_r, loopaddr, ETHER_ADDR_LEN) == 0);
  auto * out = reinterpret_cast<scnp_out *>(packet);
  REQUIRE(out->type == SCNP_OUT);
  REQUIRE(out->id == 0x12345678);
  REQUIRE(out->direction);
  REQUIRE(!out->side);
  REQUIRE(out->height == 0.6f);

  /* scnp_movement */
  uint8_t mov_buf[] = { SCNP_MOV, MOV_REL << 7u, 0x12, 0x34, 0x56, 0x78, 0x90, 0x12 };

  b = sendto(fd, mov_buf, MOV_LENGTH, 0, (struct sockaddr *) &addr, addrlen);
  REQUIRE(b == MOV_LENGTH);

  memset(packet, 0, sizeof(struct scnp_packet));
  memset(addr_r, 1, ETHER_ADDR_LEN);
  while (packet->type != SCNP_MOV) {
    REQUIRE(scnp_recv(packet, addr_r) == 0);
  }

  REQUIRE(memcmp(addr_r, loopaddr, ETHER_ADDR_LEN) == 0);
  auto * mov = reinterpret_cast<scnp_movement *>(packet);
  REQUIRE(mov->type == SCNP_MOV);
  REQUIRE(mov->move_type == MOV_REL);
  REQUIRE(mov->code == 0x1234);
  REQUIRE(mov->value == 0x56789012);

  /* scnp_key */
  uint8_t key_buf[] = { SCNP_KEY, 0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0xc0 };

  b = sendto(fd, key_buf, KEY_LENGTH, 0, (struct sockaddr *) &addr, addrlen);
  REQUIRE(b == KEY_LENGTH);

  memset(packet, 0, sizeof(struct scnp_packet));
  memset(addr_r, 1, ETHER_ADDR_LEN);
  while (packet->type != SCNP_KEY) {
    REQUIRE(scnp_recv(packet, addr_r) == 0);
  }

  REQUIRE(memcmp(addr_r, loopaddr, ETHER_ADDR_LEN) == 0);
  auto * key = reinterpret_cast<scnp_key *>(packet);
  REQUIRE(key->type == SCNP_KEY);
  REQUIRE(key->id == 0x12345678);
  REQUIRE(key->code == 0x9012);
  REQUIRE(key->pressed);
  REQUIRE(key->repeated);

  free(packet);
  close(fd);
  scnp_stop();
}

void recv_and_ack()
{
  struct scnp_packet packet{};
  packet.type = 0;
  uint8_t addr[ETHER_ADDR_LEN];
  while (packet.type != SCNP_KEY) {
    scnp_recv(&packet, addr);
  }
}

TEST_CASE("scnp_ack") {
  REQUIRE(scnp_start(LOOP_INDEX, nullptr) == 0);
  std::thread t(recv_and_ack);
  struct scnp_key key = { SCNP_KEY, 0, 0xabcd, true, true };
  uint8_t loopaddr[] = { 0, 0, 0, 0, 0, 0 };
  REQUIRE(scnp_send((struct scnp_packet *) &key, loopaddr) == 0);
  t.join();
  REQUIRE(scnp_send((struct scnp_packet *) &key, loopaddr) == -1);
  scnp_stop();
}

void send_encrypted()
{
  struct scnp_key key = {SCNP_KEY, 0, 0xabcd, true, true};
  uint8_t loopaddr[] = { 0, 0, 0, 0, 0, 0 };
  scnp_send(reinterpret_cast<scnp_packet *>(&key), loopaddr);
}

TEST_CASE("crypto") {
  REQUIRE(scnp_start(LOOP_INDEX, "test") == 0);

  auto * packet = static_cast<scnp_packet *>(malloc(sizeof(struct scnp_packet)));
  REQUIRE(packet != NULL);
  packet->type = 0;
  uint8_t addr[ETHER_ADDR_LEN];

  std::thread t(send_encrypted);
  int i = 0;
  while (i++ < 3 && packet->type != SCNP_KEY) {
    REQUIRE(scnp_recv(packet, addr) == 0);
  }
  t.join();
  auto * key = reinterpret_cast<scnp_key *>(packet);
  REQUIRE(key->code == 0xabcd);

  free(packet);
  scnp_stop();
}
