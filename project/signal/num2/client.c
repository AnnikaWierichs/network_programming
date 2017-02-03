// CLIENT

#include <stdio.h> 
#include <string.h> 

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>


unsigned int BUFFER_SIZE = 256;

int main() {
    // Create and define sockaddr_in to save server details
    struct sockaddr_in server_address;
    int port_num = 1025;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_num);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");    // localhost

    // Create sockaddr to store server properties receiving a response from
    // later
    struct sockaddr server_rec_address;
    socklen_t server_rec_len;

    // Create buffer to store a string input by user
    char buffer[BUFFER_SIZE];
    char buffer_response[BUFFER_SIZE];
    int str_size = BUFFER_SIZE;
    // Ask user to input string
    printf("Input data to send: \n");
    scanf("%255s", buffer);

    // Initialize socket (DGRAM: non-connection-mode socket)
    int s = socket(AF_INET, SOCK_DGRAM, 0);

    pid_t = pid;
    if ((pid = fork()) == 0) { // child process
        while (1) {
            // Send buffer containing string to server
            ssize_t succ = new_sendto(s, buffer, strlen(buffer)+1, 0, (struct
                        sockaddr*) &server_address, sizeof server_address, stream);
            if (succ < 0)
                printf("Error sending data.");

            sleep(3);
        }
    }


    // Receive and print response from server
    recvfrom(s, buffer_response, str_size, 0, &server_rec_address,
             &server_rec_len);
    printf("Data received from server:\n%s\n", buffer_response);
}
    
ssize_t new_sendto(int s, const void *buf, size_t len, int flags, const struct
        sockaddr *to, socklen_t tolen, int stream) {

}
