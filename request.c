#include "request.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int parse_http_request(int client_fd, struct http_request *req) {
    char buffer[2048] = {0};
    ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) {
        return -1;
    }

    sscanf(buffer, "%s %s %s", req->method, req->path, req->version);
    return 0;
}
