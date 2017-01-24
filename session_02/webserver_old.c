// SERVER

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
    struct sockaddr_in server_address;
    int port_num = 3333;    // random port number > 1024
    char dir[] = "/home/anni";
    int num_pending_connections = 10;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_num);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    // Create sockaddr to store client properties later when receiving from client 
    struct sockaddr_in client_address;
    socklen_t client_addr_len;

    // Buffer to write HTTP request to
    char buffer[BUFFER_SIZE];       
    size_t str_size = BUFFER_SIZE;     

    // Initialize and bind socket (DGRAM: non-connection-mode socket)
    int s = socket(AF_INET, SOCK_STREAM, 0);
    int succ = bind(s, (struct sockaddr*) &server_address, sizeof server_address);
    if (succ < 0)
        printf("Bind not successful.");
    
    // Listen (TCP specific)
    int succ_listen = listen(s, num_pending_connections);
    if (succ_listen < 0)
        printf("listen(): An error has occured");

    
    // Accept connection from queue (TCP specific)
    int new_s = accept(s, (struct sockaddr*) &client_address, &client_addr_len);

    // Receive request
    int succ_recv = recv(new_s, buffer, str_size, 0);
    printf("\n%d\n", succ_recv);
    if (succ_recv < 0)
        printf("\nrecv(): An error has occured");

    // Try to print request
    printf("\nRequest received:\n%s \n", buffer);
  
    char* first = strtok(buffer, " ");
    char* second = strtok(NULL, " ");
    char* third = strtok(NULL, "\r");

    printf("\n\nthird:\n%s\n", third);
    
    strcat(dir, second);
    printf("%s", dir);
    
    FILE *f = fopen(dir, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  //same as rewind(f);

    char *file_content = malloc(fsize + 1);
    fread(file_content, fsize, 1, f);
    fclose(f);

    char http_response[1000];
    char http_status_line[] = "200 OK";
    sprintf(http_response,
            "%s %s\r\nContent-Length:%ld\r\nContent-Type:text/html\r\n\r\n%s",
            third, http_status_line, fsize, file_content);

    printf("HTTP Response:\n\n%s", http_response);
    send(new_s, http_response, strlen(http_response)+1, 0);
    
    int succ_close = shutdown(new_s, SHUT_RDWR);
    if (succ_close < 0)
        printf("\nshutdown(): An error has occured");
}
