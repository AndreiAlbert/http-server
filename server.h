#ifndef SERVER_H
#define SERVER_H

typedef struct HTTP_Server{
    int socket; 
    int port;
} HTTP_Server;

void init_server(HTTP_Server *srv, int port);

#endif
