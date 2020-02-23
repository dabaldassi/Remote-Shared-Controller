#ifndef SCNP_SOCKET_H
#define SCNP_SOCKET_H

#include <stdbool.h>
#include <stdint.h>

/* EtherType */
#define ETH_P_SCNP 0x8888

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __gnu_linux__
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <unistd.h>


    struct scnp_socket
    {
        int fd;
        int if_index;
    };

#else

#include <pcap.h>
#include <winsock2.h>
  
#define ETHER_ADDR_LEN 6

    struct scnp_socket
    {
        pcap_t* fp;
        int     if_index;
        uint8_t src_addr[ETHER_ADDR_LEN];
    };

    typedef long long int ssize_t;

#endif

    int scnp_socket_open(struct scnp_socket * socket, int if_index);
    int scnp_socket_close(struct scnp_socket* socket);

    bool scnp_socket_opened(struct scnp_socket* socket);

    ssize_t scnp_socket_recvfrom(struct scnp_socket* socket, void * buf, size_t len, int flags, uint8_t * src_addr);
    ssize_t scnp_socket_sendto(struct scnp_socket* socket,const void * buf, size_t len, int flags, const uint8_t * dest_addr);

#ifdef __cplusplus
}
#endif

#endif
