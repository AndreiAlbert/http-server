#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "server.h"
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdio.h>
#include <assert.h>
#define PORT 8080
#define BUFFER_SIZE 1024

void handle_request(char *method, char *route) {
    printf("Method: %s\n", method);
    printf("Route: %s\n", route);
    assert(false && "not implemented yet");
}

int main() {
    HTTP_Server *srv = malloc(sizeof(HTTP_Server));
    init_server(srv, PORT);
    char client_msg[4096];
    while(true) {
        int client_socket = accept(srv->socket, NULL, NULL);
        char *method = "";
        char *route = "";
        read(client_socket, client_msg, 4095);
        printf("%s\n", client_msg);
        printf("hei\n");
        char *header = strtok(client_msg, "\n");
        char *header_token = strtok(header, " ");
        method = header_token;
        header_token = strtok(NULL, " ");
        route = header_token;
        handle_request(method, route);
    }
    return 0;
}
