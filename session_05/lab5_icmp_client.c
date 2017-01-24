#include <sys/types.h> // for things like AF_INET, SOCK_DGRAM
#include <sys/socket.h> // for socket()
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <bits/ioctls.h>

#define IP4_HDRLEN 20
#define ICMP_HDRLEN 8 //No options for now

uint16_t checksum (uint16_t *addr, int len)
{
	int count = len;
	register uint32_t sum = 0;
	uint16_t answer = 0;

	// Sum up 2-byte values until none or only one byte left.
	while (count > 1) {
		sum += *(addr++);
		count -= 2;
	}

	// Add left-over byte, if any.
	if (count > 0) {
		sum += *(uint8_t *) addr;
	}

	// Fold 32-bit sum into 16 bits; we lose information by doing this,
	// increasing the chances of a collision.
	// sum = (lower 16 bits) + (upper 16 bits shifted right 16 bits)
	while (sum >> 16) {
		sum = (sum & 0xffff) + (sum >> 16);
	}

	// Checksum is one's compliment of sum.
	answer = ~sum;

	return (answer);
}

void main(){
	char src_ip_char[100];
	char dst_ip_char[100];
	struct ip iphdr;
	struct icmp icmphdr;
	uint32_t src_ip;
	uint32_t dst_ip;
	struct sockaddr_in src_in, client_addr;
	struct ifreq ifr;
	int ip_flags[4];

	int tmp_skt =  socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if(tmp_skt < 0)
			perror("Can't open socket");
	char * if_name = "eth0";
	struct ifreq if_req;
	memset (&ifr, 0, sizeof (ifr));
	snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", if_name);
	if (ioctl (tmp_skt, SIOCGIFINDEX, &ifr) < 0)
		perror ("ioctl() failed to find interface ");
	close(tmp_skt);

	printf("Type the IPv4 address of client. Press only Enter for using 192.168.88.129\n");
	fgets(src_ip_char, sizeof(src_ip_char), stdin);
	if(src_ip_char[0] == '\n')
		strcpy(src_ip_char, "192.168.88.129");

	printf("Type the IPv4 address of server. Press only Enter for using 8.8.8.8\n");
	fgets(dst_ip_char, sizeof(dst_ip_char), stdin);
	if(dst_ip_char[0] == '\n')
		strcpy(dst_ip_char, "8.8.8.8");

	int datalen = 4;

	int final_data_len = IP4_HDRLEN + ICMP_HDRLEN + datalen;

	uint8_t * final_result = malloc(final_data_len);
	uint8_t * reply_rcvd = malloc(final_data_len);
	if(final_result == NULL || reply_rcvd == NULL)
		perror("Memory not reserved");

	char data[4] = "test";

	iphdr.ip_hl = IP4_HDRLEN / sizeof(uint32_t);
	iphdr.ip_v = 4;
	iphdr.ip_tos = 0;
	iphdr.ip_len = htons (IP4_HDRLEN + ICMP_HDRLEN + datalen);
	iphdr.ip_id = htons (0);
	ip_flags[0] = 0;
	ip_flags[1] = 0;
	ip_flags[2] = 0;
	ip_flags[3] = 0;
	iphdr.ip_off = htons ((ip_flags[0] << 15) + (ip_flags[1] << 14)+ (ip_flags[2] << 13)+  ip_flags[3]);
	iphdr.ip_ttl = 255;
	iphdr.ip_p = IPPROTO_ICMP;
	inet_pton(AF_INET, src_ip_char, &iphdr.ip_src);
	inet_pton(AF_INET, dst_ip_char, &iphdr.ip_dst);
	iphdr.ip_sum = 0;
	iphdr.ip_sum = checksum((uint16_t *) &iphdr, IP4_HDRLEN);

	icmphdr.icmp_type = ICMP_ECHO;
	icmphdr.icmp_code = 0;
	icmphdr.icmp_id = htons (1234);
	icmphdr.icmp_seq = htons (0);
	icmphdr.icmp_cksum = 0;

	memcpy (final_result, &iphdr, IP4_HDRLEN);
	memcpy ((final_result + IP4_HDRLEN), &icmphdr, ICMP_HDRLEN);
	memcpy (final_result + IP4_HDRLEN + ICMP_HDRLEN, data, datalen);
	icmphdr.icmp_cksum = checksum ((uint16_t *) (final_result + IP4_HDRLEN), ICMP_HDRLEN + datalen);
	memcpy ((final_result + IP4_HDRLEN), &icmphdr, ICMP_HDRLEN);

	/*
	for(int temp_cnt = 0; temp_cnt < final_data_len; temp_cnt++) {
		printf("%02x ", final_result[temp_cnt]);
	}
	*/


	src_in.sin_family = AF_INET;
	src_in.sin_addr.s_addr = src_ip;

	int skt_raw =  socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if(skt_raw < 0)
			perror("Can't open socket");

	int on = 1;
	int opt_ret = setsockopt (skt_raw, IPPROTO_IP, IP_HDRINCL, &on, sizeof (on));
	if(opt_ret)
		perror("opt_ret error");

	opt_ret = setsockopt (skt_raw, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof (ifr));
	if(opt_ret)
		perror("opt_ret error");

	int b = sizeof(client_addr);
	int recv_ret = 0;

	char remote_ip_reply[100];
	struct icmp * icmp_reply;
	struct ip * iphdr_reply;

	while(1) {
		int sendto_ret = sendto(skt_raw, final_result, IP4_HDRLEN + ICMP_HDRLEN + datalen, 0, (struct sockaddr *) &src_in, sizeof (struct sockaddr));
		if(sendto_ret < 0)
			perror ("sendto failed ");

		sleep(1);

		int recv_ret = recvfrom(skt_raw, reply_rcvd, final_data_len, 0, (struct sockaddr *) &client_addr, &b);
		if(recv_ret < 0)
			perror ("recvfrom failed ");

		if(recv_ret) {

			iphdr_reply = (struct ip *) reply_rcvd;
			icmp_reply = (struct icmp *) (iphdr_reply + 1);

			inet_ntop(AF_INET, &(iphdr_reply->ip_src), remote_ip_reply, 100);
			printf("%d bytes from %s: icmp_seq=%d ttl=NA time=NA ms\n", recv_ret, remote_ip_reply, icmp_reply->icmp_seq);

			/*for(int temp_cnt = 0; temp_cnt < recv_ret; temp_cnt++) {
				printf("%02x ", reply_rcvd[temp_cnt]);
			}
			printf("\n");*/
		}
	}

	return;
}
