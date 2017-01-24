// WEBSERVER USING SELECT()

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>


int BUFFER_SIZE = 10000;

int main() {
    // Create and define sockaddr_in to store server properties
    int port_num = 3333;                // random port number > 1024
    char dir[] = "/home/anni";          // home directory
    int num_pending_connections = 10;   // max no. of pending connections
    char in_buffer[BUFFER_SIZE];
    int fd_max;
    int go_on = 1;

    // Set up getaddrinfo call
    struct addrinfo hints;
    struct addrinfo* res;

    fd_set read_fds;
    fd_set main_fds;
    int client_socks[FD_SETSIZE];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int gai_status = getaddrinfo(NULL, "3333", &hints, &res);
    if (gai_status != 0) {
        printf("getaddrinfo(): Not successful.\n");
    }

    // Buffer to write HTTP request to
    char http_request[BUFFER_SIZE];
    size_t buffer_size = BUFFER_SIZE;

    // Initialize and bind socket (SOCK_STREAM: connection-mode socket for TCP)
    printf("----------\nBinding socket.\n");
    int s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    int optval = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    int succ_bind = bind(s, res->ai_addr, res->ai_addrlen);
    if (succ_bind < 0)
        printf("\nBind not successful.\n");

    // Listen (TCP specific)
    printf("Listening.\n");
    int succ_listen = listen(s, num_pending_connections);
    if (succ_listen < 0)
        printf("listen(): An error has occured.\n");

    for (int i = 0; i<FD_SETSIZE; i++)
        client_socks[i] = -1;

    FD_ZERO(&read_fds);
    FD_ZERO(&main_fds);
    FD_SET(s, &main_fds);
    FD_SET(0, &main_fds);

    fd_max = s;

    while (go_on) {
        read_fds = main_fds;

        int succ_select = select(fd_max+1, &read_fds, NULL, NULL, NULL);
        if (succ_select == -1)
            printf("select() not successful.\n");

        for (int i = 0; i <= fd_max; i++) {
            if(FD_ISSET(i, &read_fds)) {

                // if user has typed a command
                if (i == 0) {
                    fgets(in_buffer, BUFFER_SIZE, stdin);
                    switch (in_buffer[0]) {
                    case 'q':
                        go_on = 0;
                        break;
                    case 's':
                        fflush(stdout);
                        close(s);
                        FD_CLR(s, &main_fds);
                        break;
                    default:
                        fflush(stdout);
                    }
                }
                else {
                    // Create sockaddr to store client properties when receiving
                    struct sockaddr_storage client_address;
                    socklen_t client_addr_len = sizeof client_address;

                    // Accept connection from queue (TCP specific).
                    // accept() creates new connected socket and returns its file descriptor.
                    int new_socket = accept(s, (struct sockaddr*) &client_address,
                                            &client_addr_len);

                    FD_SET(new_socket, &main_fds);

                    if (new_socket > fd_max)
                        fd_max = new_socket;

                    // Receive request
                    printf("Receiving.\n");
                    int succ_recv = recv(new_socket, http_request, buffer_size, 0);
                    if (succ_recv < 0)
                        printf("recv(): An error has occured.\n");
                    printf("Number of received bytes (-1 if unsuccessful): %d\n",
                           succ_recv);

                    /*printf("\n\nRequest received:\n\n%s\n", http_request);*/

                    // Split first 3 WORDS of request into 3 separate char arrays
                    char* first = strtok(http_request, " ");    // GET/POST etc.
                    char* second = strtok(NULL, " ");           // Requested file
                    char* third = strtok(NULL, "\r");           // http version

                    // Check if request is not a http GET request
                    if (strcmp(first, "GET"))       // 0 if equal
                        printf("Not a supported http request (only GET is accepted).\n");

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
                        printf("Sending http response.\n");
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
                        printf("Sending http response not successful.\n");
                    else
                        printf("Bytes sent: %d\n", num_bytes_sent);

                    // close new socket in child process
                    int succ_close = close(new_socket);
                    if (succ_close < 0)
                        printf("close(): An error has occured.\n");
                    FD_CLR(new_socket, &main_fds);
                } // if not fd==0
            }   // if (FD_ISSET)
        }   // for all file descriptors

        // Parent closes new socket
    }

    freeaddrinfo(res);
    return 0;
}
