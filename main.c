#include <arpa/inet.h>
#include <asm-generic/errno.h>
#include <string.h>
#include <unistd.h>
#include "server.h"
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdio.h>
#define PORT 8080

char *prepend_char(char* str, char chr) {
    size_t len = strlen(str);
    char* new_str = malloc(len + 2);
    if(!new_str) {
        perror("Could not allocate to prepend");
        return NULL;
    }
    new_str[0] = chr; 
    if(memcpy(new_str + 1, str, len) < 0) {
        perror("Could not memcpy");
        return NULL;
    }
    new_str[len + 1] = '\0';
    return new_str;
}

char* read_resource(char *path_to_resource) {
    FILE *f = fopen(path_to_resource, "rb");
    if(f == NULL) {
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* content = malloc(sizeof(char) * (file_size + 1));
    if(!content) {
        perror("Could not allocate memory for file");
        return NULL;
    }
    size_t i = 0;
    char ch; 
    while((ch = getc(f)) != EOF) {
        content[i] = ch; 
        i++;
    }
    fclose(f);
    content[i] = '\0';
    return content;
}

int main() {
    HTTP_Server *srv = malloc(sizeof(HTTP_Server));
    init_server(srv, 8080, "./resources");
    start_server(srv);
    //HTTP_Server *srv = malloc(sizeof(HTTP_Server));
    //if(!srv) {
        //perror("Memory allocation failed");
        //return 1;
    //}
    //init_server(srv, PORT);
    //while(true) {
        //char *method = "";
        //char *route = "";
        //char client_socket_header[BUFFER_SIZE];
        //int client_socket = accept(srv->socket, NULL, NULL);
        //if(client_socket < 0) {
            //perror("Failed to accept connection");
            //continue;
        //}
        //size_t bytes = read(client_socket, client_socket_header, BUFFER_SIZE);
        //if(bytes < 0) {
            //perror("read");
            //continue;
        //}
        //if(bytes < 3) {
            //perror("not valid ");
            //continue;
        //}
        //client_socket_header[bytes] = '\0';

        //char *header = strtok(client_socket_header, "\n");
        //char *header_token = strtok(header, " ");
        //method = header_token;
        //header_token = strtok(NULL, " ");
        //route = header_token;

        //printf("%s\n", method);
        //printf("%s\n", route);

        //char *path_to_resource = prepend_char(route, '.');
        //char *resource_content = read_resource(path_to_resource);
        //printf("%s\n", resource_content);

        //char response[BUFFER_SIZE] = "HTTP/1.1 200 OK\r\n\r\n";
        //strcat(response, resource_content);
        //strcat(response, "\r\n\r\n");

        //send(client_socket, response, strlen(response), 0);

        //close(client_socket);
    //}
    //free(srv);
    //return 0;
}
