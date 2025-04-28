#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include "request.h"
#include "response.h"

#define DEFAULT_PORT 80

void *handle_client(void *arg);

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        }
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(port),
    };

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", port);

    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        pthread_t tid;
        int *client_ptr = malloc(sizeof(int));
        *client_ptr = client_fd;
        pthread_create(&tid, NULL, handle_client, client_ptr);
        pthread_detach(tid);
    }

    close(server_fd);
    return 0;
}

void *handle_client(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);

    while (1) {
        struct http_request req;
        memset(&req, 0, sizeof(req));

        if (parse_http_request(client_fd, &req) != 0) {
            break; // client closed or invalid
        }

        handle_http_response(client_fd, &req);
    }

    close(client_fd);
    return NULL;
}
