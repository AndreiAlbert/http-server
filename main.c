#include "server.h"
#include <stdlib.h>
#define PORT 8080

int main() {
    HTTP_Server *srv = malloc(sizeof(HTTP_Server));
    init_server(srv, 8080, "./resources");
    start_server(srv);
    return 0;
}
