// SCTP SERVER
//
// Action points:
// - Update the project description
// - Implement SCTP server client communication using 3 streams in parallel
// - Check the implementation by blocking the first stream for a while
// - Implement the SCTP streaming feature using the original TCP/UDP send
//   function
// ---> Keyboard Interrupt machen, kann auf stream 1 gesendet werden, ansonsten
// wird auf stream 2 immer text gesendet. signals nutzen?!
// ausserdem auch fuer server den loop implementieren.
//  

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>


unsigned int BUFFER_SIZE = 256;
unsigned int NUM_STREAMS = 10;


int main() {
    // Create and define sockaddr_in to store server properties
    struct sockaddr_in server_address;
    int port_num = 1025;    // random port number > 1024

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_num);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    struct sctp_event_subscribe events;

    struct sctp_initmsg initmsg;

    // Create sockaddr to store client properties later when receiving from client
    struct sockaddr_in client_address;
    socklen_t client_addr_len;

    // Buffer to write received string to
    char buffer[BUFFER_SIZE];
    char buffer_response[BUFFER_SIZE];

    // Store recvmsg information
    struct sctp_sndrcvinfo srinfo;
    int flags;

    // Initialize and bind socket (DGRAM: non-connection-mode socket)
    printf("Initializing socket.\n");
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
    if (s < 0)
        perror("socket(): Error received.\n");

    printf("Binding socket.\n");
    int succ_bind = bind(s, (struct sockaddr*) &server_address, sizeof server_address);
    if (succ_bind < 0)
        printf("bind(): Bind not successful.\n");

    bzero(&events, sizeof events);
    events.sctp_data_io_event = 1;
    setsockopt(s, IPPROTO_SCTP, SCTP_EVENTS, &events, sizeof events);

    memset(&initmsg, 0, sizeof initmsg);
    initmsg.sinit_num_ostreams = 3;
    initmsg.sinit_max_instreams = 3;
    initmsg.sinit_max_attempts = 2;
    setsockopt(s, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof initmsg);

    printf("Listening.\n");
    int succ_listen = listen(s, 1);
    if (succ_listen < 0)
        printf("listen(): Listen not successful.\n");

    printf("Entering while Loop.\n");
    while(1) {
        flags = 0;

        // Wait for message from a client (blocking)
        client_addr_len = sizeof client_address;
        int recv_num_bytes = sctp_recvmsg(s, buffer, BUFFER_SIZE, (struct
                sockaddr*) &client_address, &client_addr_len, &srinfo, &flags);
        if (recv_num_bytes < 0) {
            printf("recvmsg(): Transmission not successful.\n");
        }

        else {
            printf("Received from %s:%u, stream no. %d:\n%s\n\n",
                   inet_ntoa(server_address.sin_addr),
                   ntohs(server_address.sin_port), srinfo.sinfo_stream,
                   (char*) buffer);
            printf("Client address: %s\n", inet_ntoa(client_address.sin_addr));

            sleep(2);

            // Send buffer containing string to server
            strcpy(buffer_response, "Received the message.\n");
            printf("Buffer Response: %s\n", buffer_response); 
            printf("Sending confirmation back to client.\n\n\n");
            int succ_send = sctp_sendmsg(s, (void*) buffer_response,
                    BUFFER_SIZE, (struct sockaddr*) &client_address,
                    sizeof client_address, 0, 0, 1, 10000, 0); 
            if (succ_send < 0)
                printf("sctp_sendmsg(): Transmission to client not successful.\n");

            if (0 == strcmp(buffer, "quit"))
                break;
        }
    }

    printf("Closing socket.\n");
    close(s);
}
