// CLIENT

#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>

#define PACKET_SIZE 64

unsigned int MAX_IPHDR_LEN = 60;


struct packet {
    struct icmphdr hdr;
    char msg[PACKET_SIZE-sizeof(struct icmphdr)];
};

int pid = -1;


unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum=0;
    unsigned short result;

    for ( sum = 0; len > 1; len -= 2 ) {
        sum += *buf++;
    }

    if ( len == 1 )
        sum += *(unsigned char*)buf;

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;

    return result;
}


void ping(struct sockaddr_in *server_address) {
    printf("ping() started.\n");

    int i, cnt = 1;
    struct packet pckt;

    // Create sockaddr to store server properties receiving a response from
    // later
    struct sockaddr server_rec_address;
    socklen_t server_rec_len = sizeof server_rec_address;

    // Initialize socket (SOCK_RAW: IP)
    int s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (s < 0)
        perror("socket(): Error received.\n");

    struct ip* ip_a;
    struct icmp* icmp_a;

    for (;;) {
        pckt.hdr.type = 8;                                  // Type ICMP_ECHO
        pckt.hdr.un.echo.id = pid;                          // ID / PID
        pckt.hdr.code = 0;

        // Create buffer to store a string input by user
        char buffer_response[sizeof(pckt) + MAX_IPHDR_LEN];

        // Message
        for ( i = 0; i < sizeof(pckt.msg)-1; i++ ) {
            pckt.msg[i] = i;
        }
        pckt.msg[i] = 0;

        pckt.hdr.un.echo.sequence = cnt++;                  // Seq. number
        pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));  // Checksum

        // Send to
        int sendto_succ = sendto(s, &pckt, sizeof(pckt), 0, (struct sockaddr*)
                                 server_address, sizeof *server_address);
        if (sendto_succ < 0)
            perror("sendto(): Error received.\n");

        // Receive
        size_t buffer_size_t = sizeof buffer_response;
        int recv_num_bytes = recvfrom(s, buffer_response, buffer_size_t, 0,
                                     &server_rec_address, &server_rec_len);

        ip_a = (struct ip*) buffer_response;
        icmp_a = (struct icmp*) (buffer_response + sizeof(struct ip));

        // we need num bytes, ip sender, ip receiver, icmp seq no, ttl=64, time
        // ms

        if (recv_num_bytes < 0) {
            perror("recvfrom(): Error received.\n");
        } else {
            printf("%d bytes from %s. IP TTL: %d / ICMP sequence number %d\n",
                    recv_num_bytes, inet_ntoa(server_address->sin_addr),
                    ip_a->ip_ttl, icmp_a->icmp_seq);
        }

        sleep(2);
    }
}   // ping()



int main() {
    printf("main() started.\n");

    // Create and define sockaddr_in to save server details
    int port_num = 3333;
    struct sockaddr_in server_address;
    pid = getpid();

    server_address.sin_family = AF_INET;
    server_address.sin_port = 0;    // should be 0 for sending for raw sockets
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");    // localhost

    ping(&server_address);

    return 0;
}

