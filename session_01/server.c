// SCTP SERVER

#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

unsigned int BUFFER_SIZE = 256


int main() {
    // Create and define sockaddr_in to store server properties
    struct sockaddr_in server_address;
    int port_num = 1025;    // random port number > 1024

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_num);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    // Create sockaddr to store client properties later when receiving from client
    struct sockaddr client_address;
    socklen_t client_addr_len;

    // Buffer to write received string to
    char buffer[BUFFER_SIZE];
    int str_size = BUFFER_SIZE;

    // Store recvmsg information
    struct sctp_sndrcvinfo srinfo;
    int flags;

    // Initialize and bind socket (DGRAM: non-connection-mode socket)
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
    if (s < 0)
        perror("socket(): Error received.\n");

    int succ_bind = bind(s, (struct sockaddr*) &server_address, sizeof server_address);
    if (succ_bind < 0)
        printf("bind(): Bind not successful.\n");

    int succ_listen = listen(s, 1);
    if (succ_listen < 0)
        printf("listen(): Listen not successful.\n");

    while() {
        flags = 0;

        recv_num_bytes = sctp_recvmsg(s, buffer, BUFFER_SIZE, (struct
                sockaddr*) &client_address, &client_addr_len, &srinfo, &flags);
        if (recv_num_bytes < 0)
            printf("recvmsg(): Transmission not successful.\n");
            break;
        else {
            printf("Received from %s:%u:\n%s\n\n", inet_ntoa(addr.sin_addr),
                   ntohs(addr.sin_port), (char*) buffer);

            if (0 == strcmp(buffer, "quit"))
                break;
        }

    }

    printf("Closing socket.\n");
    close(s);


    // Send received string back to the client
    int ret_msg = sendto(s, buffer, strlen(buffer)+1, 0, &client_address, client_addr_len);
    printf("%d", ret_msg);
}

