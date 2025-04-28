#include "response.h"
#include "request.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

const char *get_mime_type(const char *filename) {
    struct {
        const char *ext;
        const char *mime_type;
    } mime_types[] = {
        { "html", "text/html" },
        { "htm", "text/html" },
        { "txt", "text/plain" },
        { "png", "image/png" },
        { "jpg", "image/jpeg" },
        { "jpeg", "image/jpeg" },
        { "gif", "image/gif" },
        { "css", "text/css" },
        { "js", "application/javascript" },
        { NULL, NULL }
    };

    const char *ext = strrchr(filename, '.');
    if (!ext) {
        return "application/octet-stream";
    }
    ext++;

    for (int i = 0; mime_types[i].ext != NULL; ++i) {
        if (strcmp(ext, mime_types[i].ext) == 0) {
            return mime_types[i].mime_type;
        }
    }

    return "application/octet-stream";
}

void send_404(int client_fd) {
    const char *msg =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Length: 13\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "404 Not Found";
    write(client_fd, msg, strlen(msg));
}

void send_405(int client_fd) {
    const char *msg =
        "HTTP/1.1 405 Method Not Allowed\r\n"
        "Content-Length: 23\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "405 Method Not Allowed";
    write(client_fd, msg, strlen(msg));
}

void send_static_file(int client_fd, const char *path) {
    char full_path[1024] = "./static";
    strncat(full_path, path + 7, sizeof(full_path) - strlen(full_path) - 1);

    int file_fd = open(full_path, O_RDONLY);
    if (file_fd < 0) {
        send_404(client_fd);
        return;
    }

    struct stat st;
    fstat(file_fd, &st);

    const char *mime_type = get_mime_type(full_path);

    dprintf(client_fd,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %ld\r\n"
            "\r\n",
            mime_type, st.st_size);

    char buffer[1024];
    ssize_t n;
    while ((n = read(file_fd, buffer, sizeof(buffer))) > 0) {
        write(client_fd, buffer, n);
    }

    close(file_fd);
}

void handle_calc(int client_fd, const char *path) {
    char operation[16];
    int num1, num2;
    int result;
    char op_symbol = '?';
    char response_body[2048];
    char response_header[512];

    if (sscanf(path, "/calc/%15[^/]/%d/%d", operation, &num1, &num2) != 3) {
        const char *msg = "HTTP/1.1 400 Bad Request\r\n"
                          "Content-Length: 12\r\n"
                          "Content-Type: text/plain\r\n"
                          "\r\n"
                          "Bad Request";
        write(client_fd, msg, strlen(msg));
        return;
    }

    if (strcmp(operation, "add") == 0) {
        result = num1 + num2;
        op_symbol = '+';
    } else if (strcmp(operation, "mul") == 0) {
        result = num1 * num2;
        op_symbol = '*';
    } else if (strcmp(operation, "div") == 0) {
        if (num2 == 0) {
            const char *msg = "HTTP/1.1 400 Bad Request\r\n"
                              "Content-Length: 15\r\n"
                              "Content-Type: text/plain\r\n"
                              "\r\n"
                              "Division by zero";
            write(client_fd, msg, strlen(msg));
            return;
        }
        result = num1 / num2;
        op_symbol = '/';
    } else {
        const char *msg = "HTTP/1.1 400 Bad Request\r\n"
                          "Content-Length: 12\r\n"
                          "Content-Type: text/plain\r\n"
                          "\r\n"
                          "Bad Request";
        write(client_fd, msg, strlen(msg));
        return;
    }

    snprintf(response_body, sizeof(response_body),
             "<!DOCTYPE html>\r\n"
             "<html>\r\n"
             "<head>\r\n"
             "  <title>Calculator Result</title>\r\n"
             "  <style>\r\n"
             "    body { font-family: Arial, sans-serif; text-align: center; margin-top: 100px; }\r\n"
             "    h1 { font-size: 36px; color: #555; }\r\n"
             "    h2 { font-size: 48px; color: #333; }\r\n"
             "  </style>\r\n"
             "</head>\r\n"
             "<body>\r\n"
             "  <h1>%d %c %d</h1>\r\n"
             "  <h2>Result: %d</h2>\r\n"
             "</body>\r\n"
             "</html>\r\n",
             num1, op_symbol, num2, result);

    size_t body_length = strlen(response_body);

    snprintf(response_header, sizeof(response_header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %zu\r\n"
             "\r\n",
             body_length);

    write(client_fd, response_header, strlen(response_header));
    write(client_fd, response_body, body_length);
}



void handle_sleep(int client_fd, const char *path) {
    int seconds = 0;
    if (sscanf(path, "/sleep/%d", &seconds) != 1 || seconds < 0) {
        const char *msg = "HTTP/1.1 400 Bad Request\r\n"
                          "Content-Length: 12\r\n"
                          "Content-Type: text/plain\r\n"
                          "\r\n"
                          "Bad Request";
        write(client_fd, msg, strlen(msg));
        return;
    }

    sleep(seconds);

    const char *msg = "HTTP/1.1 200 OK\r\n"
                      "Content-Length: 4\r\n"
                      "Content-Type: text/plain\r\n"
                      "\r\n"
                      "Done";
    write(client_fd, msg, strlen(msg));
}

void handle_http_response(int client_fd, struct http_request *req) {
    if (strcmp(req->method, "GET") != 0) {
        send_405(client_fd);
        return;
    }

    if (strncmp(req->path, "/static", 7) == 0) {
        send_static_file(client_fd, req->path);
    } else if (strncmp(req->path, "/calc", 5) == 0) {
        handle_calc(client_fd, req->path);
    } else if (strncmp(req->path, "/sleep", 6) == 0) {
        handle_sleep(client_fd, req->path);
    } else {
        send_404(client_fd);
    }
}
