#ifndef RESPONSE_H
#define RESPONSE_H

#include "request.h"

void handle_http_response(int client_fd, struct http_request *req);

#endif
