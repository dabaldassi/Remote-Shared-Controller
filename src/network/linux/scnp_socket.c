#include <string.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>

#include <scnp_socket.h>

int scnp_socket_open(struct scnp_socket* socket, int if_index)
{
    socket->fd = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_SCNP));
    if (socket->fd < 0) {
        return -1;
    }

    socket->if_index = if_index;

    /* bind the socket to the interface */
    struct sockaddr_ll addr;
    socklen_t addrlen = sizeof(struct sockaddr_ll);
    memset(&addr, 0, addrlen);
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_SCNP);
    addr.sll_ifindex = (int)if_index;
    if (bind(thread_info.socket, (struct sockaddr*) & addr, addrlen)) {
        return -1;
    }

    return 0;
}

int scnp_socket_close(struct scnp_socket* socket)
{
    close(socket->fd);
    socket->fd = -1;
    socket->if_index = 0;
}

bool scnp_socket_opened(struct scnp_socket* socket)
{
    return socket->fd > 0;
}

ssize_t scnp_socket_recvfrom(struct scnp_socket* socket, void* buf, size_t len, int flags, uint8_t* src_addr)
{
    socklen_t          addrlen = sizeof(struct sockaddr_ll);
    struct sockaddr_ll addr;

    memset(&addr, 0, addrlen);
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_SCNP);
    addr.sll_ifindex = socket->if_index;

    ssize_t ret = recvfrom(socket->fd, buf, packetlen, flags, (struct sockaddr*) &addr, &addrlen);

    if (ret == -1) return -1;

    memcpy(src_addr, addr, ETHER_ADDR_LEN);

    return ret;
}

ssize_t scnp_socket_sendto(struct scnp_socket* socket, const void* buf, size_t len, int flags, const uint8_t* dest_addr)
{
    socklen_t          addrlen = sizeof(struct sockaddr_ll);
    struct sockaddr_ll addr;

    memset(&addr, 0, addrlen);
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_SCNP);
    addr.sll_ifindex = socket->if_index;
    addr.sll_halen = ETHER_ADDR_LEN;
    memcpy(addr.sll_addr, dest_addr, ETHER_ADDR_LEN);

    return sendto(socket->fd, waste.buf, buf_length, flags, (struct sockaddr*) & addr, addrlen);
}