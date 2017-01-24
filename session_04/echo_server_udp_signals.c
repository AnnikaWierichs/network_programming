// ECHO SERVER USING UDP / SIGNALS

#include <stdio.h> 
#include <string.h> 

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>


void io_handler(int);


// Buffer to write received string to
unsigned int BUFFER_SIZE = 256;
int s = 0;


int main() {
    // Create and define sockaddr_in to store server properties
    struct sockaddr_in server_address;
    int port_num = 1025;    // random port number > 1024

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_num);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    // Initialize and bind socket (DGRAM: UDP / non-connection-mode socket)
    printf("Creating socket.\n");
    s = socket(AF_INET, SOCK_DGRAM, 0);

    int optval = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    // Bind socket.
    printf("Binding socket.\n");
    int succ_bind = bind(s, (struct sockaddr*) &server_address, sizeof server_address);
    if (succ_bind == -1)
        perror("Bind not successful.\n");

    // Specify signal disposition and function to be called (io_handler()).
    signal(SIGIO, io_handler);

    // Set process ID that receives signals to own ID.
    int succ_fcntl_own = fcntl(s, F_SETOWN, getpid());
    if (succ_fcntl_own == -1)
        perror("fcntl() F_SETOWN) not successful.\n");

    // Allow socket to receive asynchronous I/O signals.
    int succ_fcntl_async = fcntl(s, F_SETFL, O_ASYNC);
    if (succ_fcntl_async == -1)
        perror("fcntl() (ASYNC) not successful.\n");

    while(1);   // Wait for any signals.

    return 0;
}
    

void io_handler(int sig) {
    // Create sockaddr to store client properties later when receiving from client 
    struct sockaddr client_address;
    socklen_t client_addr_len = sizeof client_address;
    int num_bytes_received;

    int str_size = BUFFER_SIZE;     
    char buffer[BUFFER_SIZE];       
    
    // Receive data from client and store client address.
    num_bytes_received = recvfrom(s, buffer, str_size, 0, &client_address,
                                  &client_addr_len);
    if (num_bytes_received == -1)
        perror("recvfrom(): An error occured.\n");

    // Print message received from client.
    printf("\nData sent from client:\n%s \n", buffer);

    // Send received string back to the client
    int succ_sendto = sendto(s, buffer, strlen(buffer)+1, 0, &client_address, 
                             client_addr_len);  
    if (succ_sendto == -1)
        perror("sendto(): An error occured.\n");

    return;
}

