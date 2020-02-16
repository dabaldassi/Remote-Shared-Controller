#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <interface.h>
#include <scnp_socket.h>

int scnp_socket_open(struct scnp_socket* socket, int if_index)
{
	char pcap_if_prefix[100] = "rpcap://\\Device\\NPF_";
    char errbuf[PCAP_ERRBUF_SIZE];

	IF* interfaces = get_interfaces();
	IF* i = NULL;

	for (i = interfaces; i->if_index != if_index; ++i);

	char * pcap_name = strcat(pcap_if_prefix, i->if_win_name);
    memcpy(socket->src_addr, i->if_addr, ETHER_ADDR_LEN);

	free_interfaces(interfaces);

    socket->fp = pcap_open(pcap_name,
                           100 /*snaplen*/,
                           PCAP_OPENFLAG_PROMISCUOUS /*flags*/,
                           -1 /*read timeout*/,
                           NULL /* remote authentication */,
                           errbuf);

    if(socket->fp == NULL) {
        fprintf(stderr, "\nError opening source: %s\n", errbuf);
        return -1;
    }

    socket->if_index = if_index;

    return 0;
}

int scnp_socket_close(struct scnp_socket* socket)
{
    pcap_close(socket->fp);
    socket->fp = NULL;
    socket->if_index = 0;

    return 0;
}

bool scnp_socket_opened(struct scnp_socket* socket)
{
	return socket->fp != NULL;
}

ssize_t scnp_socket_recvfrom(struct scnp_socket* socket, void* buf, size_t len, int flags, uint8_t* src_addr)
{
    const size_t len_header = 2 * ETHER_ADDR_LEN + 2; // dest and src addr + ether type
    struct pcap_pkthdr* header;
    u_char* pkt_data;
    uint16_t ethtype = 0;

    while (ethtype != ETH_P_SCNP) {
        int res = pcap_next_ex(socket->fp, &header, &pkt_data);
        if (res <= 0) return -1;
        memcpy(&ethtype, pkt_data + 2 * ETHER_ADDR_LEN, sizeof(ethtype));
    }

    size_t payload_len = min(len, header->caplen - len_header);

    memcpy(src_addr, pkt_data + ETHER_ADDR_LEN, ETHER_ADDR_LEN);
    memcpy(buf, pkt_data + len_header, payload_len);

    return payload_len;
}

ssize_t scnp_socket_sendto(struct scnp_socket* socket, const void* buf, size_t len, int flags, const uint8_t* dest_addr)
{
    const size_t len_header = 2 * ETHER_ADDR_LEN + 2; // dest and src addr + ether type
    const uint16_t ethtype = ETH_P_SCNP;

    u_char* packet;

    packet = calloc((len_header + len), sizeof(u_char));
    if (packet == NULL) return -1;

    /* Fill the header */
    memcpy(packet, dest_addr, ETHER_ADDR_LEN);
    memcpy(packet + ETHER_ADDR_LEN, socket->src_addr, ETHER_ADDR_LEN);
    memcpy(packet + 2 * ETHER_ADDR_LEN, &ethtype, sizeof(ethtype));

    /* Fill the payload */

    memcpy(packet + len_header, buf, len);

    /* Send the packet */

    int ret = pcap_sendpacket(socket->fp, packet, (int)(len_header + len));
    free(packet);

    if (ret == 0) ret = (int)len; // The number of bytes sent

    return ret;
}