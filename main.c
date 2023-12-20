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
    return 0;
}
