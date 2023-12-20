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
    socket_t sockfd = socket(AF_INET, SOCK_STREAM, 0);
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
    free(headers);
}

char** parse_request_route(HTTP_Client* client) {
    char** parsed_route; 
    size_t i = 0;
    char* pch = strchr(client->header.target, '/');
    while(pch) {
        i++;
        pch = strchr(pch + 1, '/');
    }
    if(i == 1) {
        return NULL;
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

void send_403_status(HTTP_Client* client, HTTP_Server* srv) {
    const char http_server_err_403_page[] = "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<title>403 Forbidden</title>"
        "</head>"
        "<body>"
        "<h1>403 Forbidden</h1>"
        "<p>"
        "Access to this resource is forbidden."
        "</p>" 
        "</body>"
        "</html>";
    const char header[] = "HTTP/1.1 403 Forbidden\r\n"
                          "Content-type: text/html\r\n"
                          "Content-Length: ";
    char content_length[50];
    sprintf(content_length, "%lu\r\n\r\n", (unsigned long)strlen(http_server_err_403_page));
    char response[4097];
    response[0] = '\0';
    strcat(response, header);
    strcat(response, content_length);
    strcat(response, http_server_err_403_page);
    if(write(client -> socket, response, strlen(response)) < 0) {
        fprintf(stderr, "Could not write to socket");
    }
}

static char* read_file(char* path_to_file) {
    FILE* f = fopen(path_to_file, "rb");
    if(f == NULL) {
        fprintf(stderr, "Failed to read %s\n", path_to_file);
        free(path_to_file);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* content_file = malloc(sizeof(char) * (file_size + 1));
    if(!content_file) {
        fprintf(stderr, "Could not allocate\n");
        return NULL;
    }
    size_t index = 0; 
    char ch;
    while((ch = getc(f)) != EOF) {
        content_file[index++] = ch;
    }
    fclose(f);
    content_file[index] = '\0';
    return content_file;
}

void send_file(HTTP_Client* client, HTTP_Server* srv, char* file) {
    size_t path_size = strlen(srv->resource_dir) + strlen(file) + 2;
    char* file_path = malloc(sizeof(char) * path_size);
    if(!file_path) {
        fprintf(stderr, "Could not allocate");
        return;
    }
    file_path[0] = '\0';
    strcat(file_path, srv->resource_dir);
    file_path[strlen(file_path)] = '/';
    file_path[strlen(file_path) + 1] = '\0';
    strcat(file_path, file);
    char* file_content = read_file(file_path);
    const char header[] = "HTTP/1.1 200 OK\r\n"
                          "Content-Length: ";
    char content_length[50];
    content_length[0] = '\0';
    sprintf(content_length, "%lu\r\n\r\n", (unsigned long)strlen(file_content));
    size_t response_size = strlen(header) + strlen(content_length) + strlen(file_content) + 1;
    char *response = malloc(sizeof(char) * response_size);
    response[0] = '\0';
    strcat(response, header); 
    strcat(response, content_length);
    strcat(response, file_content);
    if(write(client->socket, response, strlen(response)) < 0) {
        fprintf(stderr, "Could not write");
    }
    free(response);
}

void handle_request_get(HTTP_Server* srv, HTTP_Client* client) {
    char** parsed_route = parse_request_route(client);
    if(parsed_route == NULL || parsed_route[0] == NULL){
        send_403_status(client, srv);
        return; 
    }
    if(strcmp(parsed_route[0], srv->resource_dir + 2) != 0) {
        send_403_status(client, srv);
        return;
    }
    else if(parsed_route[1] != NULL){
        printf("youre cool\n");
        send_file(client, srv, parsed_route[1]);
    }
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
