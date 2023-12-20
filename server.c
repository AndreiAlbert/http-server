#include "server.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <dirent.h>

static bool check_dir_exists(char *path) {
    struct stat st; 
    if(stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
        return true;
    }
    return false;
}

void init_server(HTTP_Server *srv, int port, char* resource_dir) {
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
    if(!check_dir_exists(resource_dir)) {
        fprintf(stderr, "%s is not a directory!\n", resource_dir);
        exit(1);
    }
    srv -> resource_dir = malloc(sizeof(char) * (strlen(resource_dir) + 1));
    strcpy(srv->resource_dir, resource_dir);
    srv -> socket = sockfd;
    srv -> port = port;
    printf("Started listening on port %d on directory %s\n", srv -> port, srv -> resource_dir);
}

static void parse_first_header_field(HTTP_Client* client, char* field) {
    char* token = strtok(field, " ");
    size_t index = 1;
    while(token) {
        switch (index) {
            case 1:
                strncpy(client->header.method, token, strlen(token));
                client->header.method[strlen(client->header.method)] = '\0';
                break;
            case 2:
                strncpy(client->header.target, token, strlen(token));
                client->header.target[strlen(client->header.target)] = '\0';
                break;
            case 3:
                strncpy(client->header.version, token, strlen(token));
                client->header.version[strlen(client->header.version)] = '\0';
                break;
        } 
        token = strtok(NULL, " ");
        index++;
    }
}

void parse_http_header(HTTP_Client* client, char header[4097]) {
    char *headers = strdup(header);
    printf("%s\n", headers);
    char *header_token = strtok(headers, "\r\n");
    size_t header_fields_index = 0;
    while(header_token) {
        if(header_fields_index == 0) {
            parse_first_header_field(client, header_token);
            header_token = strtok(NULL, "\r\n");
            header_fields_index++;
            continue;
        }
        header_token = strtok(NULL, "\r\n");
        header_fields_index++;
    }
}

char** parse_request_route(HTTP_Client* client) {
    char** parsed_route; 
    size_t i = 0;
    char* pch = strchr(client->header.target, '/');
    while(pch) {
        i++;
        pch = strchr(pch + 1, '/');
    }
    parsed_route = malloc(sizeof(char*) * (i + 1));
    if(!parsed_route) {
        fprintf(stderr, "Could not allocate");
        return NULL;
    }
    char* route = strdup(client->header.target);
    if(!route) {
        fprintf(stderr, "Could not strdup");
        return NULL;
    }
    char* route_token = strtok(route, "/");
    size_t index = 0;
    while(route_token) {
        parsed_route[index] = strdup(route_token);
        if(!parsed_route[index]) {
            for(size_t j = 0; j < index; j++) {
                free(parsed_route[j]);
            }
            free(parsed_route);
            free(route);
            return NULL;
        }
        index++;
        route_token = strtok(NULL, "/");
    }
    parsed_route[index] = NULL;
    free(route);
    return parsed_route;
}

void handle_request_get(HTTP_Server* srv, HTTP_Client* client) {
    DIR *d;
    struct dirent* dir; 
    d = opendir(srv->resource_dir);
    if(d) {
        while((dir = readdir(d)) != NULL) {
            printf("%s\n", dir -> d_name);
        }
        closedir(d);
    }
    parse_request_route(client);
}

void handle_request(HTTP_Server* srv, HTTP_Client* client) {
    if(strcmp(client->header.method, "GET") == 0) {
        handle_request_get(srv, client);
    }

}

void start_server(HTTP_Server* srv) {
    while(true) {
        HTTP_Client* client = malloc(sizeof(HTTP_Client));
        if(!client) {
            fprintf(stderr, "Failed to malloc");
            continue;
        }
        client -> socket = accept(srv -> socket, NULL, NULL);
        if(client -> socket < 0) {
            fprintf(stderr, "Failed to accept connecion");
            continue; 
        }
        char data[4097];
        size_t bytes = read(client -> socket, data, 4096);
        if(bytes < 0) {
            fprintf(stderr, "Failed to read header");
            continue;
        }
        data[bytes] = '\0';
        parse_http_header(client, data);
        handle_request(srv, client);
        close(client -> socket);
        free(client);
    }
}









