// CONCURRENT WEBSERVER SUPPORTING IPv6

#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>


int BUFFER_SIZE = 10000;

int main() {
    // Create and define sockaddr_in to store server properties
    struct sockaddr_in6 server_address;
    int port_num = 3333;                // random port number > 1024
    char dir[] = "/home/anni";          // home directory
    int num_pending_connections = 10;   // max no. of pending connections

    server_address.sin6_family = AF_INET6;
    server_address.sin6_port = htons(port_num);
    server_address.sin6_flowinfo = 0;
    server_address.sin6_addr = in6addr_any;
    server_address.sin6_scope_id = 0;

    // Buffer to write HTTP request to
    char http_request[BUFFER_SIZE];       
    size_t buffer_size = BUFFER_SIZE;     

    // Initialize and bind socket (SOCK_STREAM: connection-mode socket for TCP)
    printf("----------\nBinding socket.");
    int s = socket(AF_INET6, SOCK_STREAM, 0);

    int optval = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    int succ_bind = bind(s, (struct sockaddr*) &server_address,
                         sizeof server_address);
    if (succ_bind < 0)
        printf("\nBind not successful.");

    // Listen (TCP specific)
    printf("\nListening.");
    int succ_listen = listen(s, num_pending_connections);
    if (succ_listen < 0)
        printf("\nlisten(): An error has occured");

    pid_t pid;
    for (;;) {
        // Create sockaddr to store client properties when receiving
        struct sockaddr_in6 client_address;
        socklen_t client_addr_len;
        int client_addr_buffer_size = 100;
        char client_address_ipv6[client_addr_buffer_size];

        // Accept connection from queue (TCP specific).
        // accept() creates new connected socket and returns its file descriptor.
        int new_socket = accept(s, (struct sockaddr*) &client_address,
                                &client_addr_len);
        inet_ntop(AF_INET6, &(client_address.sin6_addr), client_address_ipv6,
                  client_addr_buffer_size);
        printf("\nIncoming connection from client with IPv6: %s",
               client_address_ipv6);

        // Fork new process and check if child or parent
        if ((pid = fork()) == 0) {  // true for child process
            close(s);

            // Receive request
            printf("\nReceiving.");
            int succ_recv = recv(new_socket, http_request, buffer_size, 0);
            if (succ_recv < 0)
                printf("\nrecv(): An error has occured");
            printf("\nNumber of received bytes (-1 if unsuccessful): %d\n",
                   succ_recv);

            /*printf("\n\nRequest received:\n\n%s\n", http_request);*/

            // Split first 3 WORDS of request into 3 separate char arrays
            char* first = strtok(http_request, " ");    // GET/POST etc.
            char* second = strtok(NULL, " ");           // Requested file
            char* third = strtok(NULL, "\r");           // http version
            
            // Check if request is not a http GET request
            if (strcmp(first, "GET"))       // 0 if equal
                printf("Not a supported http request (only GET is accepted).");
            
            // Put together the path of the requested html file using *dir*
            char path_to_requested_file[strlen(dir) + strlen(second)];
            sprintf(path_to_requested_file, "%s%s", dir, second);  
            
            int num_bytes_sent;

            // Open requested file
            long fsize = 0;
            FILE *f = fopen(path_to_requested_file, "rb");

            // Check if the requested file exists and send appropriate response
            if (f == NULL) {
                // Put together http 404 response
                char http_status_line[] = "404 Not Found";
                char http_response[strlen(third) + strlen(http_status_line) + 6];
                sprintf(http_response, "%s %s\r\n\r\n", third, http_status_line);

                // Send data using new socket
                printf("\nSending http response.");
                int num_bytes_sent = send(new_socket, http_response,
                                          strlen(http_response)+1, 0);
            }
            else {  // File does exist
                fseek(f, 0, SEEK_END);
                fsize = ftell(f);
                fseek(f, 0, SEEK_SET);  //same as rewind(f);

                // Create char array to hold file contents and read them.
                char* file_content = malloc(fsize + 1);
                fread(file_content, fsize, 1, f);
                fclose(f);

                // Put together http 200 response
                char http_status_line[] = "200 OK";
                int response_length = strlen(third) + strlen(http_status_line) + 200
                                      + fsize;
                char http_response[response_length];
                sprintf(http_response,
                        "%s %s\r\nContent-Length:%ld\r\nContent-Type:text/html\r\n\r\n%s",
                        third, http_status_line, fsize, file_content);
         
                // Send data using new socket
                printf("\nSending http response:\n%s\n", http_response);
                int num_bytes_sent = send(new_socket, http_response,
                                          strlen(http_response)+1, 0);
            }

            if (num_bytes_sent < 0)
                printf("\nSending http response not successful.");
            else
                printf("\nBytes sent: %d", num_bytes_sent);

            // close new socket in child process
            int succ_close = close(new_socket);
            /*int succ_close = shutdown(new_socket, SHUT_RDWR);*/
            if (succ_close < 0)
                printf("\nshutdown(): An error has occured");

            exit(0);
        }
        // Parent closes new socket
        close(new_socket);
    }
    return 0;
}
