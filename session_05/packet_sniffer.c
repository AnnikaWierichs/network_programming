// BASIC WIRESHARK COMMAND LINE VERSION

// Example: Ether header | IP header | TCP header | Data

// ip address sender, receiver / length / counter / time


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netdb.h>

#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <netinet/ip_icmp.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void ProcessPacket(unsigned char*, int);


int main()
{
	int saddr_size , data_size;
	struct sockaddr saddr;

	unsigned char *buffer = (unsigned char *) malloc(65536); //Its Big!

    // Create raw socket receiving via all protocols (ETH_P_ALL), not just IP.
	int sock_raw = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(sock_raw < 0) {
		perror("socket(): Error creating socket.\n");
	}

    // Set promiscuous receive mode (man 7 packet -> Socket options)
    struct ifreq ifr;                           // needed for packet_mreq
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);

    struct packet_mreq mr;                      // needed for setsockopt
    mr.mr_ifindex = ifr.ifr_ifindex;
    mr.mr_type = PACKET_MR_PROMISC;             // promiscuous
	setsockopt(sock_raw, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr));

	// Listen and receive packets
    while(1)
	{
		saddr_size = sizeof saddr;

		// Receive a packet
		data_size = recvfrom(sock_raw, buffer, 65536, 0, &saddr,
                             (socklen_t*) &saddr_size);

		if(data_size <0 )
		{
			perror("recvfrom(): Error receiving.\n");
			return 1;
		}

		ProcessPacket(buffer, data_size);
	}

	close(sock_raw);
	return 0;
}


void ProcessPacket(unsigned char* buffer, int size) {
    // Ethernet header
    struct ethhdr* eth_header = (struct ethhdr*) (buffer);

    int eth_protocol = ntohs(eth_header->h_proto);
    printf("Ether header: ");
    switch (eth_protocol) {
    case 0x0800:
        printf("IPv4\t\t");
        break;
    case 0x0004:
        printf("802.2\t");
        break;
    case 0x0806:
        printf("ARP\t\t");
        break;
    case 0x8035:
        printf("RARP\t");
        break;
    case 0x8138:
        printf("Novell\t");
        break;
    case 0x86DD:
        printf("IPv6\t");
        break;
    default:
        printf("Other\t");
    }

    // IP header (in case IP is used)
    if (eth_protocol == 0x0800) {
        struct iphdr* ip_header = (struct iphdr*) (buffer + sizeof(struct ethhdr));

        printf("IPv4 header: ");
        switch (ip_header->protocol) {
        case 1:
            printf("ICMP");
            break;
        case 2:
            printf("IGMP");
            break;
        case 6:
            printf("TCP ");
            break;
        case 17:
            printf("UDP ");
            break;
        default:
            printf("Other");
        }
    }

    printf("\n");
}
