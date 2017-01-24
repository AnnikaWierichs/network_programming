// CLIENT

#include <stdio.h> 
#include <string.h> 

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>


unsigned int BUFFER_SIZE = 256

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

    // Send buffer containing string to server
    ssize_t succ = sendto(s, buffer, strlen(buffer)+1, 0, (struct sockaddr*)
                          &server_address, sizeof server_address);
    if (succ < 0)
        print("Error sending data.");

    // Receive and print response from server
    recvfrom(s, buffer_response, str_size, 0, &server_rec_address,
             &server_rec_len);
    printf("Print out of data received from server: %255s \n", buffer_response);
}
    

