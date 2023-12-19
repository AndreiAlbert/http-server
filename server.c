#include "server.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

void init_server(HTTP_Server *srv, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("[ERRROR]: Could not init socket");
        exit(1);
    }
    struct sockaddr_in host_addr;
    int host_addrlen = sizeof(host_addr);

    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(port);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if(bind(sockfd, (struct sockaddr*)&host_addr, host_addrlen) != 0) {
        perror("[ERROR]: Could not bind to port");
        exit(1);
    }
    if(listen(sockfd, SOMAXCONN) != 0) {
        perror("[ERROR]: Could not start listening");
        exit(1);
    }
    srv -> socket = sockfd;
    srv -> port = port;
    printf("Started listening on port %d\n", srv -> port);
}
