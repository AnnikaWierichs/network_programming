// CLIENT

#include <stdio.h> 
#include <string.h> 

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/sctp.h>
#include <netdb.h>
#include <unistd.h>


unsigned int BUFFER_SIZE = 256;
unsigned int NUM_STREAMS = 3;

int main() {
    // Create and define sockaddr_in to save server details
    struct sockaddr_in server_address;
    int port_num = 1025;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_num);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");    // localhost
    
    struct sctp_initmsg initmsg;

    struct sctp_event_subscribe events;
    bzero(&events, sizeof events);
    events.sctp_data_io_event = 1;

    // Create sockaddr to store server properties receiving a response from
    // later
    struct sockaddr server_rec_address;
    socklen_t server_rec_len;

    // Create buffer to store a string input by user
    char buffer[BUFFER_SIZE];
    char buffer_response[BUFFER_SIZE];

    struct sctp_sndrcvinfo srinfo;
    bzero(&srinfo, sizeof srinfo);
    int flags = 0;

    // Ask user to input string
    printf("Input data to send: \n");
    int succ_scanf = scanf("%255s", buffer);
    if (succ_scanf == 0)
        perror("scanf(): Failed to read input.\n");

    // Initialize socket
    printf("Initializing socket.\n");
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
    if (s < 0)
        perror("socket(): Error received.\n");

    setsockopt(s, IPPROTO_SCTP, SCTP_EVENTS, &events, sizeof events);
    
    memset(&initmsg, 0, sizeof initmsg);
    initmsg.sinit_num_ostreams = 3;
    initmsg.sinit_max_instreams = 3;
    initmsg.sinit_max_attempts = 2;
    setsockopt(s, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof initmsg);

    // Connect socket
    printf("Connecting.\n");
    int succ_conn = connect(s, (struct sockaddr*) &server_address,
                            sizeof(server_address));
    if (succ_conn < 0)
        perror("connect(): Error received.\n");
    
    int stream;
    for (stream = 0; stream < NUM_STREAMS; stream++) {
        srinfo.sinfo_stream = stream;

        // Send buffer containing string to server
        printf("Sending message.\n"); 
        int succ_send = sctp_sendmsg(s, (void*) buffer, BUFFER_SIZE, (struct
                                     sockaddr*) &server_address,
                                     sizeof(server_address), 0, 0,
                                     srinfo.sinfo_stream, 10000, 0);
        if (succ_send < 0)
            printf("sctp_sendmsg(): Error sending data.\n");

        // Receive and print response from server
        server_rec_len = sizeof(server_rec_address);
        printf("Receiving response from server.\n");
        sctp_recvmsg(s, buffer_response, BUFFER_SIZE, (struct sockaddr*)
                     &server_rec_address, &server_rec_len, &srinfo, &flags);
        printf("Print out of data received from server: %255s \n", buffer_response);
    }

    close(s);
}
    

