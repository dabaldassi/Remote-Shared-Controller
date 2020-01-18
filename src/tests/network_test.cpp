#include "catch/catch.hpp"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <unistd.h>
#include <netinet/in.h>

#include "scnp.h"

#define LOOP_INDEX 1

TEST_CASE("scnp_socket") {
  int error_test;
  int if_index = 1; // loopback index
  struct scnp_socket sock{};

  error_test = scnp_create_socket(&sock, if_index);
  if (error_test == -1) perror("Cannot create socket");
  REQUIRE_FALSE(error_test == -1);

  REQUIRE(sock.if_index == if_index);
  REQUIRE(sock.fd >= 0);

  scnp_close_socket(&sock);
}

int init_recv_socket()
{
  /* create socket */
  int fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_SCNP));
  if (fd == -1) perror("Cannot create receiving socket");
  else {
    /* set acknowledgment timeout */
    struct timeval timeout = {1, 0};
    int error_test = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    if (error_test) perror("Cannot set timeout on receiving socket");
    else {
      /* bind receiving socket to loopback interface */
      struct sockaddr_ll addr{};
      socklen_t addrlen = sizeof(sockaddr_ll);
      memset(&addr, 0, addrlen);
      addr.sll_family = AF_PACKET;
      addr.sll_protocol = htons(ETH_P_SCNP);
      addr.sll_ifindex = LOOP_INDEX;
      error_test = bind(fd, (struct sockaddr *) &addr, addrlen);
      if (error_test) perror("Cannot bind receiving socket to the loopback interface");
    }

    if (error_test) {
      if (close(fd)) perror("Error while closing socket");
      fd = -1;
    }
  }

  return fd;
}

TEST_CASE("scnp_send") {
  /* initialize receiving socket */
  int recv_sock = init_recv_socket();
  REQUIRE(recv_sock >= 0);

  /* create sending socket */
  struct scnp_socket send_sock{};
  int error_test = scnp_create_socket(&send_sock, LOOP_INDEX);
  if (error_test) perror("Cannot create socket");
  REQUIRE_FALSE(error_test);

  uint8_t loopaddr[] = {0, 0, 0, 0, 0, 0};
  uint8_t broadcastaddr[] = {255, 255, 255, 255, 255, 255};
  uint16_t proto = htons(ETH_P_SCNP);

  SECTION("scnp_ack") {
    /* send ack */
    struct scnp_ack ack = {SCNP_ACK};
    ssize_t bytes_sent = scnp_send (&send_sock, (struct scnp_packet *) &ack, loopaddr);
    if (bytes_sent == 0) perror("Cannot send acknowledgement");
    REQUIRE(bytes_sent == ACK_LENGTH);

    /* receive ack */
    uint8_t buf[MAX_PACKET_LENGTH + ETHER_HDR_LEN];
    struct sockaddr srcaddr{};
    socklen_t srcaddr_len = sizeof(struct sockaddr);
    memset(&srcaddr, 0, srcaddr_len);
    ssize_t bytes_recv = recvfrom(recv_sock, buf, sizeof(buf), 0, &srcaddr, &srcaddr_len);
    if (bytes_recv == -1) perror("Cannot receive acknowledgement");
    REQUIRE(bytes_recv == ETHER_HDR_LEN + ACK_LENGTH);

    /* verify buffer */
    uint8_t * test = buf;
    REQUIRE_FALSE(memcmp(test, loopaddr, ETHER_ADDR_LEN));
    REQUIRE_FALSE(memcmp(test += ETHER_ADDR_LEN, loopaddr, ETHER_ADDR_LEN));
    REQUIRE_FALSE(memcmp(test += ETHER_ADDR_LEN, &proto, ETHER_TYPE_LEN));
    REQUIRE_FALSE(memcmp(test += ETHER_TYPE_LEN, &ack.type, sizeof(ack.type)));
  }

  SECTION("scnp_management") {
    /* send mng */
    struct scnp_management mng = {SCNP_MNG, "test"};
    ssize_t bytes_sent = scnp_send (&send_sock, (struct scnp_packet *) &mng, broadcastaddr);
    if (bytes_sent == 0) perror("Cannot send management");
    REQUIRE(bytes_sent == MNG_LENGTH);

    /* receive mng */
    uint8_t buf[MAX_PACKET_LENGTH + ETHER_HDR_LEN];
    struct sockaddr srcaddr{};
    socklen_t srcaddr_len = sizeof(struct sockaddr);
    memset(&srcaddr, 0, srcaddr_len);
    ssize_t bytes_recv = recvfrom(recv_sock, buf, sizeof(buf), 0, &srcaddr, &srcaddr_len);
    if (bytes_recv == -1) perror("Cannot receive management");
    REQUIRE(bytes_recv == ETHER_HDR_LEN + MNG_LENGTH);

    /* verify buffer */
    uint8_t * test = buf;
    REQUIRE_FALSE(memcmp(test, broadcastaddr, ETHER_ADDR_LEN));
    REQUIRE_FALSE(memcmp(test += ETHER_ADDR_LEN, loopaddr, ETHER_ADDR_LEN));
    REQUIRE_FALSE(memcmp(test += ETHER_ADDR_LEN, &proto, ETHER_TYPE_LEN));
    REQUIRE_FALSE(memcmp(test += ETHER_TYPE_LEN, &mng.type, sizeof(mng.type)));
    size_t n = strlen(mng.hostname) + 1;
    REQUIRE_FALSE(memcmp(test += sizeof(mng.type), mng.hostname, n));
    test += n;
    REQUIRE(std::all_of(test, test + HOSTNAME_LENGTH - n, [](uint8_t a) { return a == 0;}));
  }

  SECTION("scnp_out") {
    /* send out */
    struct scnp_out out = {SCNP_OUT, OUT_EGRESS, OUT_LEFT, 0.1f};
    ssize_t bytes_sent = scnp_send (&send_sock, (struct scnp_packet *) &out, loopaddr);
    if (bytes_sent == 0) perror("Cannot send out");
    REQUIRE(bytes_sent == - OUT_LENGTH);

    /* receive out */
    uint8_t buf[MAX_PACKET_LENGTH + ETHER_HDR_LEN];
    struct sockaddr srcaddr{};
    socklen_t srcaddr_len = sizeof(struct sockaddr);
    memset(&srcaddr, 0, srcaddr_len);
    ssize_t bytes_recv = recvfrom(recv_sock, buf, sizeof(buf), 0, &srcaddr, &srcaddr_len);
    if (bytes_recv == -1) perror("Cannot receive out");
    REQUIRE(bytes_recv == ETHER_HDR_LEN + OUT_LENGTH);

    /* verify buffer */
    uint8_t * test = buf;
    REQUIRE_FALSE(memcmp(test, loopaddr, ETHER_ADDR_LEN));
    REQUIRE_FALSE(memcmp(test += ETHER_ADDR_LEN, loopaddr, ETHER_ADDR_LEN));
    REQUIRE_FALSE(memcmp(test += ETHER_ADDR_LEN, &proto, ETHER_TYPE_LEN));
    REQUIRE_FALSE(memcmp(test += ETHER_TYPE_LEN, &out.type, sizeof(out.type)));
    uint8_t data = 0x80;
    REQUIRE_FALSE(memcmp(test += sizeof(out.type), &data, sizeof(data)));
    uint16_t data2 = htons(0x1999);
    REQUIRE_FALSE(memcmp(test += sizeof(data), &data2, sizeof(data2)));
  }

  SECTION("scnp_mov") {
    /* send mov */
    struct scnp_movement mov = {SCNP_MOV, 0x1234, 0x09876543};
    ssize_t bytes_sent = scnp_send (&send_sock, (struct scnp_packet *) &mov, loopaddr);
    if (bytes_sent == 0) perror("Cannot send movement");
    REQUIRE(bytes_sent == MOV_LENGTH);

    /* receive mov */
    uint8_t buf[MAX_PACKET_LENGTH + ETHER_HDR_LEN];
    struct sockaddr srcaddr{};
    socklen_t srcaddr_len = sizeof(struct sockaddr);
    memset(&srcaddr, 0, srcaddr_len);
    ssize_t bytes_recv = recvfrom(recv_sock, buf, sizeof(buf), 0, &srcaddr, &srcaddr_len);
    if (bytes_recv == -1) perror("Cannot receive movement");
    REQUIRE(bytes_recv == ETHER_HDR_LEN + MOV_LENGTH);

    /* verify buffer */
    uint8_t * test = buf;
    REQUIRE_FALSE(memcmp(test, loopaddr, ETHER_ADDR_LEN));
    REQUIRE_FALSE(memcmp(test += ETHER_ADDR_LEN, loopaddr, ETHER_ADDR_LEN));
    REQUIRE_FALSE(memcmp(test += ETHER_ADDR_LEN, &proto, ETHER_TYPE_LEN));
    REQUIRE_FALSE(memcmp(test += ETHER_TYPE_LEN, &mov.type, sizeof(mov.type)));
    mov.code = htons(mov.code);
    REQUIRE_FALSE(memcmp(test += sizeof(mov.type), &mov.code, sizeof(mov.code)));
    mov.value = htonl(mov.value);
    REQUIRE_FALSE(memcmp(test += sizeof(mov.code), &mov.value, sizeof(mov.value)));
  }

  SECTION("scnp_key") {
    /* send key */
    struct scnp_key key = {SCNP_KEY, 0x1234, true};
    ssize_t bytes_sent = scnp_send (&send_sock, (struct scnp_packet *) &key, loopaddr);
    if (bytes_sent == 0) perror("Cannot send key");
    REQUIRE(bytes_sent == - KEY_LENGTH);

    /* receive key */
    uint8_t buf[MAX_PACKET_LENGTH + ETHER_HDR_LEN];
    struct sockaddr srcaddr{};
    socklen_t srcaddr_len = sizeof(struct sockaddr);
    memset(&srcaddr, 0, srcaddr_len);
    ssize_t bytes_recv = recvfrom(recv_sock, buf, sizeof(buf), 0, &srcaddr, &srcaddr_len);
    if (bytes_recv == -1) perror("Cannot receive key");
    REQUIRE(bytes_recv == ETHER_HDR_LEN + KEY_LENGTH);

    /* verify buffer */
    uint8_t * test = buf;
    REQUIRE_FALSE(memcmp(test, loopaddr, ETHER_ADDR_LEN));
    REQUIRE_FALSE(memcmp(test += ETHER_ADDR_LEN, loopaddr, ETHER_ADDR_LEN));
    REQUIRE_FALSE(memcmp(test += ETHER_ADDR_LEN, &proto, ETHER_TYPE_LEN));
    REQUIRE_FALSE(memcmp(test += ETHER_TYPE_LEN, &key.type, sizeof(key.type)));
    key.code = htons(key.code);
    REQUIRE_FALSE(memcmp(test += sizeof(key.type), &key.code, sizeof(key.code)));
    uint8_t data = 0x80;
    REQUIRE_FALSE(memcmp(test += sizeof(key.code), &data, sizeof(data)));
  }

  scnp_close_socket(&send_sock);
  close(recv_sock);
}

TEST_CASE("scnp_recv") {
  REQUIRE(1);
}

TEST_CASE("scnp_session") {
  REQUIRE(1);
}