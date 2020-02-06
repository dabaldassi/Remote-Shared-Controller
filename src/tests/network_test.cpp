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
  REQUIRE(scnp_start(42) == -1);
  REQUIRE(scnp_start(LOOP_INDEX) == 0);
  REQUIRE(scnp_start(LOOP_INDEX) == -1);
  sleep(2);
  scnp_stop();
  REQUIRE(scnp_start(LOOP_INDEX) == 0);
  scnp_stop();
}
