// CLIENT

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>


unsigned int BUFFER_SIZE = 256;


ssize_t new_sendto(int s, const void *buf, size_t len, int flags, const struct
        sockaddr *to, socklen_t tolen, int stream) {
    char str_stream[2];
    sprintf(str_stream, "%d", stream);

    char* full_buffer = malloc(len + strlen(str_stream));
    int full_buffer_size = len + strlen(str_stream);
    strcpy(full_buffer, str_stream);
    if (stream == 1) {
        strcat(full_buffer, buf);
    }
    else {
        memcpy(full_buffer+1, buf, len);
    }

    printf("Buffer to send, first char: %c\n", full_buffer[0]);

    // Send buffer containing string to server
    ssize_t succ = sendto(s, full_buffer, full_buffer_size, flags, to, tolen);
    return succ;
}


int main() {
    // Create and define sockaddr_in to save server details
    struct sockaddr_in server_address;
    int port_num = 1025;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_num);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");    // localhost

    // Create sockaddr to store server properties receiving a response from
    // later
    /*struct sockaddr server_rec_address;*/
    /*socklen_t server_rec_len;*/

    // Create buffer to store a string input by user
    char buffer[BUFFER_SIZE];
    /*char buffer_response[BUFFER_SIZE];*/
    // Ask user to input string
    printf("Input data to send periodically: \n");
    scanf("%255s", buffer);

    // Initialize socket (DGRAM: non-connection-mode socket)
    int s = socket(AF_INET, SOCK_DGRAM, 0);

    // Open the dog file.
    char file_name[] = "dog.jpg";
    FILE *fp = fopen(file_name, "r");
    if (fp == NULL)
        printf("File doesn't exist.\n");

    // Get file size.
    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    printf("File size: %zu\n", file_size);
    fseek(fp, 0, SEEK_SET);

    // Create buffer and copy file contents into buffer
    char file_buffer[file_size];
    int fread_succ = fread(file_buffer, file_size, 1, fp);
    if (fread_succ <= 0)
        perror("Unable to copy file to buffer.\n");

    pid_t pid;
    if ((pid = fork()) == 0) { // child process 1
        int stream_num = 1;
        for (int i = 0; i<6; i++) {
            new_sendto(s, buffer, strlen(buffer)+1, 0, (struct sockaddr*)
                    &server_address, sizeof server_address, stream_num);
            sleep(3);
        }
    }

    if ((pid = fork()) == 0) { // child process 2
        int stream_num = 2;
        for (int i = 0; i<1; i++) {
            // Ask user to input string
            /*char buffer_keyboard[BUFFER_SIZE];*/
            /*printf("Input data to send now: \n");*/
            /*scanf("%255s", buffer_keyboard);*/
            sleep(4);
            printf("File Buffer size: %zu\n", sizeof(file_buffer));
            new_sendto(s, file_buffer, file_size, 0, (struct sockaddr*)
                    &server_address, sizeof server_address, stream_num);
        }
    }
    
    else
        sleep(20);

    // Receive and print response from server
    /*recvfrom(s, buffer_response, str_size, 0, &server_rec_address,*/
             /*&server_rec_len);*/
    /*printf("Data received from server:\n%s\n", buffer_response);*/
    return 0;
}

