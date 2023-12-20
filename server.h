#ifndef SERVER_H
#define SERVER_H
#define BUFFER_SIZE 4096
#include <stdbool.h>

typedef int socket_t;

typedef struct HTTP_Header {
    char method[8];
    char target[128];
    char version[16];
    char host[64];
    char buffer[BUFFER_SIZE];
} HTTP_Header;

typedef struct HTTP_Server{
    socket_t socket; 
    int port;
    char* resource_dir;
} HTTP_Server;
 
typedef struct HTTP_Client {
    socket_t socket; 
    HTTP_Header header;
} HTTP_Client;

typedef struct HTTP_Response {
    int status_code; 
    char* content; 
    char* type; 
} HTTP_Response;

void init_server(HTTP_Server* srv, int port, char* resource_dir);
void start_server(HTTP_Server* srv);
void parse_http_header(HTTP_Client* client, char header[4097]);
void handle_request(HTTP_Server* srv, HTTP_Client* client);
void handle_request_get(HTTP_Server* srv, HTTP_Client* client);
char** parse_request_route(HTTP_Client* client);
void send_403_status(HTTP_Client* client, HTTP_Server* srv);
void send_file(HTTP_Client* client, HTTP_Server* srv, char* file);

static bool check_dir_exists(char *path);
static void parse_first_header_field(HTTP_Client* client, char* field);
static char* read_file(char* path_to_file);

#endif
