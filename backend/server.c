#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 4096

int main(void) {
    int server_fd;
    int client_fd;

    struct sockaddr_in address;
    socklen_t address_length = sizeof(address);

    char request[BUFFER_SIZE];

    const char *body =
        "<!doctype html>"
        "<html><body>"
        "<h1>Hello from C</h1>"
        "<p>This response came from a tiny C web server.</p>"
        "</body></html>";

    char response[BUFFER_SIZE];

    // 1. Create a TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    // Allow quick reuse of the port after restarting
    int reuse = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // 2. Configure the address
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(PORT);

    // 3. Bind the socket to port 8080
    if (bind(server_fd,
             (struct sockaddr *)&address,
             sizeof(address)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    // 4. Start listening
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("Server listening at http://localhost:%d\n", PORT);

    while (1) {
        // 5. Accept a client connection
        client_fd = accept(
            server_fd,
            (struct sockaddr *)&address,
            &address_length
        );

        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        // 6. Read the HTTP request
        ssize_t bytes_read = read(
            client_fd,
            request,
            sizeof(request) - 1
        );

        if (bytes_read > 0) {
            request[bytes_read] = '\0';
            printf("Request:\n%s\n", request);
        }

        // 7. Build an HTTP response
        int response_length = snprintf(
            response,
            sizeof(response),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=utf-8\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n"
            "\r\n"
            "%s",
            strlen(body),
            body
        );

        // 8. Send the response
        write(client_fd, response, response_length);

        // 9. Close the connection
        close(client_fd);
    }

    close(server_fd);
    return 0;
}
