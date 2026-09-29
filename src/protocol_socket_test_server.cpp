#include "../include/protocol.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/protocol_test.sock"

int main() {
    int listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    unlink(SOCKET_PATH);
    bind(listen_fd, (sockaddr*)&addr, sizeof(addr));
    listen(listen_fd, 5);
    std::cout << "Server waiting...\n";

    int client_fd = accept(listen_fd, nullptr, nullptr);
    Connection conn{client_fd, ""};

    // Read three lines, echo each one back as a response.
    for (int i = 0; i < 3; i++) {
        auto line = readLine(conn);
        if (!line) { std::cout << "Client disconnected early\n"; break; }
        std::cout << "Server got: [" << *line << "]\n";

        ParsedRequest r = parseRequest(*line);
        std::string body = "you sent cmd=" + r.command + " arg=" + r.arg;
        sendAll(client_fd, buildResponse(true, body));
    }

    close(client_fd);
    close(listen_fd);
    unlink(SOCKET_PATH);
}