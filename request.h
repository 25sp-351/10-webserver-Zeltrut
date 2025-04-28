#ifndef REQUEST_H
#define REQUEST_H

struct http_request {
    char method[8];
    char path[1024];
    char version[16];
};

int parse_http_request(int client_fd, struct http_request *req);

#endif
